///////////////////////////////////////////////////////////////////////////////
// ShapeMeshes.cpp
// ============
// Procedural geometry engine for generating and rendering 3D primitives
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#include "ShapeMeshes.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>

// =================================================================
// CONSTRUCTOR & DESTRUCTOR
// =================================================================
ShapeMeshes::ShapeMeshes() {
    // Intentionally blank; handles member initialization safely via header defaults
}

ShapeMeshes::~ShapeMeshes() {
    // Clean up Non-Indexed Meshes to prevent VRAM memory leaks
    if (boxVAO != 0) glDeleteVertexArrays(1, &boxVAO);
    if (boxVBO != 0) glDeleteBuffers(1, &boxVBO);

    if (planeVAO != 0) glDeleteVertexArrays(1, &planeVAO);
    if (planeVBO != 0) glDeleteBuffers(1, &planeVBO);

    if (pyramid4VAO != 0) glDeleteVertexArrays(1, &pyramid4VAO);
    if (pyramid4VBO != 0) glDeleteBuffers(1, &pyramid4VBO);

    // Clean up Indexed Meshes
    if (cylinderVAO != 0) glDeleteVertexArrays(1, &cylinderVAO);
    if (cylinderVBO != 0) glDeleteBuffers(1, &cylinderVBO);
    if (cylinderEBO != 0) glDeleteBuffers(1, &cylinderEBO);

    if (taperedCylinderVAO != 0) glDeleteVertexArrays(1, &taperedCylinderVAO);
    if (taperedCylinderVBO != 0) glDeleteBuffers(1, &taperedCylinderVBO);
    if (taperedCylinderEBO != 0) glDeleteBuffers(1, &taperedCylinderEBO);

    if (coneVAO != 0) glDeleteVertexArrays(1, &coneVAO);
    if (coneVBO != 0) glDeleteBuffers(1, &coneVBO);
    if (coneEBO != 0) glDeleteBuffers(1, &coneEBO);

    if (prismVAO != 0) glDeleteVertexArrays(1, &prismVAO);
    if (prismVBO != 0) glDeleteBuffers(1, &prismVBO);
    if (prismEBO != 0) glDeleteBuffers(1, &prismEBO);

    if (sphereVAO != 0) glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO != 0) glDeleteBuffers(1, &sphereVBO);
    if (sphereEBO != 0) glDeleteBuffers(1, &sphereEBO);

    if (torusVAO != 0) glDeleteVertexArrays(1, &torusVAO);
    if (torusVBO != 0) glDeleteBuffers(1, &torusVBO);
    if (torusEBO != 0) glDeleteBuffers(1, &torusEBO);
}

// =================================================================
// INTERNAL BUFFER GENERATION HELPER
// =================================================================
void ShapeMeshes::BuildMesh(GLuint& vao, GLuint& vbo, const GLfloat* vertices, int vertexCount) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Every vertex contains 8 floats (3 position, 3 normal, 2 UV)
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // Position attribute (0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // UV attribute (2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// =================================================================
// PROCEDURAL CYLINDER & CONE GENERATOR
// =================================================================
void GenerateSubdividedCylinder(GLuint& vao, GLuint& vbo, GLuint& ebo, GLsizei& indexCount, float bottomRadius, float topRadius, float height, int slices) {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    float halfH = height / 2.0f;

    GLuint vertexCounter = 0;

    // 1. Generate Side Vertices
    for (int i = 0; i <= slices; ++i) {
        float angle = (float)i / slices * 2.0f * glm::pi<float>();
        float cosA = cos(angle);
        float sinA = sin(angle);

        float slope = (bottomRadius - topRadius) / height;
        glm::vec3 normal = glm::normalize(glm::vec3(cosA, slope, sinA));

        // Bottom side ring
        vertices.push_back(bottomRadius * cosA); vertices.push_back(-halfH); vertices.push_back(bottomRadius * sinA);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
        vertices.push_back((float)i / slices); vertices.push_back(0.0f);
        vertexCounter++;

        // Top side ring
        vertices.push_back(topRadius * cosA); vertices.push_back(halfH); vertices.push_back(topRadius * sinA);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
        vertices.push_back((float)i / slices); vertices.push_back(1.0f);
        vertexCounter++;
    }

    // 2. Side Indices
    for (int i = 0; i < slices; ++i) {
        indices.push_back(i * 2); indices.push_back(i * 2 + 1); indices.push_back(i * 2 + 2);
        indices.push_back(i * 2 + 2); indices.push_back(i * 2 + 1); indices.push_back(i * 2 + 3);
    }

    // 3. Bottom Cap Center
    GLuint bottomCenterIdx = vertexCounter;
    vertices.push_back(0.0f); vertices.push_back(-halfH); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
    vertices.push_back(0.5f); vertices.push_back(0.5f);
    vertexCounter++;

    // Bottom Cap Ring
    GLuint bottomRingStartIdx = vertexCounter;
    for (int i = 0; i <= slices; ++i) {
        float angle = (float)i / slices * 2.0f * glm::pi<float>();
        vertices.push_back(bottomRadius * cos(angle)); vertices.push_back(-halfH); vertices.push_back(bottomRadius * sin(angle));
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(0.5f + 0.5f * cos(angle)); vertices.push_back(0.5f + 0.5f * sin(angle));
        vertexCounter++;

        if (i < slices) {
            indices.push_back(bottomCenterIdx);
            indices.push_back(bottomRingStartIdx + i + 1);
            indices.push_back(bottomRingStartIdx + i);
        }
    }

    // 4. Top Cap Center
    GLuint topCenterIdx = vertexCounter;
    vertices.push_back(0.0f); vertices.push_back(halfH); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
    vertices.push_back(0.5f); vertices.push_back(0.5f);
    vertexCounter++;

    // Top Cap Ring
    GLuint topRingStartIdx = vertexCounter;
    for (int i = 0; i <= slices; ++i) {
        float angle = (float)i / slices * 2.0f * glm::pi<float>();
        vertices.push_back(topRadius * cos(angle)); vertices.push_back(halfH); vertices.push_back(topRadius * sin(angle));
        vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
        vertices.push_back(0.5f + 0.5f * cos(angle)); vertices.push_back(0.5f + 0.5f * sin(angle));
        vertexCounter++;

        if (i < slices) {
            indices.push_back(topCenterIdx);
            indices.push_back(topRingStartIdx + i);
            indices.push_back(topRingStartIdx + i + 1);
        }
    }

    // 5. Buffer Allocation & Setup
    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// =================================================================
// STANDARD MESH LOADING IMPLEMENTATIONS
// =================================================================

void ShapeMeshes::LoadBoxMesh() {
    std::vector<GLfloat> vertices = {
        // Back Face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        // Front Face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        // Left Face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        // Right Face
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        // Bottom Face
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

       // Top Face
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
       0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };
    BuildMesh(boxVAO, boxVBO, vertices.data(), 36);
}

void ShapeMeshes::LoadPlaneMesh() {
    std::vector<GLfloat> vertices = {
        -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
         0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
         0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,   0.0f, 1.0f, 0.0f,     0.0f, 0.0f
    };
    BuildMesh(planeVAO, planeVBO, vertices.data(), 6);
}

void ShapeMeshes::LoadCylinderMesh() {
    GenerateSubdividedCylinder(cylinderVAO, cylinderVBO, cylinderEBO, cylinderIndicesCount, 0.5f, 0.5f, 1.0f, 32);
}

void ShapeMeshes::LoadConeMesh() {
    GenerateSubdividedCylinder(coneVAO, coneVBO, coneEBO, coneIndicesCount, 0.5f, 0.0f, 1.0f, 32);
}

void ShapeMeshes::LoadSphereMesh() {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    const unsigned int X_SEGMENTS = 32;
    const unsigned int Y_SEGMENTS = 32;

    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>()) * 0.5f;
            float yPos = std::cos(ySegment * glm::pi<float>()) * 0.5f;
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>()) * 0.5f;

            vertices.push_back(xPos); vertices.push_back(yPos); vertices.push_back(zPos);
            glm::vec3 norm = glm::normalize(glm::vec3(xPos, yPos, zPos));
            vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);
            vertices.push_back(xSegment); vertices.push_back(ySegment);
        }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
            indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
        }
    }

    sphereIndicesCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void ShapeMeshes::LoadPrismMesh() {
    std::vector<GLfloat> vertices = {
        // Front Triangular Face
         0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,

         // Back Triangular Face
          0.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.5f, 1.0f,
          0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

         // Left Slanted Face
         -0.5f, -0.5f, -0.5f, -0.894f,  0.447f,  0.0f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f, -0.894f,  0.447f,  0.0f,  0.0f, 0.0f,
          0.0f,  0.5f,  0.5f, -0.894f,  0.447f,  0.0f,  1.0f, 0.0f,
          0.0f,  0.5f,  0.5f, -0.894f,  0.447f,  0.0f,  1.0f, 0.0f,
          0.0f,  0.5f, -0.5f, -0.894f,  0.447f,  0.0f,  1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f, -0.894f,  0.447f,  0.0f,  0.0f, 1.0f,

         // Right Slanted Face
          0.0f,  0.5f, -0.5f,  0.894f,  0.447f,  0.0f,  0.0f, 1.0f,
          0.0f,  0.5f,  0.5f,  0.894f,  0.447f,  0.0f,  0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.894f,  0.447f,  0.0f,  1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.894f,  0.447f,  0.0f,  1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  0.894f,  0.447f,  0.0f,  1.0f, 1.0f,
          0.0f,  0.5f, -0.5f,  0.894f,  0.447f,  0.0f,  0.0f, 1.0f,

          // Bottom Face
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };
    BuildMesh(prismVAO, prismVBO, vertices.data(), 24);
}

// =================================================================
// PROCEDURAL GEOMETRY COMPLETION LAYER
// =================================================================

void ShapeMeshes::LoadTaperedCylinderMesh() {
    GenerateSubdividedCylinder(taperedCylinderVAO, taperedCylinderVBO, taperedCylinderEBO, taperedCylinderIndicesCount, 0.5f, 0.3f, 1.0f, 32);
}

void ShapeMeshes::LoadPyramid4Mesh() {
    std::vector<GLfloat> vertices = {
        // Front Face
         0.0f,  0.5f,  0.0f,  0.0f,  0.447f,  0.894f,  0.5f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.447f,  0.894f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.447f,  0.894f,  1.0f, 0.0f,

         // Right Face
          0.0f,  0.5f,  0.0f,  0.894f,  0.447f,  0.0f,  0.5f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.894f,  0.447f,  0.0f,  0.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  0.894f,  0.447f,  0.0f,  1.0f, 0.0f,

          // Back Face
           0.0f,  0.5f,  0.0f,  0.0f,  0.447f, -0.894f,  0.5f, 1.0f,
           0.5f, -0.5f, -0.5f,  0.0f,  0.447f, -0.894f,  0.0f, 0.0f,
          -0.5f, -0.5f, -0.5f,  0.0f,  0.447f, -0.894f,  1.0f, 0.0f,

          // Left Face
           0.0f,  0.5f,  0.0f, -0.894f,  0.447f,  0.0f,  0.5f, 1.0f,
          -0.5f, -0.5f, -0.5f, -0.894f,  0.447f,  0.0f,  0.0f, 0.0f,
          -0.5f, -0.5f,  0.5f, -0.894f,  0.447f,  0.0f,  1.0f, 0.0f,

          // Bottom Base Face Triangle 1
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
           // Bottom Base Face Triangle 2
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
           -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };
    BuildMesh(pyramid4VAO, pyramid4VBO, vertices.data(), 18);
}

void ShapeMeshes::LoadTorusMesh() {
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    const unsigned int MAIN_SEGMENTS = 32;
    const unsigned int TUBE_SEGMENTS = 16;
    const float mainRadius = 0.4f;
    const float tubeRadius = 0.1f;

    for (unsigned int i = 0; i <= MAIN_SEGMENTS; ++i) {
        float mainAngle = (float)i / MAIN_SEGMENTS * 2.0f * glm::pi<float>();
        float cosMain = std::cos(mainAngle);
        float sinMain = std::sin(mainAngle);

        for (unsigned int j = 0; j <= TUBE_SEGMENTS; ++j) {
            float tubeAngle = (float)j / TUBE_SEGMENTS * 2.0f * glm::pi<float>();
            float cosTube = std::cos(tubeAngle);
            float sinTube = std::sin(tubeAngle);

            // Ring Coordinates
            float xPos = (mainRadius + tubeRadius * cosTube) * cosMain;
            float yPos = tubeRadius * sinTube;
            float zPos = (mainRadius + tubeRadius * cosTube) * sinMain;
            vertices.push_back(xPos); vertices.push_back(yPos); vertices.push_back(zPos);

            // Normal Vector calculations pointing outward from tube core
            glm::vec3 normal(cosTube * cosMain, sinTube, cosTube * sinMain);
            normal = glm::normalize(normal);
            vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

            // Texturing UV mappings
            vertices.push_back((float)i / MAIN_SEGMENTS);
            vertices.push_back((float)j / TUBE_SEGMENTS);
        }
    }

    for (unsigned int i = 0; i < MAIN_SEGMENTS; ++i) {
        for (unsigned int j = 0; j < TUBE_SEGMENTS; ++j) {
            unsigned int nextI = i + 1;
            unsigned int nextJ = j + 1;

            indices.push_back(i * (TUBE_SEGMENTS + 1) + j);
            indices.push_back(nextI * (TUBE_SEGMENTS + 1) + j);
            indices.push_back(nextI * (TUBE_SEGMENTS + 1) + nextJ);

            indices.push_back(i * (TUBE_SEGMENTS + 1) + j);
            indices.push_back(nextI * (TUBE_SEGMENTS + 1) + nextJ);
            indices.push_back(i * (TUBE_SEGMENTS + 1) + nextJ);
        }
    }

    torusIndicesCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &torusVAO);
    glGenBuffers(1, &torusVBO);
    glGenBuffers(1, &torusEBO);

    glBindVertexArray(torusVAO);
    glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// =================================================================
// RENDER / DRAW MESH IMPLEMENTATIONS
// =================================================================

void ShapeMeshes::DrawBoxMesh() {
    glBindVertexArray(boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawPlaneMesh() {
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawCylinderMesh() {
    glBindVertexArray(cylinderVAO);
    glDrawElements(GL_TRIANGLES, cylinderIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawConeMesh() {
    glBindVertexArray(coneVAO);
    glDrawElements(GL_TRIANGLES, coneIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawSphereMesh() {
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawPrismMesh() {
    glBindVertexArray(prismVAO);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawTaperedCylinderMesh() {
    glBindVertexArray(taperedCylinderVAO);
    glDrawElements(GL_TRIANGLES, taperedCylinderIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawPyramid4Mesh() {
    glBindVertexArray(pyramid4VAO);
    glDrawArrays(GL_TRIANGLES, 0, 18);
    glBindVertexArray(0);
}

void ShapeMeshes::DrawTorusMesh() {
    glBindVertexArray(torusVAO);
    glDrawElements(GL_TRIANGLES, torusIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}