# Sphere Scene Rasterizer – Assignment 5

This project implements a **CPU-based software rasterizer** that renders a shaded 3D sphere using manually defined transformations and depth buffering.  
It displays the result inside a window using a minimal OpenGL viewer (via `glDrawPixels`), but **all rasterization is done in software** — **no OpenGL rendering pipeline** is used. 

## ✅ Features

- Procedurally generated 3D sphere
- Software rasterization (no OpenGL)
- Perspective projection
- Back-face culling
- Z-buffer based occlusion
- Outputs 512x512 PNG image

* Generates a 32 × 16 triangulated **unit sphere** (sphere_scene.c from the course).
* **Model → View → Perspective → Viewport** transform chain  
  * centre = (0 0 −7), radius = 2  
  * frustum l −0.1  r 0.1  b −0.1  t 0.1  n −0.1  f −1000
* **Depth buffer** for hidden-surface removal (float Z per pixel).
* Build targets  
  * **Unshaded** – flat white sphere (matches lecture “Unshaded” slide)
* Final image is **displayed in a window**


## 🛠️ Getting Started

### 1. Clone the Repository

bash
git clone https://github.com/tuguldur77/CG_HW5_202213827.git
cd sphere_scene_rasterizer


### 2. Open the solution

Locate and open .sln (double-click it), or
In Visual Studio: File → Open → Folder... and select the project directory

### 3. Build the project

Press Ctrl + Shift + B to build the project

### 4. Run the program

Press Ctrl + F5 (Start Without Debugging)
On success, the program will show output image using viewer in HW1


## 📸 Sample Output
This rendered sphere in white on a black background same as providen example image:
Scene: 450 vertices, 868 triangles
![output_HW5](https://github.com/user-attachments/assets/d4705f9c-ed53-418b-93f4-daccdd909ebb)

## ✍️ Author
Computer Graphics – Spring 2025 – Assignment 5, Konkuk University
Maintainer: Tuguldur SID-202213827
