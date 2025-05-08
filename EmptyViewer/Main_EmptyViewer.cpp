#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>

#ifndef M_PI  
#define M_PI 3.14159265358979323846  
#endif  

// ------------------ Globals ----------------------
int Width = 512;
int Height = 512;
std::vector<float> OutputImage; // RGB float values [0,1]

// -------- Rasterizer Globals ---------------------
constexpr int IMG_W = 512;
constexpr int IMG_H = 512;
static unsigned char framebuffer[IMG_H][IMG_W][3];
static float zbuffer[IMG_H][IMG_W];

int gNumVertices = 0;
int gNumTriangles = 0;
int* gIndexBuffer = nullptr;
float* gVertexBuffer = nullptr;

// ------------ Scene and Math ---------------------
struct Vec3 {
    float x, y, z;
    Vec3() = default;
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
};
inline Vec3 normalize(Vec3 v) {
    float L = std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return (L > 0) ? v * (1.0f / L) : v;
}

struct VScreen {
    float x, y, zcam, depth;
    Vec3 n;
};

// ---------- Core Rasterization -------------------
void clear_buffers() {
    std::memset(framebuffer, 0, sizeof(framebuffer));
    for (int y = 0; y < IMG_H; ++y)
        for (int x = 0; x < IMG_W; ++x)
            zbuffer[y][x] = 1e9f;
}

void put_pixel(int x, int y, float z, unsigned char r, unsigned char g, unsigned char b) {
    if ((unsigned)x >= IMG_W || (unsigned)y >= IMG_H) return;
    if (z < zbuffer[y][x]) {
        zbuffer[y][x] = z;
        framebuffer[y][x][0] = r;
        framebuffer[y][x][1] = g;
        framebuffer[y][x][2] = b;
    }
}

void raster_triangle(const VScreen& a, const VScreen& b, const VScreen& c) {
    int x0 = int(a.x), y0 = int(a.y);
    int x1 = int(b.x), y1 = int(b.y);
    int x2 = int(c.x), y2 = int(c.y);
    int minX = std::max(0, std::min({ x0, x1, x2 }));
    int maxX = std::min(IMG_W - 1, std::max({ x0, x1, x2 }));
    int minY = std::max(0, std::min({ y0, y1, y2 }));
    int maxY = std::min(IMG_H - 1, std::max({ y0, y1, y2 }));

    float area = float((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0));
    if (area == 0) return;
    float invA = 1.0f / area;

    for (int y = minY; y <= maxY; ++y)
        for (int x = minX; x <= maxX; ++x) {
            float w0 = float((x1 - x0) * (y - y0) - (x - x0) * (y1 - y0));
            float w1 = float((x2 - x1) * (y - y1) - (x - x1) * (y2 - y1));
            float w2 = float((x0 - x2) * (y - y2) - (x - x2) * (y0 - y2));
            if (!((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0))) continue;

            float alpha = w1 * invA;
            float beta = w2 * invA;
            float gamma = 1.0f - alpha - beta;
            float depth = alpha * a.depth + beta * b.depth + gamma * c.depth;
            if (depth >= zbuffer[y][x]) continue;

            put_pixel(x, y, depth, 255, 255, 255);
        }
}

void create_scene() {
    int width = 32;
    int height = 16;
    float theta, phi;
    int t;

    gNumVertices = (height - 2) * width + 2;
    gNumTriangles = (height - 2) * (width - 1) * 2;

    gVertexBuffer = new float[3 * gNumVertices];
    gIndexBuffer = new int[3 * gNumTriangles];

    t = 0;
    for (int j = 1; j < height - 1; ++j) {
        for (int i = 0; i < width; ++i) {
            theta = (float)j / (height - 1) * M_PI;
            phi = (float)i / (width - 1) * M_PI * 2;
            theta = (float)j / (height - 1) * M_PI;
            float x = sinf(theta) * cosf(phi);
            float y = cosf(theta);
            float z = -sinf(theta) * sinf(phi);
            gVertexBuffer[3 * t + 0] = x;
            gVertexBuffer[3 * t + 1] = y;
            gVertexBuffer[3 * t + 2] = z;
            t++;
        }
    }
    gVertexBuffer[3 * t + 0] = 0.0f;
    gVertexBuffer[3 * t + 1] = 1.0f;
    gVertexBuffer[3 * t + 2] = 0.0f;
    t++;
    gVertexBuffer[3 * t + 0] = 0.0f;
    gVertexBuffer[3 * t + 1] = -1.0f;
    gVertexBuffer[3 * t + 2] = 0.0f;
    t++;

    t = 0;
    for (int j = 0; j < height - 3; ++j) {
        for (int i = 0; i < width - 1; ++i) {
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
            gIndexBuffer[t++] = j * width + (i + 1);
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
        }
    }
    for (int i = 0; i < width - 1; ++i) {
        gIndexBuffer[t++] = (height - 2) * width;
        gIndexBuffer[t++] = i;
        gIndexBuffer[t++] = i + 1;
        gIndexBuffer[t++] = (height - 2) * width + 1;
        gIndexBuffer[t++] = (height - 3) * width + (i + 1);
        gIndexBuffer[t++] = (height - 3) * width + i;
    }
}

// ---------------- Render & Window -----------------
void render() {
    clear_buffers();
    create_scene();
    std::vector<VScreen> vs(gNumVertices);

    float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, n = -0.1f, f = -1000.0f;

    for (int i = 0; i < gNumVertices; ++i) {
        Vec3 p = {
            gVertexBuffer[3 * i + 0],
            gVertexBuffer[3 * i + 1],
            gVertexBuffer[3 * i + 2]
        };
        Vec3 cam = { p.x * 2.0f, p.y * 2.0f, p.z * 2.0f - 7.0f };
        float px = (2 * n / (r - l)) * cam.x + (r + l) / (r - l) * cam.z;
        float py = (2 * n / (t - b)) * cam.y + (t + b) / (t - b) * cam.z;
        float pz = -(f + n) / (f - n) * cam.z - 2 * f * n / (f - n);
        float pw = -cam.z;

        float ndcX = px / pw;
        float ndcY = py / pw;
        vs[i].x = (ndcX + 1.f) * 0.5f * IMG_W;
        vs[i].y = (1.f - ndcY) * 0.5f * IMG_H;
        vs[i].zcam = cam.z;
        vs[i].depth = -cam.z;
        vs[i].n = normalize(p);
    }

    for (int t = 0; t < gNumTriangles; ++t) {
        const VScreen& v0 = vs[gIndexBuffer[3 * t]];
        const VScreen& v1 = vs[gIndexBuffer[3 * t + 1]];
        const VScreen& v2 = vs[gIndexBuffer[3 * t + 2]];
        float face = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
        if (face <= 0) continue;
        raster_triangle(v0, v1, v2);
    }

    OutputImage.resize(IMG_W * IMG_H * 3);
    for (int j = 0; j < IMG_H; ++j)
        for (int i = 0; i < IMG_W; ++i) {
            int idx = (j * IMG_W + i) * 3;
            OutputImage[idx + 0] = framebuffer[j][i][0] / 255.0f;
            OutputImage[idx + 1] = framebuffer[j][i][1] / 255.0f;
            OutputImage[idx + 2] = framebuffer[j][i][2] / 255.0f;
        }
}

void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Width, 0.0, Height, 1.0, -1.0);
    OutputImage.reserve(Width * Height * 3);
    render();
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(Width, Height, "Software Sphere Rasterizer", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    resize_callback(NULL, Width, Height);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i((Width - IMG_W) / 2, (Height - IMG_H) / 2);
        glDrawPixels(IMG_W, IMG_H, GL_RGB, GL_FLOAT, OutputImage.data());
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
