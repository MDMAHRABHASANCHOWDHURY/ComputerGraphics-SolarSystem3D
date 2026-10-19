#include "../SolarSystem.h"
Planet makeMercury() {
    return Planet{"Mercury",
                  "Rocky planet",
                  "Smallest planet; heavily cratered.",
                  "Almost no atmosphere.",
                  4879,
                  0.38709893,
                  87.969,
                  1407.6,
                  0.034,
                  3.7,
                  .43,
                  4.5,
                  20,
                  3.301e+23,
                  5429,
                  167,
                  47.36,
                  0.2056,
                  7.004,
                  4222.6,
                  0,
                  "Very thin exosphere"};
}
void renderMercury(double radius) {
    glPushMatrix();
    glRotated(planets[0].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[0].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[0].flattening, 1);
    ball(0, radius);
    glPopMatrix();
    glPopMatrix();
}

SurfaceColor mercuryPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 7, 11), fine = terrain(p, 55, 17, 3);
    SurfaceColor base = blend(color(100, 98, 94), color(170, 163, 151), n);
    double basin = pow(wrapAngle(lon - 2.65) / .55, 2) + pow((lat - .52) / .42, 2);
    if (basin < 1)
        base = blend(base, color(127, 119, 106), .42 * (1 - basin));
    return shade(base, .88 + fine * .25);
}
