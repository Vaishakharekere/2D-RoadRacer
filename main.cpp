
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>
#include <GLUT/glut.h>

#define GL_SILENCE_DEPRECATION

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --- Texture Struct ---
struct Texture {
    GLuint id;
    float aspect;
};

// --- Texture Loaders ---
Texture loadTexturePNG(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // Force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return {0, 1.0f};
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(data);
    return {texture, static_cast<float>(width) / height};
}

GLuint loadBMP(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        std::cerr << "Failed to load texture file " << filename << "\n";
        return 0;
    }

    unsigned char header[54];
    fread(header, 1, 54, file);
    int dataPos = *(int*)&(header[0x0A]);
    int width = *(int*)&(header[0x12]);
    int height = *(int*)&(header[0x16]);
    int imageSize = *(int*)&(header[0x22]);

    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;

    unsigned char* data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return textureID;
}

// --- Game State ---
int windowWidth = 500, windowHeight = 700;
float carX = 0.0f;
float speed = 0.01f;
int score = 0;
bool isGameOver = false;
GLuint roadTexture;
Texture playerTexture, enemyTexture;

const float lanes[] = {-0.6f, 0.0f, 0.6f};
const int maxEnemies = 3;

struct EnemyCar {
    float x, y;
    bool passed = false;
};

std::vector<EnemyCar> enemies;

// --- Rendering ---
void drawTexturedQuad(float x, float y, float height, Texture tex) {
    float width = height * tex.aspect;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(x - width / 2, y);
        glTexCoord2f(1, 0); glVertex2f(x + width / 2, y);
        glTexCoord2f(1, 1); glVertex2f(x + width / 2, y + height);
        glTexCoord2f(0, 1); glVertex2f(x - width / 2, y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawTexturedBackground() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roadTexture);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1, 0); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1, 1); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0, 1); glVertex2f(-1.0f, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawText(const std::string& text, float x, float y, void* font = GLUT_BITMAP_HELVETICA_18) {
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(font, c);
}

// --- Game Logic ---
void resetEnemies() {
    enemies.clear();
    std::vector<int> usedLanes = {0, 1, 2};
    std::shuffle(usedLanes.begin(), usedLanes.end(), std::default_random_engine(std::rand()));

    int numToSpawn = rand() % 2 + 1; // Spawn 1 or 2 enemies
    for (int i = 0; i < numToSpawn; i++) {
       enemies.emplace_back(lanes[usedLanes[i]], 1.2f + i * 0.3f, false);


    }
}

void resetGame() {
    carX = 0.0f;
    speed = 0.01f;
    score = 0;
    isGameOver = false;
    resetEnemies();
}

// --- GLUT Callbacks ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawTexturedBackground();

    if (isGameOver) {
        drawText("GAME OVER", -0.3f, 0.2f);
        drawText("Score: " + std::to_string(score), -0.3f, 0.1f);
        drawText("Press ENTER to retry", -0.45f, -0.0f);
    } else {
        drawTexturedQuad(carX, -0.9f, 0.3f, playerTexture);

        for (auto& enemy : enemies)
            drawTexturedQuad(enemy.x, enemy.y, 0.3f, enemyTexture);

        drawText("Score: " + std::to_string(score), -0.95f, 0.9f);
    }

    glutSwapBuffers();
}

void update(int value) {
    if (!isGameOver) {
        for (auto& enemy : enemies) {
            enemy.y -= speed;

            if (!enemy.passed && enemy.y < -1.2f) {
                enemy.passed = true;
                score++;
                if (score % 5 == 0 && speed < 0.05f)
                    speed += 0.002f;
            }

            // Collision
            if (abs(enemy.y + 0.15f + 0.9f) < 0.15f && abs(enemy.x - carX) < 0.2f) {
                isGameOver = true;
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [](EnemyCar& e) { return e.y < -1.2f; }), enemies.end());

        if (enemies.empty()) {
            resetEnemies();
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, update, 0);
}

void handleKey(int key, int x, int y) {
    if (isGameOver) return;

    if (key == GLUT_KEY_LEFT && carX > -0.6f)
        carX -= 0.6f;
    else if (key == GLUT_KEY_RIGHT && carX < 0.6f)
        carX += 0.6f;

    glutPostRedisplay();
}

void handleNormalKey(unsigned char key, int x, int y) {
    if (isGameOver && key == 13) {
        resetGame();
        glutPostRedisplay();
    }
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    srand((unsigned)time(0));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    roadTexture = loadBMP("road.bmp");
    playerTexture = loadTexturePNG("car.png");
    enemyTexture = loadTexturePNG("police.png");

    if (!roadTexture || !playerTexture.id || !enemyTexture.id) {
        std::cerr << "One or more textures failed to load. Exiting.\n";
        exit(1);
    }

    resetGame();
}

// --- Main ---
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Top View Car Game");
    init();
    glutDisplayFunc(display);
    glutSpecialFunc(handleKey);
    glutKeyboardFunc(handleNormalKey);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
