// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include <cmath>
#include <array>
#include "CImg.h"
#include "advisor-annotate.h" 

#define WIDTH 4096
#define HEIGHT 2160
#define NCOLORS 3

using namespace cimg_library;
using namespace std;

int main(int argc, char* argv[])
{
    auto ar = new float[WIDTH][HEIGHT];
    int wcenter = WIDTH / 2;
    int hcenter = HEIGHT / 2;
    float smallerdim = (float)min(wcenter, hcenter);
    // Describes the color of the vignette
    int vignette_color[3] = { 0, 128, 255 };
    
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
            
            auto d = sqrt(xd*xd + yd*yd)/(float)smallerdim; // calc distance & normalize

            // Find edges of vignette.
            auto v = ((d - n)/(m - n))*opacity;
            v = d > n ? v : 0;
            v = d >= m ? opacity : v;

            // Populate vignette
            ar[i][j] = v;
        }
    }
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
    delete[] ar;
    return 0;
}

