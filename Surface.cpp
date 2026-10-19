#include "SolarSystem.h"

SurfaceColor color(double r, double g, double b) { return SurfaceColor{r / 255, g / 255, b / 255}; }
SurfaceColor blend(SurfaceColor a, SurfaceColor b, double t) {
    t = clamp(t, 0, 1);
    return SurfaceColor{a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t};
}
SurfaceColor shade(SurfaceColor a, double t) { return SurfaceColor{a.r * t, a.g * t, a.b * t}; }
double smooth(double a, double b, double x) {
    double t = clamp((x - a) / (b - a), 0, 1);
    return t * t * (3 - 2 * t);
}
double wrapAngle(double a) { return atan2(sin(a), cos(a)); }
Vec spherePoint(double lon, double lat) {
    double c = cos(lat);
    return Vec{c * cos(lon), sin(lat), c * sin(lon)};
}
static double lattice(int x, int y, int z, int seed) {
    std::uint32_t h = std::uint32_t(x) * 374761393u + std::uint32_t(y) * 668265263u +
                      std::uint32_t(z) * 2246822519u + std::uint32_t(seed) * 3266489917u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return double(h) / 4294967295.0;
}
double surfaceNoise(double x, double y, double z, int seed) {
    int ix = (int)floor(x), iy = (int)floor(y), iz = (int)floor(z);
    double a = x - ix, b = y - iy, c = z - iz;
    a = a * a * (3 - 2 * a);
    b = b * b * (3 - 2 * b);
    c = c * c * (3 - 2 * c);
    double v[2][2];
    for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++)
            v[j][k] = lattice(ix, iy + j, iz + k, seed) * (1 - a) +
                      lattice(ix + 1, iy + j, iz + k, seed) * a;
    return ((1 - b) * v[0][0] + b * v[1][0]) * (1 - c) + ((1 - b) * v[0][1] + b * v[1][1]) * c;
}
double terrain(Vec p, double frequency, int seed, int octaves) {
    double result = 0, weight = 1, total = 0;
    for (int i = 0; i < octaves; i++) {
        result +=
            weight * surfaceNoise(p.x * frequency, p.y * frequency, p.z * frequency, seed + i * 19);
        total += weight;
        weight *= .5;
        frequency *= 2;
    }
    return result / total;
}
void craterPixels(std::vector<unsigned char> &pixels, int w, int h, int seed, int count,
                  double strength) {
    for (int c = 0; c < count; c++) {
        double lon = 2 * PI * lattice(c, seed, 3, seed);
        double lat = asin((lattice(c, seed, 7, seed) * 2 - 1) * .91);
        int cx = (int)(lon / (2 * PI) * w), cy = (int)((PI / 2 - lat) / PI * h);
        double r = 1.0 + 7 * pow(lattice(c, seed, 11, seed), 2), rx = r / std::max(.28, cos(lat));
        for (int dy = -(int)ceil(r); dy <= (int)ceil(r); dy++)
            for (int dx = -(int)ceil(rx); dx <= (int)ceil(rx); dx++) {
                int y = cy + dy, x = (cx + dx + w) % w;
                if (y < 0 || y >= h)
                    continue;
                double q = sqrt(dx * dx / (rx * rx) + dy * dy / (r * r));
                if (q > 1)
                    continue;
                size_t k = ((size_t)y * w + x) * 3;
                double local = (pixels[k] + pixels[k + 1] + pixels[k + 2]) / 765.0;
                double s = strength * (local < .27 ? .4 : 1);
                double factor = q < .76 ? 1 - s * .63 + s * .19 * dx / rx
                                        : 1 + s * .55 * sin((q - .76) / .24 * PI);
                for (int z = 0; z < 3; z++)
                    pixels[k + z] = (unsigned char)clamp(pixels[k + z] * factor, 0, 255);
            }
    }
}
