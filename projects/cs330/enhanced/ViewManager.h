///////////////////////////////////////////////////////////////////////////////
// ViewManager.h
// ============
// Manage the viewing of 3D objects within the viewport - camera, projection
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"
#include "GLFW/glfw3.h"

class ViewManager
{
public:
	// constructor
	ViewManager(ShaderManager* pShaderManager);
	// destructor
	~ViewManager();

	// Mouse position callback for mouse interaction with the 3D scene
	static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);

	// Scroll-wheel callback for adjusting camera movement/zoom variables
	static void Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset);

	// Key callback for handling discrete events like perspective toggles
	static void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	// Pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// Active OpenGL display window
	GLFWwindow* m_pWindow;

	// Process continuous keyboard events for active fluid camera movement
	void ProcessKeyboardEvents();

public:
	// Create the initial OpenGL display window
	GLFWwindow* CreateDisplayWindow(const char* windowTitle);

	// Prepare the conversion from 3D object display to 2D scene display
	void PrepareSceneView();
};