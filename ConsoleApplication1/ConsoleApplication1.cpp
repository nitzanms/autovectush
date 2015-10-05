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

void PopulateVignette(float ar[][HEIGHT] )
{
    int wcenter = WIDTH / 2;
    int hcenter = HEIGHT / 2;
    float smallerdim = (float)min(wcenter, hcenter);
    // Describes the color of the vignette

    // The part of the picture that has the vignette gradient. 0 is the center, 1 is the edge of the picture.
    float m = 1;
    float n = 0.5;
    // Maximum opacity of vignette.
    float opacity = 0.7;
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            float xd = (float)(i - wcenter);
            float yd = (float)(j - hcenter);

            auto d = sqrt(xd*xd + yd*yd) / (float)smallerdim; // calc distance & normalize

            // Find edges of vignette.
            auto v = ((d - n) / (m - n))*opacity;
            v = d > n ? v : 0;
            v = d >= m ? opacity : v;

            // Populate vignette
            ar[i][j] = v;
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
    for (size_t i = 0; i < img.width()*img.height(); i++)
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
    for (size_t i = 0; i < img.width()*img.height(); i++)
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
    for (size_t i = 0; i < img.width()*img.height(); i++)
    {
        Cb(img, i) = constrain((sat * (Cb(img, i) - 128) + 128), float(0.0), float(255.0));
        Cr(img, i) = constrain((sat * (Cr(img, i) - 128) + 128), (float)0.0, (float)255.0);
    }
    YCrCb2RGB(img);
}
void Contrast(CImg<float> &img, float const gain)
{
    RGB2YCbCr(img);
    for (size_t i = 0; i < img.width()*img.height(); i++)
    {
        Y(img, i) = constrain(Y(img, i)*gain, 0, 255);
    }
    YCrCb2RGB(img);
}


int main(int argc, char* argv[])
{
    auto ar = new float[WIDTH][HEIGHT];
    int vignette_color[3] = { 0, 128, 255 };

    PopulateVignette(ar);

    int n_colors = 3;
    CImg<float> visu(WIDTH, HEIGHT, 1, n_colors, 255);
    //visu.fillC(0, 300, 255, 0, 1, 2, 3, 4);
    size_t image_it = 0;
    float* image_data = visu.data();
    for (int color = 0; color < n_colors; color++)
    {
        auto color_value = vignette_color[color];
        for (int j = 0; j < HEIGHT; j++)
        {
            for (int i = 0; i < WIDTH; i++)
            {
                image_data[image_it] = (ar[i][j]*color_value + (1-ar[i][j])*image_data[image_it]);
                image_it++;
            }
        }
    }

    Contrast(visu, 2.0);
    Saturate(visu, 2.0);
    visu.save("vignette.bmp");
    return 0;
}

