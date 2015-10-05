// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include <cmath>
#include <array>
#include "CImg.h"
//#include "GSL/include/gsl.h"

#define WIDTH 4096
#define HEIGHT 2160
#define NCOLORS 3

using namespace cimg_library;
using namespace std;

// Outer and inner specify the radius of the vignette, relative to the smaller dimension of the 
// image (1 means the radius is the same as the smaller dimension)
void PopulateVignette(CImg<float> &ar, float outer, float inner, float opacity)
{
    float wcenter = ar.width() / 2;
    float hcenter = ar.height() / 2;
    float smallerdim = (float)min(wcenter, hcenter);
    // Describes the color of the vignette

    // The part of the picture that has the vignette gradient. 0 is the center, 1 is the edge of the picture.
    float m = outer;
    float n = inner;
    // Maximum opacity of vignette.
    size_t hmax = ar.height();
    size_t wmax = ar.width();
    for (size_t j = 0; j < hmax; j++)
    {
        for (size_t i = 0; i < wmax; i++)
        {
            float xd = (i - wcenter);
            float yd = (j - hcenter);

            auto d = sqrt(xd*xd + yd*yd) / (float)smallerdim; // calc distance & normalize

            // Find edges of vignette radii.
            auto v = ((d - n) / (m - n))*opacity;
            v = d > n ? v : 0;
            v = d >= m ? opacity : v;

            // Populate vignette
            ar(i, j) = v;
        }
    }
}

#define R(img, i) (img[(i)])
#define G(img, i) (img[(i)+img.width()*img.height()])
#define B(img, i) (img[(i)+2*img.width()*img.height()])

#define Y(img, i) img[i]
#define Cb(img, i) img[i+img.width()*img.height()]
#define Cr(img, i) img[i+2*img.width()*img.height()]

float constrain(float value, float min, float max)
{
    value = value > max ? max : value;
    return value < min ? min : value;
}

// assumes a flat, 3 color value image.
void RGB2YCbCr(CImg<float> &img)
{
    size_t max_iter = img.width()*img.height();
#pragma ivdep
    for (size_t i = 0; i < max_iter; i++)
    {
        // Formula given in http://www.w3.org/Graphics/JPEG/jfif3.pdf
        float Ytemp =   0.299* R(img, i) + 0.587* G(img, i) + 0.114* B(img, i);
        float Cbtemp = -0.1687*R(img, i) - 0.3313*G(img, i) + 0.5*   B(img, i) + 128;
        float Crtemp =  0.5*   R(img, i) - 0.4187*G(img, i) - 0.0813*B(img, i) + 128;
        Y(img, i) = Ytemp;
        Cb(img, i) = Cbtemp;
        Cr(img, i) = Crtemp;
    }
}
// assumes a flat, 3 color value image.
void YCrCb2RGB(CImg<float> &img)
{
    size_t max_iter = img.width()*img.height();
#pragma ivdep
    for (size_t i = 0; i < max_iter; i++)
    {
        // Formula given in http://www.w3.org/Graphics/JPEG/jfif3.pdf
        float Rtemp = constrain(Y(img, i) + 1.402*  (Cr(img, i) - 128),                              0, 255);
        float Gtemp = constrain(Y(img, i) - 0.71414*(Cr(img, i) - 128) - 0.34414*(Cb(img, i) - 128), 0, 255);
        float Btemp = constrain(Y(img, i)                              + 1.772  *(Cb(img, i) - 128), 0, 255);
        R(img, i) = Rtemp;
        G(img, i) = Gtemp;
        B(img, i) = Btemp;
    }
}

// sat = 0 will result in B&W
// 0 < sat < 1 will result in lower saturation
// sat = 1 will result in the same image.
// 1 < sat will result in higher saturation
void Saturate(CImg<float> &img, float const sat)
{
    RGB2YCbCr(img);
    size_t max_iter = img.width()*img.height();
#pragma ivdep

    for (size_t i = 0; i < max_iter; i++)
    {
        Cb(img, i) = constrain((sat * (Cb(img, i) - 128) + 128), float(0.0), float(255.0));
        Cr(img, i) = constrain((sat * (Cr(img, i) - 128) + 128), (float)0.0, (float)255.0);
    }
    YCrCb2RGB(img);
}
void Contrast(CImg<float> &img, float const gain)
{
    RGB2YCbCr(img);
    size_t max_iter = img.width()*img.height();
#pragma ivdep
    for (size_t i = 0; i < max_iter; i++)
    {
        Y(img, i) = constrain(Y(img, i)*gain, 0, 255);
    }
    YCrCb2RGB(img);
}

void ApplyVignette(CImg<float> &img, int vignette_color[3], float outer_radius, float inner_radius, float opacity)
{
    auto ar = CImg<float>(img.width(), img.height(), 1, 1);
    PopulateVignette(ar, outer_radius, inner_radius, opacity);
    int n_colors = img.spectrum();
    //visu.fillC(0, 300, 255, 0, 1, 2, 3, 4);
    size_t image_it = 0;
    float* image_data = img.data();
    size_t wmax = img.width();
    size_t hmax = img.height();
    for (int color = 0; color < n_colors; color++)
    {
        auto color_value = vignette_color[color];
        for (int j = 0; j < hmax; j++)
        {
            for (int i = 0; i < wmax; i++)
            {
                image_data[image_it] = (ar(i, j) * color_value + (1 - ar(i,j))*image_data[image_it]);
                image_it++;
            }
        }
    }

}

int main(int argc, char* argv[])
{
    int vignette_color[3] = { 0, 0, 0 };
    //CImg<float> img(WIDTH, HEIGHT, 1, 3, 255);
    CImg<float> img("lena.bmp");
    ApplyVignette(img, vignette_color, 1, 0.5, 0.7);


    Contrast(img, 2.0);
    Saturate(img, 2.0);
    img.save("lena_modified.bmp");
    return 0;
}

