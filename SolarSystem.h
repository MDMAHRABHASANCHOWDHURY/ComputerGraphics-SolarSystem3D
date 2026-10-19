#ifndef SOLAR_SYSTEM_H
#define SOLAR_SYSTEM_H
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
const double PI = 3.14159265358979323846, AU = 149597870.7;
struct Vec {
    double x, y, z;
};
struct SurfaceColor {
    double r, g, b;
};
struct Planet {
    const char *name, *kind, *line1, *line2;
    double diameter, au, period, rotation, tilt, gravity;
    double radius, orbit, phase;
    double mass, density, temperature, orbitalSpeed, eccentricity, inclination, dayHours,
        flattening;
    const char *atmosphere;
};
struct Moon {
    const char *name;
    int parent;
    double orbitRatio, period, phase;
    float r, g, b;
    double meanRadius, gm, density, semiMajor, inclination;
};

extern Planet planets[8];
extern Moon moons[16];
extern const char *moonDetails[16];
extern double days;
extern GLuint textures[25];
extern GLUquadric *sphere;
double rad(double a);
double clamp(double a, double lo, double hi);
std::string number(double v, int n = 1);
void makeTexture(int index);
void ball(int texture, double radius);
void drawPlanet(int i, double radius);
void rings(double radius, int planet);
double moonOrbitalAngle(int i, double t);
void moonOrientation(int i, double t);
void drawMoon(int i, double radius);
void drawSun(double radius);
Planet makeMercury();
void renderMercury(double radius);
Planet makeVenus();
void renderVenus(double radius);
Planet makeEarth();
void renderEarth(double radius);
Planet makeMars();
void renderMars(double radius);
Planet makeJupiter();
void renderJupiter(double radius);
Planet makeSaturn();
void renderSaturn(double radius);
Planet makeUranus();
void renderUranus(double radius);
Planet makeNeptune();
void renderNeptune(double radius);
SurfaceColor color(double r, double g, double b);
SurfaceColor blend(SurfaceColor a, SurfaceColor b, double t);
SurfaceColor shade(SurfaceColor a, double t);
double smooth(double a, double b, double x);
double wrapAngle(double a);
Vec spherePoint(double lon, double lat);
double surfaceNoise(double x, double y, double z, int seed);
double terrain(Vec p, double frequency, int seed, int octaves = 4);
void craterPixels(std::vector<unsigned char> &pixels, int w, int h, int seed, int count,
                  double strength);
SurfaceColor sunPixel(double lon, double lat);
SurfaceColor moonPixel(int i, double lon, double lat);
SurfaceColor mercuryPixel(double lon, double lat);
SurfaceColor venusPixel(double lon, double lat);
SurfaceColor earthPixel(double lon, double lat);
SurfaceColor marsPixel(double lon, double lat);
SurfaceColor jupiterPixel(double lon, double lat);
SurfaceColor saturnPixel(double lon, double lat);
SurfaceColor uranusPixel(double lon, double lat);
SurfaceColor neptunePixel(double lon, double lat);
#endif
