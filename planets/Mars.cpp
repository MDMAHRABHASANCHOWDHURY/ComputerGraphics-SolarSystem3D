#include "../SolarSystem.h"
Planet makeMars() {
    return Planet{"Mars",
                  "Rocky planet",
                  "Iron-rich dust gives a rusty color.",
                  "Thin atmosphere and polar ice caps.",
                  6792,
                  1.52366231,
                  686.98,
                  24.6229,
                  25.19,
                  3.7,
                  .56,
                  11.3,
                  200,
                  6.4171e+23,
                  3934,
                  -65,
                  24.08,
                  0.0935,
                  1.848,
                  24.6597,
                  0.00589,
                  "Thin carbon dioxide atmosphere"};
}
void renderMars(double radius) {
    glPushMatrix();
    glRotated(planets[3].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[3].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[3].flattening, 1);
    ball(3, radius);
    glPopMatrix();
    glPopMatrix();
}

SurfaceColor marsPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 4, 41), fine = terrain(p, 45, 43, 3);
    SurfaceColor base = blend(color(103, 66, 46), color(196, 122, 76), smooth(.30, .65, n));
    double canyon = fabs(sin(lon * 3 + .26 * sin(lat * 9)));
    if (lat < .10 && lat > -.30 && canyon < .045)
        base = shade(base, .65);
    double ice = smooth(1.15, 1.42, fabs(lat) + .07 * sin(lon * 5));
    return blend(shade(base, .86 + fine * .30), color(226, 221, 205), ice);
}
