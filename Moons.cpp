#include "SolarSystem.h"
Moon moons[16] = {
    {"Moon", 2, 2.1, 27.322, 20, .72f, .72f, .69f, 1737.4, 4902.8, 3.344, 384400, 5.16},
    {"Phobos", 3, 2.0, 0.31891, 30, .55f, .48f, .41f, 11.08, 0.0007087, 1.872, 9375, 1.1},
    {"Deimos", 3, 3.0, 1.26244, 190, .65f, .59f, .51f, 6.2, 9.62e-05, 1.471, 23457, 1.8},
    {"Io", 4, 2.1, 1.769138, 10, .92f, .77f, .31f, 1821.49, 5959.91547, 3.5276, 421800, 0},
    {"Europa", 4, 3.0, 3.551181, 105, .85f, .79f, .65f, 1560.8, 3202.7121, 3.013, 671100, 0.5},
    {"Ganymede", 4, 4.0, 7.154553, 210, .59f, .54f, .48f, 2631.2, 9887.83275, 1.9416, 1070400, 0.2},
    {"Callisto", 4, 5.0, 16.689017, 315, .38f, .36f, .34f, 2410.3, 7179.2834, 1.834, 1882700, 0.3},
    {"Enceladus", 5, 2.8, 1.370218, 30, .91f, .94f, .97f, 252.1, 7.21037, 1.6097, 238400, 0},
    {"Rhea", 5, 3.8, 4.5175, 170, .76f, .77f, .78f, 763.5, 153.94175, 1.2372, 527200, 0.3},
    {"Titan", 5, 5.0, 15.945421, 280, .85f, .61f, .25f, 2574.76, 8978.1371, 1.8814, 1221900, 0.3},
    {"Miranda", 6, 2.0, 1.413479, 0, .72f, .72f, .72f, 235.8, 4.3, 1.178, 129846, 4.4},
    {"Ariel", 6, 2.7, 2.520379, 72, .81f, .82f, .83f, 578.9, 83.5, 1.539, 190929, 0},
    {"Umbriel", 6, 3.4, 4.144177, 144, .43f, .44f, .45f, 584.7, 85.1, 1.523, 265986, 0.1},
    {"Titania", 6, 4.2, 8.705869, 216, .70f, .68f, .66f, 788.9, 226.9, 1.653, 436298, 0.1},
    {"Oberon", 6, 5.0, 13.463237, 288, .57f, .54f, .51f, 761.4, 205.3, 1.664, 583511, 0.1},
    {"Triton", 7, 3.2, -5.876994, 60, .76f, .80f, .79f, 1352.6, 1428.49546, 2.0649, 354800, 157.3}};
const char *moonDetails[16] = {"Cratered rocky natural satellite.",
                               "Small, irregular and dark.",
                               "Small, irregular and dark.",
                               "Volcanic moon with a yellowish surface.",
                               "Icy moon with an ocean beneath its crust.",
                               "Largest moon in the solar system.",
                               "Dark surface covered with old craters.",
                               "Bright icy moon with water-rich plumes.",
                               "Icy, heavily cratered natural satellite.",
                               "Large moon with a thick atmosphere.",
                               "Small moon with a rugged icy surface.",
                               "Icy natural satellite of Uranus.",
                               "Dark natural satellite of Uranus.",
                               "Largest natural satellite of Uranus.",
                               "Outer major natural satellite of Uranus.",
                               "Icy moon with a retrograde orbit."};

double moonOrbitalAngle(int i, double t) {
    return fmod(moons[i].phase + fmod(t, fabs(moons[i].period)) * 360 / moons[i].period, 360);
}
void moonOrientation(int i, double t) {
    const Moon &m = moons[i];
    double tilt = m.parent == 2 ? 0 : planets[m.parent].tilt;
    double inclination = m.inclination > 90 ? 180 - m.inclination : m.inclination;
    glRotated(tilt, 0, 0, 1);
    glRotated(-inclination, 1, 0, 0);
    glRotated(-moonOrbitalAngle(i, t), 0, 1, 0);
}
void drawMoon(int i, double radius) {
    glPushMatrix();
    moonOrientation(i, days);
    if (moons[i].parent == 3)
        glScaled(1.17, .82, 1);
    ball(i + 9, radius);
    glPopMatrix();
}

SurfaceColor moonPixel(int i, double lon, double lat) {
    Vec p = spherePoint(lon, lat);
    double n = terrain(p, 6, 109 + i * 13), fine = terrain(p, 52, 127 + i * 17, 3);
    SurfaceColor base = shade(SurfaceColor{moons[i].r, moons[i].g, moons[i].b}, .71 + .42 * n);
    if (i == 0) {
        base = blend(color(124, 125, 124), color(190, 188, 182), n);
        const double seas[][4] = {{-.30, .55, .35, .27},
                                  {.28, .36, .30, .34},
                                  {.55, .14, .22, .34},
                                  {-.45, -.15, .30, .24},
                                  {-.88, .25, .30, .45}};
        for (int k = 0; k < 5; k++) {
            double dx = wrapAngle(lon - seas[k][0]) / seas[k][2],
                   dy = (lat - seas[k][1]) / seas[k][3];
            double r = sqrt(dx * dx + dy * dy) + .16 * (n - .5);
            base = blend(base, color(75, 78, 79), 1 - smooth(.72, 1.12, r));
        }
    }
    if (i == 3) {
        base = blend(color(174, 141, 65), color(235, 215, 125), n);
        double lava = smooth(.69, .80, terrain(p, 22, 211, 3));
        base = blend(base, color(66, 53, 39), lava);
        double hot = smooth(.82, .91, terrain(p, 9, 223, 3));
        base = blend(base, color(171, 70, 34), hot);
    }
    if (i == 4 || i == 7) {
        base = blend(color(190, 181, 160), color(236, 232, 216), n);
        double crack = fabs(sin(lon * 12 + 2.6 * sin(lat * 8) + n * 4));
        double crack2 = fabs(sin(lat * 18 + sin(lon * 7) * 1.4));
        double lines = 1 - smooth(.018, .065, std::min(crack, crack2));
        base = blend(base, i == 4 ? color(138, 104, 79) : color(132, 168, 180), lines * .65);
    }
    if (i == 5) {
        double grooves = 1 - smooth(.015, .07, fabs(sin(lon * 19 + sin(lat * 11) + n * 3)));
        base = blend(base, color(153, 148, 132), grooves * .35);
    }
    if (i == 9) {
        base = blend(color(173, 119, 46), color(225, 174, 85), .45 + .25 * sin(lat * 11) + .20 * n);
    }
    if (i == 10) {
        double cliffs = 1 - smooth(.02, .09, fabs(sin(lon * 11 + lat * 8 + n * 4)));
        base = blend(base, color(99, 103, 106), cliffs * .45);
    }
    if (i == 15) {
        base = blend(color(176, 163, 155), color(217, 218, 205), n);
        base = blend(base, color(226, 225, 216), smooth(.65, 1.25, -lat));
    }
    return shade(base, .86 + .25 * fine);
}
