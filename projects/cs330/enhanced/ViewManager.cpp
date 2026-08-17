///////////////////////////////////////////////////////////////////////////////
// ViewManager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    
#include <iostream>

// Namespace for declaring global/file-local variables
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// Camera object used for viewing and interacting with the 3D scene
	Camera* g_pCamera = nullptr;

	// These variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// Time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// The following variable is false when orthographic projection is off and true when it is on
	bool bOrthographicProjection = false;
}

// =================================================================
// CONSTRUCTOR & DESTRUCTOR
// =================================================================

ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = nullptr;
	g_pCamera = new Camera();

	// Default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80.0f;
	g_pCamera->MovementSpeed = 20.0f;
}

ViewManager::~ViewManager()
{
	m_pShaderManager = nullptr;
	m_pWindow = nullptr;
	if (nullptr != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = nullptr;
	}
}

// =================================================================
// WINDOW CREATION & INPUT REGISTER
// =================================================================

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// Tell GLFW to capture and isolate mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Route GLFW callbacks directly to static class listeners
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glfwSetScrollCallback(window, &ViewManager::Scroll_Callback);
	glfwSetKeyCallback(window, &ViewManager::Key_Callback);

	// Enable blending for supporting transparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return window;
}

// =================================================================
// STATIC EVENT ENGINE CALLBACKS
// =================================================================

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// Calculate offset relative to previous coordinate frame
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // Reversed since y-coordinates go from bottom to top

	gLastX = xMousePos;
	gLastY = yMousePos;

	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

void ViewManager::Scroll_Callback(GLFWwindow* window, double dx, double yOffset)
{
	// Guard speed values to stay interactive above absolute baseline threshold
	g_pCamera->MovementSpeed = std::max(0.1f, g_pCamera->MovementSpeed + (float)yOffset);
}

void ViewManager::Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_O)
		bOrthographicProjection = true;
	else if (key == GLFW_KEY_P)
		bOrthographicProjection = false;
}

// =================================================================
// FRAME TRANSFORMATION LAYER
// =================================================================

void ViewManager::ProcessKeyboardEvents()
{
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// Forward / Backward travel ticks
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// Lateral Panning ticks
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	// Vertical elevation adjustments
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}
}

void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// Calculate consistent per-frame movement normalization metrics
	float currentFrame = (float)glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	ProcessKeyboardEvents();

	// Extract active view translation matrix directly out of local camera calculations
	view = g_pCamera->GetViewMatrix();

	// Establish projection volume coordinates based on selected view mode flags
	if (bOrthographicProjection)
	{
		float size = 10.0f;
		float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
		projection = glm::ortho(-aspect * size, aspect * size, -size, size, 0.1f, 1000.0f);
	}
	else
	{
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
			0.1f, 1000.0f
		);
	}

	// Safe dispatch uniforms via matched ShaderManager syntax naming conventions
	if (nullptr != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}