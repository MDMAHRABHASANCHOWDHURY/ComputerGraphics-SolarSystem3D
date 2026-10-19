#include "SolarSystem.h"
void makeTexture(int index) {
    int w = index == 2 ? 1024 : 512, h = w / 2;
    std::vector<unsigned char> pixels(w * h * 3);
    SurfaceColor (*functions[])(double, double) = {mercuryPixel, venusPixel,   earthPixel,
                                          marsPixel,    jupiterPixel, saturnPixel,
                                          uranusPixel,  neptunePixel, sunPixel};
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            double lon = -PI + 2 * PI * (x + .5) / w, lat = PI / 2 - PI * (y + .5) / h;
            SurfaceColor c = index < 9 ? functions[index](lon, lat) : moonPixel(index - 9, lon, lat);
            size_t k = (size_t(y) * w + x) * 3;
            pixels[k] = (unsigned char)clamp(c.r * 255, 0, 255);
            pixels[k + 1] = (unsigned char)clamp(c.g * 255, 0, 255);
            pixels[k + 2] = (unsigned char)clamp(c.b * 255, 0, 255);
        }
    const int craters[] = {220, 100, 60, 0, 8, 120, 280, 25, 200, 0, 60, 100, 200, 130, 180, 40};
    if (index == 0)
        craterPixels(pixels, w, h, 41, 190, .30);
    if (index == 3)
        craterPixels(pixels, w, h, 43, 30, .13);
    if (index >= 9)
        craterPixels(pixels, w, h, 100 + index, craters[index - 9], .27);
    for (int y = 0; y < h / 2; y++)
        for (int x = 0; x < w * 3; x++)
            std::swap(pixels[size_t(y) * w * 3 + x], pixels[size_t(h - 1 - y) * w * 3 + x]);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
}
void ball(int index, double radius) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glColor3f(1, 1, 1);
    glPushMatrix();
    glRotated(-90, 1, 0, 0);
    gluSphere(sphere, radius, 96, 64);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
void rings(double radius, int planet) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int count = planet == 5 ? 400 : 3;
    for (int b = 0; b < count; b++) {
        double r = planet == 5 ? 1.11 + b * 1.20 / count : 1.8 + b * .012;
        double alpha = .18, brightness = .6;
        if (planet == 5) {
            alpha = r < 1.52 ? .19 : r < 1.95 ? .88 : r < 2.03 ? .015 : r < 2.27 ? .66 : .08;
            if (fabs(r - 2.215) < .005)
                alpha = .035;
            brightness = .69 + .11 * sin(r * 420) + .06 * sin(r * 1190);
            glColor4d(brightness, brightness * .92, brightness * .76, alpha);
        } else
            glColor4f(.4f, .4f, .37f, planet == 4 ? .12f : .25f);
        double inner = radius * r, outer = inner + radius * (planet == 5 ? 1.20 / count : .003);
        glBegin(GL_QUAD_STRIP);
        for (int k = 0; k <= 200; k++) {
            double a = 2 * PI * k / 200;
            glVertex3d(inner * cos(a), 0, inner * sin(a));
            glVertex3d(outer * cos(a), 0, outer * sin(a));
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
void drawPlanet(int i, double radius) {
    void (*render[])(double) = {renderMercury, renderVenus,  renderEarth,  renderMars,
                                renderJupiter, renderSaturn, renderUranus, renderNeptune};
    if (i >= 0 && i < 8)
        render[i](radius);
}
