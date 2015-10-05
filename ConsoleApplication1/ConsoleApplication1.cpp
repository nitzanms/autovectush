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


void ModifyImageContrast(CImg<float> img, float r, float g, float b)
{

}

#define R(img, i) (img[(i)])
#define G(img, i) (img[(i)+img.width()*img.height()])
#define B(img, i) (img[(i)+2*img.width()*img.height()])

#define Y(img, i) img[i]
#define Cr(img, i) img[i+img.width()*img.height()]
#define Cb(img, i) img[i+2*img.width()*img.height()]

// assumes a flat, 3 color value image.
void RGB2YCbCr(CImg<float> img)
{
    for (size_t i = 0; i < img.width()*img.height(); i++)
    {
        // Formula given in https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
        Y(img, i) = 0 + 0.299 * R(img, i) + 0.587 * G(img, i) + 0.114 * B(img, i);
        Cr(img, i) = 0.5*R(img, i) - 0.4187*G(img, i) - 0.0813*B(img, i) + 128;
        Cb(img, i) = -0.1687*R(img, i) - 0.3313*G(img, i) + 0.5*B(img, i) + 128;
    }
}
// assumes a flat, 3 color value image.
void YCrCb2RGB(CImg<float> img)
{
    for (size_t i = 0; i < img.width()*img.height(); i++)
    {
        // Formula given in https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
        R(img, i) = Y(img, i) + 1.402*(Cr(img, i) - 128);
        G(img, i) = Y(img, i) - 0.34414*(Cb(img, i) - 128) - 0.71414*(Cr(img, i) - 128);
        B(img, i) = Y(img, i) + 1.772*(Cb(img, i) - 128);
    }
}

int main(int argc, char* argv[])
{

    CImg<float> lena("lena.bmp");
    RGB2YCbCr(lena);
    YCrCb2RGB(lena);
    lena.save("lena_modified.bmp");
    return 0;
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
    visu.save("vignette.bmp");
    return 0;
}

