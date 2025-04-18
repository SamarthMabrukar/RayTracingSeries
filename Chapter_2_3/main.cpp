#include <iostream>

#include "vec3.h"
#include "color.h"

int main()
{
    int iImageWidth = 256;
    int iImageHeight = 256;
    std::cout << "P3\n"
              << iImageWidth << " " << iImageHeight << "\n255\n";

    for (int j = iImageHeight - 1; j >= 0; j--)
    {
        std::clog << "\rScanlines remaining: " << (iImageHeight - j) << ' ' << std::flush;
        for (int i = 0; i < iImageWidth; i++)
        {
            auto pixel_color = color(double(i) / double(iImageWidth), double(j) / double(iImageHeight), 0.2);
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.                 \n";

    return 0;
}
