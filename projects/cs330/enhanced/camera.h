///////////////////////////////////////////////////////////////////////////////
// camera.h
// ============
// Header-only FPS/Free-look Euler camera controller matrix utility.
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// Enumeration abstraction for navigation vectors
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera
{
public:
    // Spatial Coordinate Vectors
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler Orientations and View Modifiers
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with default state configurations
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Calculates the Look-At transformation view matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes standard directional input while keeping ground-movement locked horizontally
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;

        // Isolate the horizontal plane vector for clean directional walking tracking
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        if (direction == FORWARD)  Position += horizontalFront * velocity;
        if (direction == BACKWARD) Position -= horizontalFront * velocity;
        if (direction == LEFT)     Position -= horizontalRight * velocity;
        if (direction == RIGHT)    Position += horizontalRight * velocity;

        // Elevation steps shift perfectly along the true world up coordinate axis
        if (direction == UP)       Position += WorldUp * velocity;
        if (direction == DOWN)     Position -= WorldUp * velocity;
    }

    // Processes rotational angular offsets received via mouse events
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = GL_TRUE) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Prevent camera flipping upside down by guarding gimbal boundaries
        if (constrainPitch) {
            Pitch = std::max(-89.0f, std::min(89.0f, Pitch));
        }

        updateCameraVectors();
    }

private:
    // Internal trigonometric calculations updating facing direction vectors
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};