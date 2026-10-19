# Solar System 3D

An interactive 3D solar system project developed in C++ using OpenGL and GLUT/freeglut for Code::Blocks. The project lets users explore the Sun, eight planets and 16 selected moons through animated scenes and interactive information panels.

## Features

- Solar system overview with the Sun at the center and orbiting planets.
- 26 scenes: the overview, eight planets, 16 moons and the Sun.
- Planet views with nearby moons, the distant Sun and other planets.
- Moon views with their parent planet, the Sun and other planets.
- Clickable objects for travel between scenes.
- Camera rotation, zoom, close-up views and saved camera settings for each scene.
- Information panels displayed after selecting a planet or moon.
- Planet and moon measurements, rotation rates, orbital periods and model distances from Earth.
- Earth-based simulation clock with adjustable speed and pause controls.
- Synchronous moon rotation, including Triton's retrograde motion.
- Full-screen startup with a window-mode toggle.
- Surfaces, clouds, craters, stars and rings generated through C++ code without external image assets.

## Included Objects

The Sun and all eight planets are included: Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus and Neptune.

| Parent planet | Included moons |
|---|---|
| Earth | Moon |
| Mars | Phobos, Deimos |
| Jupiter | Io, Europa, Ganymede, Callisto |
| Saturn | Enceladus, Rhea, Titan |
| Uranus | Miranda, Ariel, Umbriel, Titania, Oberon |
| Neptune | Triton |

## Requirements

- Code::Blocks with a MinGW GNU GCC compiler supporting C++11.
- A freeglut development package matching the compiler's 32-bit or 64-bit architecture.
- OpenGL and GLU libraries.

## Build and Run

1. Extract the project folder and open `SolarSystem3D.cbp` in Code::Blocks.
2. Configure the `freeglut` global variable with the package's actual directories. For a package installed at `C:\freeglut`, use:
   - **Base:** `C:\freeglut`
   - **Include:** `C:\freeglut\include`
   - **Lib:** `C:\freeglut\lib`, or the matching architecture-specific library folder.
3. Ensure the include directory contains `GL\glut.h` and the library directory contains the MinGW freeglut import library. The project links `freeglut`, `glu32` and `opengl32`.
4. Place the matching `freeglut.dll` beside the generated executable in `bin\Debug` or `bin\Release`, or add its directory to `PATH`.
5. Select **Build → Rebuild**, then press **F9** to run.

### Linux

With a C++ compiler and the freeglut development package installed, run from the project folder:

```sh
g++ -std=c++11 -O2 main.cpp Graphics.cpp Surface.cpp Moons.cpp Sun.cpp planets/*.cpp -o SolarSystem3D -lglut -lGLU -lGL
./SolarSystem3D
```

## Controls

| Control | Action |
|---|---|
| Click an object | Open its scene and information |
| Hover over an object | Show its name |
| Drag / arrow keys | Rotate the camera |
| Mouse wheel / Zoom buttons / + / - | Zoom in or out |
| Go to | Frame the selected object |
| Close-up / F | Move closer and hide the information panel |
| Info / I | Show or hide information |
| Facts / Moons tabs | View planet information or select a moon |
| Previous / Next | Switch between planets or between moons |
| System | Return to the parent planet and its moon layout |
| Moons / M | Show or hide moons |
| Orbit lines / O | Show or hide orbit paths |
| Overview / 0 | Return to the solar system overview |
| 1–8 | Open Mercury through Neptune |
| G | Go to the selected object |
| Space | Pause or resume animation |
| [ / ] | Halve or double the simulation speed |
| F11 | Switch between full screen and window mode |
| Esc | Exit |

Drag to look around a planet or moon. The Sun and other objects may be outside the current camera view. Click a distant object to visit it.

## Simulation Time

All orbital and rotation speeds follow one shared Earth-based clock. One Earth day equals 24 hours or 86,400 seconds. The simulation starts at day zero with the **Low** speed selected.

| Speed setting | Simulated time per real second |
|---|---|
| Real time | One Earth second |
| Low | One Earth hour |
| Normal | One Earth day |
| High | 30 Earth days |

Each moon completes one axial rotation per modeled orbit, keeping the same hemisphere toward its parent. The parent planet rotates at its own reference rate. Changing the simulation speed scales these motions together.

## Project Structure

| File or folder | Purpose |
|---|---|
| `main.cpp` | Scenes, camera, interaction, information panels, orbital positions and simulation clock |
| `SolarSystem.h` | Shared types and function declarations |
| `Graphics.cpp` | Surface generation, shared sphere rendering and ring geometry |
| `Surface.cpp` | Color, noise and crater calculations |
| `Sun.cpp` | Sun appearance and rotation |
| `Moons.cpp` | Moon properties, surface functions, rotation and drawing |
| `planets/` | Individual planet properties, surface functions and rendering code |
| `SolarSystem3D.cbp` | Code::Blocks project configuration |

## Visualization Model

Planet surfaces are procedural illustrations calculated in memory from C++ functions. Earth's coastlines use geographic vector coordinates compiled into `planets/Earth.cpp`. No external image files are required.

Planet motion uses simplified elliptical orbits, while moons use circular orbits with reference periods. Sizes and spacing are adjusted for visibility. Distant objects use relative orbital directions and reference angular sizes, with small minimum display sizes to keep them visible.

The project is an educational visualization. Surface patterns, initial orbital phases and displayed distances represent the simulation rather than current observations. It does not model gravitational interactions, libration, precession or eclipse shadows.

## References and Credits

- [NASA Planetary Fact Sheets](https://nssdc.gsfc.nasa.gov/planetary/factsheet/) — planetary and satellite reference measurements.
- [NASA Sun Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/sunfact.html) — Sun reference measurements.
- [JPL Satellite Physical Parameters](https://ssd.jpl.nasa.gov/sats/phys_par/) — moon physical properties.
- [JPL Satellite Orbital Elements](https://ssd.jpl.nasa.gov/sats/elem/) — moon orbital reference data.
- [NASA Tidal Locking](https://science.nasa.gov/moon/tidal-locking/) — synchronous rotation.
- [NASA Triton](https://science.nasa.gov/neptune/moons/triton/) — retrograde synchronous rotation.
- [Natural Earth](https://www.naturalearthdata.com/about/terms-of-use/) — public-domain coastline vectors.
