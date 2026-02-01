#pragma once

#include <string>
#include <memory>

class Camera;
class Shader;
class TerrainMesh;

struct GLFWwindow;

class Application {
public:
    // TODO: patch exposed variables
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    bool m_firstMouse = true;
    double m_lastX = 0.0;
    double m_lastY = 0.0;
    double m_mouseX = 0.0;
    double m_mouseY = 0.0;

    int m_gridWidth = 1024;   // NxN grid
    int m_gridDepth = 1024;

    std::unique_ptr<TerrainMesh> m_terrain;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Camera> m_camera;

    Application(int width = 1280, int height = 800, const std::string& title = "Terrain Viewer");
    ~Application();

    void run();

private:

    void initWindow();
    void initOpenGL();
    void setupCallbacks();

    void processInput();
    void update(float dt);
    void render();
    void toggleMouseCapture();

private:
    int m_width;
    int m_height;
    std::string m_title;

    GLFWwindow* m_window = nullptr;
    bool m_running = true;
    bool mouseCaptured = true; // start in camera mode
};

