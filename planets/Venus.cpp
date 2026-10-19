#include "../SolarSystem.h"
Planet makeVenus() {
    return Planet{"Venus",
                  "Rocky planet",
                  "Thick clouds hide the rocky surface.",
                  "Dense CO2 atmosphere; very hot.",
                  12104,
                  0.72333199,
                  224.701,
                  -5832.6,
                  177.36,
                  8.9,
                  .72,
                  6.6,
                  75,
                  4.867e+24,
                  5243,
                  464,
                  35.02,
                  0.0068,
                  3.395,
                  2802,
                  0,
                  "Carbon dioxide and nitrogen"};
}
void renderVenus(double radius) {
    glPushMatrix();
    glRotated(planets[1].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[1].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[1].flattening, 1);
    ball(1, radius);
    glPopMatrix();
    glPopMatrix();
}

SurfaceColor venusPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double swirl = terrain(p, 4, 23), fine = terrain(p, 27, 29, 3);
    double cloud = .5 + .25 * sin(lat * 13 + swirl * 5 + sin(lon * 3)) + .25 * (fine - .5);
    return blend(color(204, 178, 124), color(246, 231, 193), clamp(cloud, 0, 1));
}
