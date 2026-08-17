///////////////////////////////////////////////////////////////////////////////
// ShapeMeshes.h
// ============
// Procedural geometry definition layer for generating and rendering 3D primitives
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class ShapeMeshes
{
public:
    ShapeMeshes();
    ~ShapeMeshes();

    // Procedural geometry loaders
    void LoadBoxMesh();
    void LoadPlaneMesh();
    void LoadCylinderMesh();
    void LoadTaperedCylinderMesh();
    void LoadConeMesh();
    void LoadPrismMesh();
    void LoadPyramid4Mesh();
    void LoadSphereMesh();
    void LoadTorusMesh();

    // Render pipeline triggers
    void DrawBoxMesh();
    void DrawPlaneMesh();
    void DrawCylinderMesh();
    void DrawTaperedCylinderMesh();
    void DrawConeMesh();
    void DrawPrismMesh();
    void DrawPyramid4Mesh();
    void DrawSphereMesh();
    void DrawTorusMesh();

private:
    // Internal helper to bind non-indexed buffers
    void BuildMesh(GLuint& vao, GLuint& vbo, const GLfloat* vertices, int vertexCount);

    // VAO and VBO handles initialized to zero for memory-safe destruction
    GLuint boxVAO = 0, boxVBO = 0;
    GLuint planeVAO = 0, planeVBO = 0;
    GLuint cylinderVAO = 0, cylinderVBO = 0;
    GLuint taperedCylinderVAO = 0, taperedCylinderVBO = 0;
    GLuint coneVAO = 0, coneVBO = 0;
    GLuint prismVAO = 0, prismVBO = 0;
    GLuint pyramid4VAO = 0, pyramid4VBO = 0;
    GLuint sphereVAO = 0, sphereVBO = 0;
    GLuint torusVAO = 0, torusVBO = 0;

    // Element Buffer Objects for indexed shapes
    GLuint cylinderEBO = 0;
    GLuint taperedCylinderEBO = 0;
    GLuint coneEBO = 0;
    GLuint prismEBO = 0;
    GLuint sphereEBO = 0;
    GLuint torusEBO = 0;

    // Element counts for indexed draw calls
    GLsizei cylinderIndicesCount = 0;
    GLsizei taperedCylinderIndicesCount = 0;
    GLsizei coneIndicesCount = 0;
    GLsizei prismIndicesCount = 0;
    GLsizei sphereIndicesCount = 0;
    GLsizei torusIndicesCount = 0;

    // Vertex tracking counts for non-indexed draw calls
    GLsizei boxVerticesCount = 0;
    GLsizei planeVerticesCount = 0;
    GLsizei pyramid4VerticesCount = 0;
};