#include <iostream>

int main()
{
    int ix = 200;
    int iy = 100;
    std::cout << "P3\n"
              << ix << " " << iy << "\n255\n";
    for (int j = iy - 1; j >= 0; j--)
    {
        std::clog << "\rScanlines remaining: " << (iy - j) << ' ' << std::flush;
        for (int i = 0; i < ix; i++)
        {
            float r = float(i) / float(ix);
            float g = float(j) / float(iy);
            float b = 0.2;
            int ir = int(255.99 * r);
            int ig = int(255.99 * g);
            int ib = int(255.99 * b);
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
    std::clog << "\rDone.                 \n";
}
