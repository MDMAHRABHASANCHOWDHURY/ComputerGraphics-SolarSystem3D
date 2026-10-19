#include "SolarSystem.h"

Planet planets[8] = {makeMercury(), makeVenus(),  makeEarth(),  makeMars(),
                     makeJupiter(), makeSaturn(), makeUranus(), makeNeptune()};
GLuint textures[25];
GLUquadric *sphere = 0;
double days = 0, speed = 1.0 / 24, yaw = 18, pitch = 40, distanceNow = 86, distanceGoal = 86;
Vec center = {0, 0, 0}, eye = {0, 0, 0};
int width = 1280, height = 800, selection = -1, hover = -1, lastTime = 0, mouseX = 0, mouseY = 0;
int dragStartX = 0, dragStartY = 0;
bool paused = false, showMoons = true, showOrbits = true, showInfo = false, fullScreen = true;
bool dragging = false, moved = false, traveling = false;
double travelTime = 0;
enum SceneType { Overview, PlanetScene, MoonScene, SunScene };
SceneType currentScene = Overview;
int infoTab = 0;
struct SceneCamera {
    double yaw, pitch, distance;
    bool visited;
};
SceneCamera sceneCameras[26] = {};
void rememberCamera() { sceneCameras[selection + 1] = SceneCamera{yaw, pitch, distanceGoal, true}; }

double model[16], projection[16];
GLint viewport[4];
std::vector<Vec> stars;
struct Pick {
    int id;
    double x, y, z, radius;
};
std::vector<Pick> picks;
struct Button {
    float x, y, w, h;
    std::string label;
    int action;
};
std::vector<Button> buttons;
float infoX = 0, infoY = 0, infoW = 0, infoH = 0;
bool infoVisible = false;
bool insideInfo(float x, float y) {
    return infoVisible && x >= infoX && x <= infoX + infoW && y >= infoY && y <= infoY + infoH;
}

double rad(double a) { return a * PI / 180; }
double clamp(double a, double lo, double hi) { return std::max(lo, std::min(hi, a)); }
std::string number(double v, int n) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(n) << v;
    return s.str();
}
Vec add(Vec a, Vec b) { return Vec{a.x + b.x, a.y + b.y, a.z + b.z}; }
double length(Vec p) { return sqrt(p.x * p.x + p.y * p.y + p.z * p.z); }
Vec planetPosition(int i, double t, bool physical = false) {
    Planet &p = planets[i];
    double mean = rad(p.phase) + 2 * PI * t / p.period;
    double E = mean;
    for (int k = 0; k < 8; k++)
        E -= (E - p.eccentricity * sin(E) - mean) / (1 - p.eccentricity * cos(E));
    double a = physical ? p.au : p.orbit;
    double x = a * (cos(E) - p.eccentricity),
           z = a * sqrt(1 - p.eccentricity * p.eccentricity) * sin(E);
    return Vec{x, z * sin(rad(p.inclination)), z * cos(rad(p.inclination))};
}
Vec moonOffset(int i, double t) {
    Moon &m = moons[i];
    double a = rad(moonOrbitalAngle(i, t));
    double r = planets[m.parent].radius * m.orbitRatio;
    double inc = rad(m.inclination > 90 ? 180 - m.inclination : m.inclination);
    Vec p = {r * cos(a), r * sin(a) * sin(inc), r * sin(a) * cos(inc)};
    double tilt = m.parent == 2 ? 0 : rad(planets[m.parent].tilt);
    return Vec{p.x * cos(tilt) - p.y * sin(tilt), p.x * sin(tilt) + p.y * cos(tilt), p.z};
}
Vec moonPhysicalOffset(int i, double t) {
    Vec offset = moonOffset(i, t);
    double scale = moons[i].semiMajor / (AU * length(offset));
    return Vec{offset.x * scale, offset.y * scale, offset.z * scale};
}
Vec observerPosition(int id, double t) {
    if (id == 8) return Vec{0, 0, 0};
    if (id < 8) return planetPosition(id, t, true);
    int i = id - 9;
    return add(planetPosition(moons[i].parent, t, true), moonPhysicalOffset(i, t));
}
Vec bodyPosition(int id) {
    if (id < 0 || id == 8)
        return Vec{0, 0, 0};
    if (currentScene == Overview) {
        if (id < 8)
            return planetPosition(id, days);
        int i = id - 9;
        return add(planetPosition(moons[i].parent, days), moonOffset(i, days));
    }
    if (currentScene == PlanetScene && id >= 9)
        return moonOffset(id - 9, days);
    return Vec{0, 0, 0};
}
double bodyRadius(int id) {
    if (id == 8)
        return 2.6;
    if (id < 0)
        return 1;
    if (id < 8)
        return planets[id].radius;
    Moon &m = moons[id - 9];
    return planets[m.parent].radius *
           std::max(.045, m.meanRadius / (planets[m.parent].diameter * .5));
}
int parentPlanet(int id) { return id >= 9 ? moons[id - 9].parent : (id < 8 ? id : -1); }
std::string bodyName(int id) {
    return id == 8   ? "Sun"
           : id >= 9 ? moons[id - 9].name
           : id >= 0 ? planets[id].name
                     : "Solar System";
}
double earthDistance(int id, double t) {
    Vec e = planetPosition(2, t, true), p = {0, 0, 0};
    if (id < 8 && id >= 0)
        p = planetPosition(id, t, true);
    if (id >= 9)
        p = observerPosition(id, t);
    return length(Vec{e.x - p.x, e.y - p.y, e.z - p.z}) * AU;
}
void focus(int id) {
    if (id < 0 || id > 24)
        return;
    rememberCamera();
    selection = id;
    infoTab = 0;
    showInfo = true;
    traveling = true;
    travelTime = 0;
    currentScene = id < 8 ? PlanetScene : id == 8 ? SunScene : MoonScene;
    center = Vec{0, 0, 0};
    SceneCamera &cam = sceneCameras[id + 1];
    Vec orbitPosition = id < 8 ? planetPosition(id, days, true) : Vec{0, 0, 0};
    double initialYaw = id < 8 ? atan2(orbitPosition.x, orbitPosition.z) * 180 / PI - 20 : 18;
    double initialPitch = id < 8 ? 5 : 18;
    if (id >= 9) {
        Vec offset = moonPhysicalOffset(id - 9, days);
        initialYaw = atan2(offset.x, offset.z) * 180 / PI - 20;
        initialPitch = clamp(asin(offset.y / length(offset)) * 180 / PI, -85, 85);
    }
    yaw = cam.visited ? cam.yaw : initialYaw;
    pitch = cam.visited ? cam.pitch : initialPitch;
    distanceGoal = cam.visited ? cam.distance : bodyRadius(id) * 8.5;
    distanceNow = distanceGoal * 1.35;
    if (id >= 9)
        showMoons = true;
}
void home() {
    rememberCamera();
    selection = -1;
    currentScene = Overview;
    showInfo = false;
    traveling = true;
    travelTime = 0;
    center = Vec{0, 0, 0};
    SceneCamera &cam = sceneCameras[0];
    yaw = cam.visited ? cam.yaw : 18;
    pitch = cam.visited ? cam.pitch : 40;
    distanceGoal = cam.visited ? cam.distance : 86;
    distanceNow = distanceGoal * 1.1;
}
void changePlanet(int direction) {
    if (currentScene == MoonScene) {
        focus(9 + (selection - 9 + direction + 16) % 16);
        return;
    }
    int parent = parentPlanet(selection);
    if (parent < 0)
        parent = direction > 0 ? -1 : 0;
    focus((parent + direction + 8) % 8);
}
void toggleFullScreen() {
    fullScreen = !fullScreen;
    if (fullScreen)
        glutFullScreen();
    else {
        glutReshapeWindow(1280, 800);
        glutPositionWindow(50, 50);
    }
}
void action(int id) {
    if (id >= 100) {
        focus(id - 100);
        return;
    }
    switch (id) {
    case 17:
        infoTab = 0;
        break;
    case 18:
        infoTab = 1;
        break;
    case 0:
        home();
        break;
    case 1:
        changePlanet(-1);
        break;
    case 2:
        changePlanet(1);
        break;
    case 3:
        if (selection >= 0)
            focus(selection);
        else
            focus(2);
        distanceGoal = bodyRadius(selection) * 8.5;
        break;
    case 4:
        if (selection >= 0) {
            distanceGoal = bodyRadius(selection) * (selection == 5 ? 5.5 : 4.2);
            showInfo = false;
        }
        break;
    case 5:
        distanceGoal =
            std::max(selection >= 0 ? bodyRadius(selection) * 2.4 : .5, distanceGoal * .8);
        break;
    case 6:
        distanceGoal = std::min(250.0, distanceGoal * 1.25);
        break;
    case 7: {
        int p = parentPlanet(selection);
        if (p >= 0) {
            focus(p);
            showMoons = true;
            distanceGoal = planets[p].radius * 19;
        }
        break;
    }
    case 8:
        showMoons = !showMoons;
        if (!showMoons && selection >= 9)
            focus(moons[selection - 9].parent);
        break;
    case 9:
        if (selection >= 0)
            showInfo = !showInfo;
        if (showInfo && selection == 5)
            distanceGoal = std::max(distanceGoal, bodyRadius(5) * 8.5);
        break;
    case 10:
        speed = 1.0 / 86400;
        break;
    case 11:
        speed = 1.0 / 24;
        break;
    case 12:
        speed = 1;
        break;
    case 13:
        speed = 30;
        break;
    case 14:
        paused = !paused;
        break;
    case 15:
        showInfo = false;
        break;
    case 16:
        showOrbits = !showOrbits;
        break;
    }
    glutPostRedisplay();
}
void text(float x, float y, const std::string &s, int size = 12) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size * .0068f, -size * .0084f, 1);
    glLineWidth(1);
    for (size_t i = 0; i < s.size(); i++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, (unsigned char)s[i]);
    glPopMatrix();
}
void rect(float x, float y, float w, float h, float r, float g, float b, float a = 1) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}
void button(float x, float y, float w, const std::string &label, int id, bool active = false) {
    buttons.push_back(Button{x, y, w, 32, label, id});
    rect(x, y, w, 32, active ? .17f : .065f, active ? .35f : .105f, active ? .44f : .15f, .94f);
    glColor3f(active ? .97f : .79f, active ? .86f : .87f, active ? .59f : .94f);
    text(x + 10, y + 21, label);
}
void projectBody(int id, Vec p, double r) {
    double x, y, z;
    gluProject(p.x, p.y, p.z, model, projection, viewport, &x, &y, &z);
    double d = length(Vec{p.x - eye.x, p.y - eye.y, p.z - eye.z});
    double pixels = r * height / (2 * tan(rad(44) / 2) * std::max(.01, d));
    picks.push_back(Pick{id, x * 1280 / width, (height - y) * 800 / height, z,
                         std::max(6.0, pixels * 1280 / width)});
}
void planetOrbit(int i) {
    glDisable(GL_LIGHTING);
    glColor3f(.095f, .145f, .20f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k < 200; k++) {
        Vec p = planetPosition(i, planets[i].period * k / 200);
        glVertex3d(p.x, p.y, p.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}
void moonOrbit(int i) {
    glDisable(GL_LIGHTING);
    glColor3f(.095f, .145f, .20f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k < 180; k++) {
        Vec p = moonOffset(i, moons[i].period * k / 180);
        glVertex3d(p.x, p.y, p.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}
std::string scientific(double value) {
    std::ostringstream o;
    o << std::scientific << std::setprecision(3) << value;
    return o.str();
}

void card() {
    if (!showInfo || selection < 0 || traveling)
        return;
    Pick anchor = {selection, 640, 400, 0, 20};
    for (size_t i = 0; i < picks.size(); i++)
        if (picks[i].id == selection)
            anchor = picks[i];
    if (selection == 5)
        anchor.radius *= 2.31;
    int count = 0;
    if (selection < 8)
        for (int i = 0; i < 16; i++)
            if (moons[i].parent == selection)
                ++count;
    float h = selection < 8    ? (infoTab == 0 ? 620.f : 190.f + count * 34)
              : selection == 8 ? 390.f
                               : 530.f;
    float x = (float)(anchor.x + anchor.radius + 28);
    if (x + 386 > 1264)
        x = (float)(anchor.x - anchor.radius - 414);
    x = (float)clamp(x, 16, 878);
    float y = (float)clamp(anchor.y - h * .42, 110, 730 - h);
    infoX = x;
    infoY = y;
    infoW = 386;
    infoH = h;
    infoVisible = true;
    rect(x, y, 386, h, .025f, .045f, .065f, .95f);
    rect(x, y, 3, h, .37f, .72f, .84f);
    glColor3f(.97f, .85f, .57f);
    text(x + 18, y + 29, bodyName(selection), 18);
    button(x + 344, y + 7, 32, "X", 15);
    glColor3f(.76f, .84f, .90f);
    if (selection < 8) {
        Planet &p = planets[selection];
        text(x + 18, y + 51, p.kind);
        button(x + 18, y + 64, 120, "Facts", 17, infoTab == 0);
        button(x + 146, y + 64, 120, "Moons", 18, infoTab == 1);
        glColor3f(.78f, .85f, .91f);
        if (infoTab == 1) {
            if (count == 0)
                text(x + 18, y + 127, "This planet has no natural moons.");
            float row = y + 110;
            for (int i = 0; i < 16; i++)
                if (moons[i].parent == selection) {
                    button(x + 18, row, 350, moons[i].name, 109 + i);
                    row += 34;
                }
            glColor3f(.47f, .58f, .66f);
            text(x + 18, y + h - 35, "Choose a moon to open its separate scene.");
            text(x + 18, y + h - 15, "A selected set of major moons is included.");
        } else {
            std::vector<std::string> rows = {
                "Equatorial diameter: " + number(p.diameter, 0) + " km",
                "Mass: " + scientific(p.mass) + " kg",
                "Density: " + number(p.density, 0) + " kg/m^3    Gravity: " + number(p.gravity, 2) +
                    " m/s^2",
                "Sidereal year: " + number(p.period, 3) + " Earth days",
                "Sidereal spin: " + number(fabs(p.rotation), 4) + " Earth hours",
                "Rotation rate: " + number(360 / fabs(p.rotation), 4) + " degrees/hour",
                "Equator rotation speed: " + number(PI * p.diameter / fabs(p.rotation), 2) +
                    " km/hour",
                std::string("Rotation: ") + (p.rotation < 0 ? "retrograde" : "prograde") +
                    "    Tilt: " + number(p.tilt, 2) + " deg",
                "Mean orbital speed: " + number(p.orbitalSpeed, 2) + " km/s",
                "Mean Sun distance: " + number(p.au, 4) + " AU",
                "Perihelion / aphelion: " + number(p.au * AU * (1 - p.eccentricity) / 1e6, 2) +
                    " / " + number(p.au * AU * (1 + p.eccentricity) / 1e6, 2) + " M km",
                "Orbit eccentricity: " + number(p.eccentricity, 4),
                "Orbit inclination: " + number(p.inclination, 3) + " deg",
                "Solar day: " + number(p.dayHours, 4) + " Earth hours",
                "Reference mean temperature: " + number(p.temperature, 0) + " C",
                "Model Earth distance: " + number(earthDistance(selection, days) / 1e6, 2) +
                    " million km"};
            float row = y + 119;
            for (size_t i = 0; i < rows.size(); i++) {
                text(x + 18, row, rows[i]);
                row += 23;
            }
            glColor3f(.62f, .77f, .85f);
            text(x + 18, y + 510, p.atmosphere);
            text(x + 18, y + 536, p.line1);
            text(x + 18, y + 558, p.line2);
            glColor3f(.47f, .58f, .66f);
            text(x + 18, y + 593, "Surface: C++ generated. Sizes/spacing compressed.");
            text(x + 18, y + 613, "Model positions are not a live ephemeris.");
        }
    } else if (selection == 8) {
        text(x + 18, y + 66, "The star at the center of our solar system.");
        text(x + 18, y + 94, "Mean diameter: 1391400 km");
        text(x + 18, y + 118, "Mass: 1.9884e30 kg");
        text(x + 18, y + 142, "Density: 1408 kg/m^3   Gravity: 274 m/s^2");
        text(x + 18, y + 166, "Effective temperature: 5772 K");
        text(x + 18, y + 190,
             "Model Earth distance: " + number(earthDistance(8, days) / 1e6, 2) + " million km");
        text(x + 18, y + 214, "Rotation model: 25.38 Earth days / 609.12 hours");
        text(x + 18, y + 238, "Rotation rate: " + number(360 / 609.12, 4) + " degrees/hour");
        text(x + 18, y + 262, "Reference tilt: 7.25 degrees   Spectral class: G2 V");
        button(x + 18, y + 292, 160, "Open Earth scene", 102);
        glColor3f(.47f, .58f, .66f);
        text(x + 18, y + 357, "Solar rotation varies with latitude.");
        text(x + 18, y + 378, "The adopted model period is for 16 deg latitude.");
    } else {
        int i = selection - 9;
        Moon &m = moons[i];
        text(x + 18, y + 56, "Natural satellite of " + std::string(planets[m.parent].name));
        std::vector<std::string> rows = {
            "Mean diameter: " + number(m.meanRadius * 2, 2) + " km",
            "Mass: " + scientific(m.gm * 1e9 / 6.67430e-11) + " kg",
            "Density: " + number(m.density * 1000, 1) + " kg/m^3",
            "Calculated gravity: " + number(m.gm * 1000 / (m.meanRadius * m.meanRadius), 4) +
                " m/s^2",
            "Mean parent distance: " + number(m.semiMajor, 0) + " km",
            "Orbit period: " + number(fabs(m.period), 6) + " Earth days",
            "Spin model: " + number(fabs(m.period) * 24, 4) + " Earth hours",
            "Rotation rate: " + number(360 / (fabs(m.period) * 24), 4) + " degrees/hour",
            "Equator spin speed: " + number(2 * PI * m.meanRadius / (fabs(m.period) * 24), 3) +
                " km/hour",
            "Mean circular speed: " + number(2 * PI * m.semiMajor / (fabs(m.period) * 86400), 3) +
                " km/s",
            "Orbital inclination: " + number(m.inclination, 2) + " deg",
            "Model Earth distance: " + number(earthDistance(selection, days) / 1e6, 4) +
                " million km",
            std::string("Orbit direction: ") + (m.period < 0 ? "retrograde" : "prograde")};
        float row = y + 88;
        for (size_t j = 0; j < rows.size(); j++) {
            text(x + 18, row, rows[j]);
            row += 22;
        }
        glColor3f(.62f, .77f, .85f);
        text(x + 18, y + 391, moonDetails[i]);
        button(x + 18, y + 410, 250, "Open " + std::string(planets[m.parent].name) + " scene",
               100 + m.parent);
        glColor3f(.47f, .58f, .66f);
        text(x + 18, y + 467, "Spin is modeled as synchronous with the orbit.");
        text(x + 18, y + 490, "Sizes and satellite spacing are enlarged to see.");
        text(x + 18, y + 515, "Surface: calculated by C++ pixel functions.");
    }
}

void overlay() {
    infoVisible = false;
    buttons.clear();
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 800, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rect(0, 0, 1280, 90, .012f, .023f, .038f, .82f);
    glColor3f(.68f, .82f, .89f);
    long long wholeDays = (long long)days;
    int seconds = (int)((days - wholeDays) * 86400);
    std::ostringstream clock;
    clock << "Earth elapsed  " << wholeDays << "d  " << std::setfill('0') << std::setw(2)
          << seconds / 3600 << ":" << std::setw(2) << (seconds / 60) % 60 << ":" << std::setw(2)
          << seconds % 60;
    text(20, 54, clock.str());
    std::string sceneTitle =
        currentScene == Overview
            ? "Solar System / Overview"
            : bodyName(selection) + (currentScene == MoonScene  ? " / Moon scene"
                                     : currentScene == SunScene ? " / Sun scene"
                                                                : " / Planet scene");
    text(20, 28, sceneTitle, 18);
    std::string rate = speed < 1.0 / 3600 ? number(speed * 86400, 2) + " Earth seconds / second"
                       : speed < 1        ? number(speed * 24, 2) + " Earth hours / second"
                                          : number(speed, 2) + " Earth days / second";
    glColor3f(.46f, .60f, .68f);
    text(20, 76, rate + (paused ? "  |  Paused" : ""));
    button(630, 19, 95, "Real time", 10, speed < .001);
    button(733, 19, 95, "Low", 11, speed > .001 && speed < .1);
    button(836, 19, 95, "Normal", 12, speed == 1);
    button(939, 19, 95, "High", 13, speed == 30);
    button(1042, 19, 95, paused ? "Play" : "Pause", 14, paused);
    button(1145, 19, 112, "Orbit lines", 16, showOrbits);
    card();
    if (hover >= 0 && hover != selection && !dragging) {
        glColor3f(.88f, .92f, .95f);
        text(mouseX + 15, mouseY - 12, bodyName(hover), 18);
    }
    rect(0, 742, 1280, 58, .012f, .023f, .038f, .88f);
    button(16, 754, 100, "Overview", 0);
    button(124, 754, 90, "Previous", 1);
    button(222, 754, 75, "Next", 2);
    button(305, 754, 90, "Go to", 3);
    button(403, 754, 100, "Close-up", 4);
    button(511, 754, 72, "Zoom +", 5);
    button(591, 754, 72, "Zoom -", 6);
    button(671, 754, 95, "System", 7);
    button(774, 754, 90, "Moons", 8, showMoons);
    button(872, 754, 75, "Info", 9, showInfo);
    glColor3f(.44f, .55f, .64f);
    text(968, 774, "Drag / wheel | F11 window | Esc exit");
    if (traveling && selection >= 0) {
        glColor3f(.66f, .82f, .9f);
        text(20, 110, "Opening " + bodyName(selection) + " scene...");
    }
    if (selection < 0) {
        glColor3f(.45f, .58f, .65f);
        text(20, 110, "Click a planet or moon to visit");
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
struct SkyBody {
    int id;
    Vec direction;
    double distanceAU, radius;
};
std::vector<SkyBody> planetSky(int observer) {
    std::vector<SkyBody> sky;
    Vec origin = observerPosition(observer, days);
    for (int id = 0; id <= 8; id++) {
        if (id == observer)
            continue;
        Vec target = id == 8 ? Vec{0, 0, 0} : planetPosition(id, days, true);
        Vec delta = {target.x - origin.x, target.y - origin.y, target.z - origin.z};
        double distance = length(delta);
        double physicalRadius = id == 8 ? 695700.0 : planets[id].diameter * .5;
        double angularRadius = physicalRadius / (AU * distance);
        double minimumPixels = id == 8 ? 3.0 : 2.0;
        double visibleRadius = std::max(400 * angularRadius,
                                       400 * tan(rad(44) / 2) * minimumPixels / height);
        sky.push_back(SkyBody{id,Vec{delta.x/distance,delta.y/distance,delta.z/distance},distance,visibleRadius});
    }
    return sky;
}
void drawPlanetSky(int observer) {
    std::vector<SkyBody> sky = planetSky(observer);
    std::sort(sky.begin(), sky.end(), [](const SkyBody &a,const SkyBody &b){return a.distanceAU>b.distanceAU;});
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    for (const SkyBody &body : sky) {
        Vec position = add(eye, Vec{400*body.direction.x,400*body.direction.y,400*body.direction.z});
        if (body.id != 8) {
            Vec sunDirection = planetPosition(body.id,days,true);
            GLfloat light[] = {GLfloat(-sunDirection.x),GLfloat(-sunDirection.y),GLfloat(-sunDirection.z),0};
            glLightfv(GL_LIGHT0,GL_POSITION,light);
        }
        glPushMatrix();
        glTranslated(position.x,position.y,position.z);
        if (body.id == 8) drawSun(body.radius);
        else drawPlanet(body.id,body.radius);
        glPopMatrix();
        projectBody(body.id,position,body.radius);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
}
void display() {
    picks.clear();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(44, (double)width / height, .001, 800);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    eye = add(center,
              Vec{distanceNow * cos(rad(pitch)) * sin(rad(yaw)), distanceNow * sin(rad(pitch)),
                  distanceNow * cos(rad(pitch)) * cos(rad(yaw))});
    Vec aim = center;
    if (selection == 5 && showInfo)
        aim = add(center,
                  Vec{distanceNow * .08 * cos(rad(yaw)), 0, -distanceNow * .08 * sin(rad(yaw))});
    gluLookAt(eye.x, eye.y, eye.z, aim.x, aim.y, aim.z, 0, 1, 0);
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glPointSize(1.3f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < stars.size(); i++) {
        float b = .38f + .10f * (i % 6);
        glColor3f(b, b, b);
        glVertex3d(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    if (currentScene == PlanetScene || currentScene == MoonScene)
        drawPlanetSky(selection);
    GLfloat light[] = {0, 0, 0, 1};
    if (currentScene == PlanetScene || currentScene == MoonScene) {
        Vec sunDirection = observerPosition(selection, days);
        light[0] = GLfloat(-sunDirection.x);
        light[1] = GLfloat(-sunDirection.y);
        light[2] = GLfloat(-sunDirection.z);
        light[3] = 0;
    } else if (currentScene != Overview) {
        light[0] = 5;
        light[1] = 3;
        light[2] = 7;
        light[3] = 0;
    }
    glLightfv(GL_LIGHT0, GL_POSITION, light);
    if (currentScene == Overview) {
        if (showOrbits)
            for (int i = 0; i < 8; i++)
                planetOrbit(i);
        drawSun(2.6);
        projectBody(8, Vec{0, 0, 0}, 2.6);
        for (int i = 0; i < 8; i++) {
            Vec p = planetPosition(i, days);
            glPushMatrix();
            glTranslated(p.x, p.y, p.z);
            drawPlanet(i, planets[i].radius);
            glPopMatrix();
            projectBody(i, p, planets[i].radius);
        }
        if (showMoons)
            for (int i = 0; i < 16; i++) {
                Vec p = bodyPosition(i + 9);
                glPushMatrix();
                glTranslated(p.x, p.y, p.z);
                drawMoon(i, bodyRadius(i + 9));
                glPopMatrix();
                projectBody(i + 9, p, bodyRadius(i + 9));
            }
    } else if (currentScene == PlanetScene) {
        int parent = selection;
        drawPlanet(parent, bodyRadius(parent));
        projectBody(parent, Vec{0, 0, 0}, bodyRadius(parent));
        if (showMoons)
            for (int i = 0; i < 16; i++)
                if (moons[i].parent == parent) {
                    Vec p = moonOffset(i, days);
                    if (showOrbits && distanceNow > planets[parent].radius * 10)
                        moonOrbit(i);
                    glPushMatrix();
                    glTranslated(p.x, p.y, p.z);
                    drawMoon(i, bodyRadius(i + 9));
                    glPopMatrix();
                    projectBody(i + 9, p, bodyRadius(i + 9));
                }
    } else if (currentScene == MoonScene) {
        drawMoon(selection - 9, bodyRadius(selection));
        projectBody(selection, Vec{0, 0, 0}, bodyRadius(selection));
    } else {
        drawSun(2.6);
        projectBody(8, Vec{0, 0, 0}, 2.6);
    }
    overlay();
    glutSwapBuffers();
}

int hitBody(float x, float y) {
    int id = -1;
    double depth = 2;
    for (size_t i = 0; i < picks.size(); ++i) {
        Pick &p = picks[i];
        if (p.z < 0 || p.z >= 1)
            continue;
        if (hypot(x - p.x, y - p.y) < p.radius + 5 && p.z < depth) {
            depth = p.z;
            id = p.id;
        }
    }
    return id;
}
bool overButton(float x, float y) {
    if (insideInfo(x, y))
        return true;
    for (size_t i = 0; i < buttons.size(); ++i) {
        Button &b = buttons[i];
        if (x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h)
            return true;
    }
    return false;
}
void mouse(int b, int state, int x, int y) {
    float ux = x * 1280.f / width, uy = y * 800.f / height;
    mouseX = (int)ux;
    mouseY = (int)uy;
    if (state == GLUT_DOWN && (b == 3 || b == 4)) {
        action(b == 3 ? 5 : 6);
        return;
    }
    if (b != GLUT_LEFT_BUTTON)
        return;
    if (state == GLUT_DOWN) {
        for (int i = (int)buttons.size() - 1; i >= 0; --i) {
            Button &bt = buttons[i];
            if (ux >= bt.x && ux <= bt.x + bt.w && uy >= bt.y && uy <= bt.y + bt.h) {
                action(bt.action);
                return;
            }
        }
        if (insideInfo(ux, uy)) {
            dragging = false;
            return;
        }
        dragging = true;
        moved = false;
        dragStartX = (int)ux;
        dragStartY = (int)uy;
    } else if (dragging) {
        dragging = false;
        if (!moved) {
            int id = hitBody(ux, uy);
            if (id >= 0)
                focus(id);
            else
                showInfo = false;
        }
    }
    glutPostRedisplay();
}
void motion(int x, int y) {
    int ux = x * 1280 / width, uy = y * 800 / height;
    if (dragging) {
        if (hypot(ux - dragStartX, uy - dragStartY) > 4)
            moved = true;
        if (moved) {
            yaw += (ux - mouseX) * .35;
            pitch = clamp(pitch + (uy - mouseY) * .35, -85, 85);
        }
    }
    mouseX = ux;
    mouseY = uy;
    hover = overButton(ux, uy) ? -1 : hitBody((float)ux, (float)uy);
    glutPostRedisplay();
}
void keyboard(unsigned char k, int, int) {
    if (k >= '1' && k <= '8') {
        focus(k - '1');
        return;
    }
    switch (k) {
    case 27:
        gluDeleteQuadric(sphere);
        glDeleteTextures(25, textures);
        std::exit(0);
    case '0':
        action(0);
        break;
    case ' ':
        action(14);
        break;
    case '+':
    case '=':
        action(5);
        break;
    case '-':
        action(6);
        break;
    case 'f':
    case 'F':
        action(4);
        break;
    case 'g':
    case 'G':
        action(3);
        break;
    case 'm':
    case 'M':
        action(8);
        break;
    case 'i':
    case 'I':
        action(9);
        break;
    case 'o':
    case 'O':
        action(16);
        break;
    case '\t':
        action(2);
        break;
    case '[':
        speed = std::max(1.0 / 86400, speed / 2);
        break;
    case ']':
        speed = std::min(365.2, speed * 2);
        break;
    }
    glutPostRedisplay();
}
void special(int key, int, int) {
    if (key == GLUT_KEY_F11)
        toggleFullScreen();
    if (key == GLUT_KEY_LEFT)
        yaw -= 5;
    if (key == GLUT_KEY_RIGHT)
        yaw += 5;
    if (key == GLUT_KEY_UP)
        pitch = clamp(pitch + 5, -85, 85);
    if (key == GLUT_KEY_DOWN)
        pitch = clamp(pitch - 5, -85, 85);
    glutPostRedisplay();
}
void reshape(int w, int h) {
    width = std::max(1, w);
    height = std::max(1, h);
    glutPostRedisplay();
}
void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    double dt = clamp((now - lastTime) / 1000.0, 0, .1);
    lastTime = now;
    if (!paused)
        days += dt * speed;
    if (traveling) {
        travelTime += dt;
        if (travelTime > .65)
            traveling = false;
    }
    Vec target = bodyPosition(selection);
    double blend = traveling ? 1 - exp(-5 * dt) : 1;
    center.x += (target.x - center.x) * blend;
    center.y += (target.y - center.y) * blend;
    center.z += (target.z - center.z) * blend;
    distanceNow += (distanceGoal - distanceNow) * (1 - exp(-6 * dt));
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
int main(int, char **argv) {
    int glutArgc = 1;
    glutInit(&glutArgc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Solar System");
    if (fullScreen)
        glutFullScreen();
    glClearColor(.004f, .008f, .016f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat ambient[] = {.25f, .25f, .28f, 1}, diffuse[] = {1, 1, .95f, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    sphere = gluNewQuadric();
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    glGenTextures(25, textures);
    for (int i = 0; i < 25; ++i)
        makeTexture(i);
    srand(7);
    for (int i = 0; i < 2200; ++i) {
        double a = 2 * PI * rand() / RAND_MAX, z = 2.0 * rand() / RAND_MAX - 1, r = sqrt(1 - z * z);
        stars.push_back(Vec{220 * r * cos(a), 220 * z, 220 * r * sin(a)});
    }
    if (selection >= 0) {
        center = bodyPosition(selection);
        distanceNow = distanceGoal = std::max(.7, bodyRadius(selection) * 8.5);
    }
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}
