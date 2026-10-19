#include "../SolarSystem.h"
Planet makeNeptune() {
    return Planet{"Neptune",
                  "Ice giant",
                  "Blue atmosphere with cloud bands.",
                  "Powerful winds; color is illustrative.",
                  49528,
                  30.178,
                  60189.018,
                  16.11,
                  28.32,
                  11,
                  .96,
                  29.5,
                  40,
                  1.02413e+26,
                  1638,
                  -200,
                  5.45,
                  0.0097,
                  1.77,
                  16.11,
                  0.01708,
                  "Hydrogen, helium and methane"};
}
void renderNeptune(double radius) {
    glPushMatrix();
    glRotated(planets[7].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[7].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[7].flattening, 1);
    ball(7, radius);
    glPopMatrix();
    rings(radius, 7);
    glPopMatrix();
}

SurfaceColor neptunePixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 16, 83, 3);
    double belts = .5 + .18 * sin(lat * 22 + (n - .5) * .6);
    SurfaceColor base = blend(color(76, 132, 167), color(112, 169, 194), belts);
    double streak = smooth(.78, .9, terrain(p, 9, 89, 3)) * smooth(.15, .5, fabs(lat));
    return blend(base, color(205, 224, 232), streak * .6);
}
