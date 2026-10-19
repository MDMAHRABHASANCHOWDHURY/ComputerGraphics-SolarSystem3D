#include "../SolarSystem.h"
Planet makeUranus() {
    return Planet{"Uranus",
                  "Ice giant",
                  "Methane gives a pale cyan color.",
                  "Its rotation axis lies almost sideways.",
                  51118,
                  19.165,
                  30685.4,
                  -17.24,
                  97.77,
                  8.7,
                  .99,
                  25.3,
                  345,
                  8.681e+25,
                  1270,
                  -195,
                  6.79,
                  0.0469,
                  0.77,
                  17.24,
                  0.02293,
                  "Hydrogen, helium and methane"};
}
void renderUranus(double radius) {
    glPushMatrix();
    glRotated(planets[6].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[6].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[6].flattening, 1);
    ball(6, radius);
    glPopMatrix();
    rings(radius, 6);
    glPopMatrix();
}

SurfaceColor uranusPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 14, 79, 3);
    double band = .5 + .22 * sin(lat * 18) + .10 * (n - .5);
    return blend(color(138, 193, 194), color(161, 210, 207), band);
}
