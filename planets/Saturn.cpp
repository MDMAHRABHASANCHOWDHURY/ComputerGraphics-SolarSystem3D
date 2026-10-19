#include "../SolarSystem.h"
Planet makeSaturn() {
    return Planet{"Saturn",
                  "Gas giant",
                  "Broad rings made mostly of ice.",
                  "Pale gold cloud bands surround it.",
                  120536,
                  9.572,
                  10755.699,
                  10.656,
                  26.73,
                  9,
                  1.34,
                  20.5,
                  300,
                  5.6832e+26,
                  687,
                  -140,
                  9.67,
                  0.052,
                  2.486,
                  10.656,
                  0.09796,
                  "Hydrogen and helium"};
}
void renderSaturn(double radius) {
    glPushMatrix();
    glRotated(planets[5].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[5].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[5].flattening, 1);
    ball(5, radius);
    glPopMatrix();
    rings(radius, 5);
    glPopMatrix();
}

SurfaceColor saturnPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 12, 67), detail = terrain(p, 45, 71, 2);
    double belts = .5 + .28 * sin(lat * 28 + (n - .5) * 1.3) + .10 * sin(lat * 63);
    return shade(blend(color(190, 169, 123), color(235, 217, 174), belts), .95 + .10 * detail);
}
