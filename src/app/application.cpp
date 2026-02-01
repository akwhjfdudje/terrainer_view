#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "application.h"
#include "render/shader.h"
#include "render/camera.h"
#include "terrain/terrainMesh.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>
#include <chrono>
#include <cassert>

static void mouseCallback(GLFWwindow* window, double x, double y) {
    auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app->m_firstMouse) {
        app->m_lastX = x;
        app->m_lastY = y;
        app->m_firstMouse = false;
    }

    // store deltas for later use in run()
    app->m_mouseX = x;
    app->m_mouseY = y;
}

static void glfwErrorCallback(int code, const char* desc) {
    std::cerr << "[GLFW] (" << code << ") " << desc << std::endl;
}

static void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

void initTriangle(Application* app) {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &app->m_vao);
    glGenBuffers(1, &app->m_vbo);

    glBindVertexArray(app->m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, app->m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

Application::Application(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title) {
    initWindow();
    initOpenGL();
    setupCallbacks();
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void Application::initWindow() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync
}

void Application::initOpenGL() {
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    std::cout << "OpenGL Vendor   : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version  : " << glGetString(GL_VERSION) << std::endl;

    // Global GL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity,
           GLsizei, const GLchar* message, const void*) {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
            std::cerr << "[GL DEBUG] " << message << std::endl;
        },
        nullptr
    );
#endif
    m_shader = std::make_unique<Shader>(
        "shaders/terrain.vert",
        "shaders/terrain.frag"
    );

    float farPlane = m_terrain->m_scale * 1.5f; // leave some margin
    m_camera = std::make_unique<Camera>(
        60.0f,
        float(m_width) / float(m_height),
        0.1f,
        500.0f
    );
    m_terrain = std::make_unique<TerrainMesh>(m_gridWidth, m_gridDepth);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = !mouseCaptured; // draw cursor only when mouse not captured

    // Setup style
    ImGui::StyleColorsDark();

    // Setup backend
    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::setupCallbacks() {
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            // get pointer to your app (stored as GLFW user pointer)
            auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
            app->toggleMouseCapture();
        }
    });

}

void Application::toggleMouseCapture() {
    mouseCaptured = !mouseCaptured;
    if (mouseCaptured) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide + capture
    } else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   // show cursor
    }
}

void Application::run() {
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();

    while (m_running && !glfwWindowShouldClose(m_window)) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Terrain Controls");
        ImGui::SliderFloat("Height Scale", &m_terrain->m_scale, 1.0f, 1000.0f);
        ImGui::SliderFloat("Noise Mix", &m_terrain->m_mix, 0.0f, 1.0f);
        if (ImGui::Button("Regenerate")) m_terrain->regenerate();
        ImGui::End();

        // Process camera input only if mouse is captured AND ImGui is not using it
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = !mouseCaptured; // draw ImGui cursor when camera is not capturing

        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        if (mouseCaptured && !io.WantCaptureMouse) {
            float dx = float(m_mouseX - m_lastX);
            float dy = float(m_lastY - m_mouseY);
            m_lastX = m_mouseX;
            m_lastY = m_mouseY;
            m_camera->processMouse(dx, dy);
        }

        // Update & render
        processInput();
        update(dt);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader->bind();
        m_shader->setMat4("uView", m_camera->view());
        m_shader->setMat4("uProj", m_camera->projection());
        m_shader->setVec3("uLightDir", glm::normalize(glm::vec3(0.5f,1.0f,0.3f)));
        m_terrain->draw();
        m_shader->unbind();

        // Render ImGui on top
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

void Application::processInput() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    // Base camera speed
    float baseSpeed = 10.0f;

    // Scale with terrain size
    float speed = baseSpeed * (m_terrain->m_scale / 100.0f); // adjust 100.0f as needed

    // Use dt for smooth movement
    speed *= 1.0f / 60.0f; // or your frame delta

    float dx = 0, dy = 0, dz = 0;

    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) dx -= 1;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) dx += 1;
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) dy -= 1;
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) dy += 1;
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) dz += 1;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) dz -= 1;

    m_camera->processKeyboard(dx, dy, dz, speed);
}

void Application::update(float /*dt*/) {
    // Terrain updates later (regen, erosion, etc.)
}

void Application::render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->bind();
    m_shader->setMat4("uView", m_camera->view());
    m_shader->setMat4("uProj", m_camera->projection());

    // optional: change light direction or color
    m_shader->setVec3("uLightDir", glm::normalize(glm::vec3(0.5f,1.0f,0.3f)));

    m_terrain->draw();
    m_shader->unbind();
}

