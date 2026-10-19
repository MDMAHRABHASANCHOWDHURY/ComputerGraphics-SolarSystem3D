#include "../SolarSystem.h"
Planet makeJupiter() {
    return Planet{"Jupiter",
                  "Gas giant",
                  "Largest planet; striped cloud belts.",
                  "The Great Red Spot is a giant storm.",
                  142984,
                  5.20336301,
                  4332.589,
                  9.925,
                  3.13,
                  23.1,
                  1.62,
                  15.3,
                  245,
                  1.89813e+27,
                  1326,
                  -110,
                  13.06,
                  0.0487,
                  1.304,
                  9.9259,
                  0.06487,
                  "Hydrogen and helium"};
}
void renderJupiter(double radius) {
    glPushMatrix();
    glRotated(planets[4].tilt, 0, 0, 1);
    glRotated(fmod(days * 24 / fabs(planets[4].rotation) * 360, 360), 0, 1, 0);
    glPushMatrix();
    glScaled(1, 1 - planets[4].flattening, 1);
    ball(4, radius);
    glPopMatrix();
    rings(radius, 4);
    glPopMatrix();
}

SurfaceColor jupiterPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 13, 53, 4), detail = terrain(p, 65, 59, 2);
    double warped = lat + .033 * (n - .5) + .010 * sin(lon * 12 + lat * 7);
    double band =
        .5 + .27 * sin(warped * 23) + .13 * sin(warped * 51 + .6) + .10 * sin(warped * 87);
    SurfaceColor base = blend(color(156, 115, 81), color(239, 226, 204), smooth(.13, .84, band));
    base = shade(base, .88 + .23 * detail);
    double dx = wrapAngle(lon - .35) / .25, dy = (lat + .38) / .105;
    double r = sqrt(dx * dx + dy * dy);
    if (r < 1.25) {
        double spiral = .5 + .5 * sin(r * 24 - atan2(dy, dx) * 2);
        SurfaceColor storm = blend(color(155, 66, 42), color(210, 131, 88), spiral * .65);
        base = blend(base, storm, 1 - smooth(.88, 1.25, r));
    }
    return base;
}
