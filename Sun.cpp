#include "SolarSystem.h"
void drawSun(double radius) {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glRotated(7.25, 0, 0, 1);
    glRotated(fmod(days * 360 / 25.38, 360), 0, 1, 0);
    ball(8, radius);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

SurfaceColor sunPixel(double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double granules = terrain(p, 75, 97, 3), large = terrain(p, 9, 101, 3);
    SurfaceColor base = blend(color(235, 118, 12), color(255, 207, 67), granules);
    double spots = smooth(.76, .86, large) * (1 - smooth(.55, 1.15, fabs(lat)));
    return blend(base, color(82, 42, 18), spots * .85);
}
