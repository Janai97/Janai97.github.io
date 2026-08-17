///////////////////////////////////////////////////////////////////////////////
// maincode.cpp
// ============
// gets called when application is launched - initializes GLEW, GLFW
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    // Macro for window title
    const char* const WINDOW_TITLE = "3-2 Milestone";

    // Main GLFW window
    GLFWwindow* g_Window = nullptr;

    // scene manager object for managing the 3D scene prepare and render
    SceneManager* g_SceneManager = nullptr;
    // shader manager object for dynamic interaction with the shader code
    ShaderManager* g_ShaderManager = nullptr;
    // view manager object for managing the 3D view setup and projection to 2D
    ViewManager* g_ViewManager = nullptr;
}

// Function declarations
bool InitializeGLFW();
bool InitializeGLEW();

/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
    // 1. Initialize GLFW library first
    if (InitializeGLFW() == false)
    {
        return(EXIT_FAILURE);
    }

    // 2. Dummy ShaderManager instance to pass down workspace links safely
    // We instantiate it now so ViewManager can use its reference type signature
    g_ShaderManager = new ShaderManager();
    g_ViewManager = new ViewManager(g_ShaderManager);

    // 3. Create the main display window context
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (g_Window == nullptr)
    {
        return(EXIT_FAILURE);
    }

    // 4. CRITICAL FIX: GLEW must be initialized directly AFTER the window context 
    // is created, but BEFORE executing any shaders or model loads.
    if (InitializeGLEW() == false)
    {
        return(EXIT_FAILURE);
    }

    // 5. Load the external shader programs now that OpenGL function pointers are active
    g_ShaderManager->LoadShaders(
        "shaders/vertexShader.glsl",
        "shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    // 6. Create the scene manager object and prepare the geometry/materials/lighting
    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // Loop will keep running until the application is closed 
    while (!glfwWindowShouldClose(g_Window))
    {
        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear the frame and z buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Convert from 3D object space to 2D view
        g_ViewManager->PrepareSceneView();

        // Refresh and draw the 3D scene geometry
        g_SceneManager->RenderScene();

        // Flips the back buffer with the front buffer every frame
        glfwSwapBuffers(g_Window);

        // Query the latest window/input events
        glfwPollEvents();
    }

    // Clear allocated memory safely
    if (nullptr != g_SceneManager)
    {
        delete g_SceneManager;
        g_SceneManager = nullptr;
    }
    if (nullptr != g_ViewManager)
    {
        delete g_ViewManager;
        g_ViewManager = nullptr;
    }
    if (nullptr != g_ShaderManager)
    {
        delete g_ShaderManager;
        g_ShaderManager = nullptr;
    }

    // Terminates the program successfully
    exit(EXIT_SUCCESS);
}

/***********************************************************
 *  InitializeGLFW()
 *
 *  This function is used to initialize the GLFW library.
 ***********************************************************/
bool InitializeGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

/***********************************************************
 *  InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
    // Enable glewExperimental to ensure core profile pointer access works correctly
    glewExperimental = GL_TRUE;

    GLenum GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << "ERROR: GLEW Initialization Failed: " << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }

    // Displays a successful OpenGL initialization message
    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

    return true;
}