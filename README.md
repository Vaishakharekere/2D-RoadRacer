# 2D-RoadRacer
# 🚗 Top View 2D Car Racing Game

A simple top-view car racing game built using **C++**, **OpenGL**, and **GLUT**, featuring:
- Dynamic enemy car generation
- Increasing difficulty with score
- Collision detection
- Score tracking
- Texture-based rendering for player, enemies, and road

---

## 🎮 Gameplay Overview

You control a car racing down a road, avoiding police cars coming toward you. As your score increases, the speed of the game ramps up.

- Use **← (Left Arrow)** and **→ (Right Arrow)** to move between lanes.
- Avoid collisions with enemy cars.
- When the game ends, press **Enter** to restart.

---


## 🧱 Features

- 🔄 Procedural enemy spawning
- 🎮 Simple and responsive controls
- 💥 Collision detection & Game Over state
- ⏫ Speed increases with score
- 🖼️ PNG and BMP texture loading using `stb_image`

---

## 🧰 Dependencies

- [OpenGL Utility Toolkit (GLUT)](https://freeglut.sourceforge.net/)
- [`stb_image.h`](https://github.com/nothings/stb) for PNG texture loading

---

## 🛠️ How to Build

### 🖥️ macOS (with GLUT via Xcode or Brew)

```bash
brew install freeglut
g++ main.cpp -o car_game -framework OpenGL -framework GLUT
./car_game


