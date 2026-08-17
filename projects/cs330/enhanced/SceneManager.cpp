///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
// Enhanced for CS-499 Computer Science Capstone
///////////////////////////////////////////////////////////////////////////////

#define GLM_ENABLE_EXPERIMENTAL

#include "SceneManager.h"
#include <memory>
#include <iostream>
#include <glm/gtx/transform.hpp>

#include "stb_image.h"
#include "ShaderManager.h"

// Declaration of global shader uniform naming variables
namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager),
    m_loadedTextures(0)
{
    // Object-Oriented design patterns using modular resource allocation
    m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
    m_pShaderManager = nullptr;
    m_objectMaterials.clear();
    if (m_basicMeshes != nullptr)
    {
        delete m_basicMeshes;
        m_basicMeshes = nullptr;
    }
}

/***********************************************************
 *  CreateGLTexture()
 *  Loads, configures, and registers an OpenGL 2D graphic texture
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0;
    int height = 0;
    int colorChannels = 0;
    GLuint textureID = 0;

    // Standardize texture layout orientation parsing alignments
    stbi_set_flip_vertically_on_load(true);

    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

    if (image)
    {
        std::cout << "Successfully loaded image: " << filename
            << " | Width: " << width << " | Height: " << height
            << " | Channels: " << colorChannels << std::endl;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set structural texture wrapping specifications
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set texture filtration behaviors
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Map colors across available channels
        if (colorChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (colorChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            std::cerr << "WARNING: Execution failure handling image with " << colorChannels << " channels\n";
            stbi_image_free(image);
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Populate object data records
        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;

        return true;
    }

    std::cerr << "ERROR: Failed to load image asset path: " << filename << std::endl;
    return false;
}

/***********************************************************
 *  BindGLTextures()
 ***********************************************************/
void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

/***********************************************************
 *  DestroyGLTextures()
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
    m_loadedTextures = 0;
}

/***********************************************************
 *  FindTextureID()
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
        {
            return m_textureIDs[i].ID;
        }
    }
    return -1;
}

/***********************************************************
 *  FindTextureSlot()
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
        {
            return i;
        }
    }
    return -1;
}

/***********************************************************
 *  FindMaterial()
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (const auto& mat : m_objectMaterials)
    {
        if (mat.tag == tag)
        {
            material = mat;
            return true;
        }
    }
    return false;
}

/***********************************************************
 *  SetTransformations()
 ***********************************************************/
void SceneManager::SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ)
{
    glm::mat4 scale = glm::scale(scaleXYZ);
    glm::mat4 rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translation = glm::translate(positionXYZ);

    glm::mat4 modelView = translation * rotationZ * rotationY * rotationX * scale;

    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
    }
}

/***********************************************************
 *  SetShaderColor()
 ***********************************************************/
void SceneManager::SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue)
{
    glm::vec4 currentColor(redColorValue, greenColorValue, blueColorValue, alphaValue);

    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}

/***********************************************************
 *  SetShaderTexture()
 ***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        int textureSlot = FindTextureSlot(textureTag);
        if (textureSlot != -1)
        {
            m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
        }
    }
}

/***********************************************************
 *  SetTextureUVScale()
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}

/***********************************************************
 *  SetShaderMaterial()
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL material;
    if (FindMaterial(materialTag, material) && m_pShaderManager != nullptr)
    {
        m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
        m_pShaderManager->setFloatValue("material.shininess", material.shininess);
    }
}

/***********************************************************
 *  LoadSceneTextures()
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
    CreateGLTexture("textures/wall.jpg", "wall");
    CreateGLTexture("textures/stairs.jpg", "stairs");
    CreateGLTexture("textures/railing.jpg", "railing");
    CreateGLTexture("textures/path.jpg", "path");
    CreateGLTexture("textures/ground.jpg", "ground");
    CreateGLTexture("textures/roof.jpg", "roof");

    BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
    OBJECT_MATERIAL wall;
    wall.ambientColor = glm::vec3(0.30f, 0.29f, 0.32f);
    wall.ambientStrength = 0.09f;
    wall.diffuseColor = glm::vec3(0.46f, 0.43f, 0.51f);
    wall.specularColor = glm::vec3(0.11f, 0.13f, 0.17f);
    wall.shininess = 6.0f;
    wall.tag = "wall";
    m_objectMaterials.push_back(wall);

    OBJECT_MATERIAL stairs;
    stairs.ambientColor = glm::vec3(0.22f, 0.21f, 0.28f);
    stairs.ambientStrength = 0.09f;
    stairs.diffuseColor = glm::vec3(0.38f, 0.36f, 0.47f);
    stairs.specularColor = glm::vec3(0.09f, 0.10f, 0.14f);
    stairs.shininess = 8.0f;
    stairs.tag = "stairs";
    m_objectMaterials.push_back(stairs);

    OBJECT_MATERIAL ground;
    ground.ambientColor = glm::vec3(0.19f, 0.19f, 0.27f);
    ground.ambientStrength = 0.08f;
    ground.diffuseColor = glm::vec3(0.28f, 0.29f, 0.39f);
    ground.specularColor = glm::vec3(0.10f, 0.11f, 0.14f);
    ground.shininess = 4.0f;
    ground.tag = "ground";
    m_objectMaterials.push_back(ground);

    OBJECT_MATERIAL metal;
    metal.ambientColor = glm::vec3(0.23f, 0.23f, 0.27f);
    metal.ambientStrength = 0.09f;
    metal.diffuseColor = glm::vec3(0.34f, 0.35f, 0.43f);
    metal.specularColor = glm::vec3(0.45f, 0.46f, 0.54f);
    metal.shininess = 24.0f;
    metal.tag = "metal";
    m_objectMaterials.push_back(metal);

    OBJECT_MATERIAL railing;
    railing.ambientColor = glm::vec3(0.20f, 0.23f, 0.33f);
    railing.ambientStrength = 0.14f;
    railing.diffuseColor = glm::vec3(0.28f, 0.40f, 0.60f);
    railing.specularColor = glm::vec3(0.50f, 0.55f, 0.65f);
    railing.shininess = 64.0f;
    railing.tag = "railing";
    m_objectMaterials.push_back(railing);

    OBJECT_MATERIAL roof;
    roof.ambientColor = glm::vec3(0.19f, 0.23f, 0.35f);
    roof.ambientStrength = 0.12f;
    roof.diffuseColor = glm::vec3(0.25f, 0.34f, 0.48f);
    roof.specularColor = glm::vec3(0.18f, 0.22f, 0.33f);
    roof.shininess = 36.0f;
    roof.tag = "roof";
    m_objectMaterials.push_back(roof);

    OBJECT_MATERIAL glass;
    glass.ambientColor = glm::vec3(0.10f, 0.15f, 0.25f);
    glass.ambientStrength = 0.20f;
    glass.diffuseColor = glm::vec3(0.60f, 0.80f, 0.95f);
    glass.specularColor = glm::vec3(0.90f, 0.95f, 1.00f);
    glass.shininess = 96.0f;
    glass.tag = "glass";
    m_objectMaterials.push_back(glass);
}

/***********************************************************
 *  SetupSceneLights()
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
    m_pShaderManager->use();
    m_pShaderManager->setBoolValue(g_UseLightingName, true);

    // Moonlight configurations (Directional Light)
    m_pShaderManager->setBoolValue("directionalLight.bActive", true);
    m_pShaderManager->setVec3Value("directionalLight.direction", -0.30f, -1.0f, -0.10f);
    m_pShaderManager->setVec3Value("directionalLight.ambient", 0.05f, 0.09f, 0.18f);
    m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.10f, 0.20f, 0.40f);
    m_pShaderManager->setVec3Value("directionalLight.specular", 0.25f, 0.33f, 0.50f);

    // Lamp positions for the 5 street lamps
    glm::vec3 lampPositions[] = {
        glm::vec3(-16.0f, 8.0f, 5.2f),
        glm::vec3(17.7f, 6.5f, -1.2f),
        glm::vec3(25.7f, 9.5f, 52.2f),
        glm::vec3(-35.7f, 9.5f, 52.2f),
        glm::vec3(7.7f, 6.5f, -36.2f)
    };

    // Configure the 5 street lamps to cast soft, localized warm light
    for (int i = 0; i < 5; ++i)
    {
        std::string baseName = "pointLights[" + std::to_string(i) + "].";
        m_pShaderManager->setBoolValue(baseName + "bActive", true);
        m_pShaderManager->setVec3Value(baseName + "position", lampPositions[i]);

        // Warm amber/yellow tones for the bulb glow
        m_pShaderManager->setVec3Value(baseName + "ambient", 0.05f, 0.04f, 0.02f);
        m_pShaderManager->setVec3Value(baseName + "diffuse", 0.55f, 0.45f, 0.30f);   // Dim, soft illumination strength
        m_pShaderManager->setVec3Value(baseName + "specular", 0.30f, 0.25f, 0.15f);

        // Attenuation settings optimized to restrict light to the nearby area
        m_pShaderManager->setFloatValue(baseName + "constant", 1.0f);
        m_pShaderManager->setFloatValue(baseName + "linear", 0.14f);      // Controls how fast light begins to fade
        m_pShaderManager->setFloatValue(baseName + "quadratic", 0.07f);   // Tightens the radius so it stays local
    }

    // Sky Dome Moon Light (Point Light 5)
    m_pShaderManager->setBoolValue("pointLights[5].bActive", true);
    m_pShaderManager->setVec3Value("pointLights[5].position", 60.0f, 100.0f, -80.3f);
    m_pShaderManager->setVec3Value("pointLights[5].ambient", 0.05f, 0.09f, 0.20f);
    m_pShaderManager->setVec3Value("pointLights[5].diffuse", 0.08f, 0.07f, 0.04f);
    m_pShaderManager->setVec3Value("pointLights[5].specular", 0.20f, 0.18f, 0.10f);
    m_pShaderManager->setFloatValue("pointLights[5].constant", 1.0f);
    m_pShaderManager->setFloatValue("pointLights[5].linear", 0.035f);
    m_pShaderManager->setFloatValue("pointLights[5].quadratic", 0.015f);
}

/***********************************************************
 *  PrepareScene()
 ***********************************************************/
void SceneManager::PrepareScene()
{
    DefineObjectMaterials();
    SetupSceneLights();
    LoadSceneTextures();

    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadPrismMesh();
    m_basicMeshes->LoadPyramid4Mesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();
    m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 ***********************************************************/
void SceneManager::RenderScene()
{
    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->use();
    }
    RenderEnvironment();
    RenderStairs();
    RenderArchitecture();
    RenderRailings();
    RenderBuildings();
}

/***********************************************************
 *  RenderEnvironment()
 ***********************************************************/
void SceneManager::RenderEnvironment()
{
    // Ground Setup
    SetTransformations(glm::vec3(60.5f, 1.0f, 60.5f), 0, 0, 0, glm::vec3(0.0f, 0.0f, 0.0f));
    SetShaderTexture("ground");
    SetShaderMaterial("ground");
    m_basicMeshes->DrawPlaneMesh();

    // Backdrop Moon Rendering
    SetTransformations(glm::vec3(23.0f, 23.0f, 1.0f), 0, 0, 0, glm::vec3(60.0f, 100.0f, -80.0f));
    SetShaderColor(0.95f, 0.95f, 1.0f, 1.0f);
    m_basicMeshes->DrawSphereMesh();

    // Moon Glassy Overlay
    SetTransformations(glm::vec3(20.0f, 20.0f, 1.0f), 0, 0, 0, glm::vec3(60.0f, 100.0f, -80.3f));
    SetShaderColor(0.70f, 0.85f, 1.0f, 1.0f);
    SetShaderMaterial("glass");
    m_basicMeshes->DrawSphereMesh();
}

/***********************************************************
 *  RenderRailings()
 ***********************************************************/
void SceneManager::RenderRailings()
{
    SetShaderTexture("railing");
    SetShaderMaterial("railing");

    // Hotel Balcony Handrails
    SetTransformations(glm::vec3(0.2f, 1.0f, 20.0f), 0, 0, 0, glm::vec3(-37.0f, 9.5f, 41.4f));
    m_basicMeshes->DrawBoxMesh();

    // Balcony Pickets Loop
    for (float z = 31.4f; z <= 51.4f; z += 1.25f)
    {
        SetTransformations(glm::vec3(0.05f, 1.0f, 0.05f), 0, 0, 0, glm::vec3(-37.0f, 9.0f, z));
        m_basicMeshes->DrawCylinderMesh();
    }

    // Cafe Plaza Handrails
    SetTransformations(glm::vec3(14.6f, 1.0f, 0.2f), 0, 0, 0, glm::vec3(34.9f, 15.3f, 25.8f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(20.0f, 1.0f, 0.2f), 0, 0, 0, glm::vec3(24.0f, 15.3f, 41.3f));
    m_basicMeshes->DrawBoxMesh();

    // Cafe Plaza Pickets Loop
    for (float x = 14.0f; x <= 34.0f; x += 2.0f)
    {
        SetTransformations(glm::vec3(0.1f, 1.0f, 0.1f), 0, 0, 0, glm::vec3(x, 14.8f, 41.3f));
        m_basicMeshes->DrawCylinderMesh();
    }
}

/***********************************************************
 *  RenderStairs()
 ***********************************************************/
void SceneManager::RenderStairs()
{
    const float h = 0.5f, d = 0.8f;

    // Left Stairway Set
    for (int i = 0; i < 16; ++i)
    {
        SetTransformations(glm::vec3(6.0f, h, d), 0, 0, 0, glm::vec3(-23.0f, (h * 0.5f + i * h), (5.2f - i * d)));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Right Courtyard Steps
    for (int i = 0; i < 6; ++i)
    {
        SetTransformations(glm::vec3(20.0f, h, d), 0, 180.0f, 0, glm::vec3(-2.0f, (h * 0.5f + i * h), (-5.2f + i * d) + 30.0f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Bridge Steps
    for (int i = 0; i < 4; ++i)
    {
        SetTransformations(glm::vec3(4.0f, h, d), 0, 180.0f, 0, glm::vec3(-47.0f, (h * 0.5f + i * h) + 7.5f, (-5.2f + i * d) + 9.5f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Cafe Shop Stairs
    for (int i = 0; i < 10; ++i)
    {
        SetTransformations(glm::vec3(6.0f, h, d), 0, 180.0f, 0, glm::vec3(22.5f, (h * 0.5f + i * h), (-5.2f + i * d) - 16.6f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Back Pathway Set
    const float backH = 0.8f, backD = 0.5f;
    for (int i = 0; i < 11; ++i)
    {
        SetTransformations(glm::vec3(d, h, 6.0f), 0, -180.0f, 0, glm::vec3((backH * 1.3f + i * backH) + 37.9f, (5.2f + i * backD) + 0.4f, 3.8f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Hotel Entrance Steps
    for (int i = 0; i < 8; ++i)
    {
        SetTransformations(glm::vec3(d, h, 10.0f), 0, 0, 0, glm::vec3((backH * 1.3f + i * backH) - 37.9f, (-5.2f - i * backD) + 12.0f, 40.8f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
    // Hotel Lobby Steps
    for (int i = 0; i < 8; ++i)
    {
        SetTransformations(glm::vec3(d, h, 4.0f), 0, 0, 0, glm::vec3((backH * 1.3f + i * backH) + 40.9f, (-5.2f - i * backD) + 19.5f, 25.8f));
        SetShaderTexture("stairs");
        SetShaderMaterial("stairs");
        m_basicMeshes->DrawCylinderMesh();
    }
}

/***********************************************************
 *  RenderArchitecture()
 ***********************************************************/
void SceneManager::RenderArchitecture()
{
    SetShaderMaterial("ground");
    SetShaderTexture("path");

    // Walkway Blocks
    SetTransformations(glm::vec3(59.7f, 0.4f, 12.0f), 0, 0, 0, glm::vec3(-30.0f, 0.5f, 15.0f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(54.7f, 0.4f, 12.0f), 0, 90.0f, 0, glm::vec3(0.0f, 0.5f, -12.5f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(59.7f, 0.4f, 12.0f), 0, 0, 0, glm::vec3(-30.0f, 0.5f, -40.8f));
    m_basicMeshes->DrawBoxMesh();

    // Curved Corner Meshes
    SetTransformations(glm::vec3(6.3f, 0.4f, 6.3f), 0, 90.0f, 0, glm::vec3(-0.3f, 0.3f, 14.7f));
    m_basicMeshes->DrawCylinderMesh();

    SetTransformations(glm::vec3(6.3f, 0.4f, 6.3f), 0, 90.0f, 0, glm::vec3(-0.3f, 0.3f, -40.5f));
    m_basicMeshes->DrawCylinderMesh();

    // Foundations, Bridges, and structural dividers
    SetShaderTexture("stairs");
    SetShaderMaterial("stairs");

    SetTransformations(glm::vec3(12.0f, 14.0f, 12.0f), -58.0f, 0, 0, glm::vec3(-23.0f, -0.9f, -4.0f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(12.0f, 14.0f, 12.0f), 58.0f, 0, 0, glm::vec3(22.6f, -3.6f, -17.2f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(12.0f, 14.0f, 12.0f), 58.0f, 90.0f, 0, glm::vec3(44.1f, 1.9f, 3.9f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(18.0f, 14.0f, 18.0f), 32.0f, 90.0f, 0, glm::vec3(-37.9f, -0.6f, 40.8f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(8.0f, 1.0f, 22.6f), 0, 0, 0, glm::vec3(-47.0f, 9.2f, 17.7f));
    m_basicMeshes->DrawBoxMesh();

    // Raised Platforms and Foundations
    SetTransformations(glm::vec3(38.0f, 8.5f, 20.0f), 0, 0, 0, glm::vec3(-36.0f, 4.0f, -16.8f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(26.0f, 8.5f, 11.0f), 0, 0, 0, glm::vec3(-42.0f, 4.0f, -1.4f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(25.0f, 3.5f, 120.6f), 0, 90.0f, 0, glm::vec3(3.3f, 1.42f, 41.5f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(11.5f, 5.5f, 11.5f), 0, 0, 0, glm::vec3(22.3f, 2.5f, -8.8f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(7.0f, 5.5f, 7.0f), 0, 0, 0, glm::vec3(34.2f, 0.0f, 2.9f));
    m_basicMeshes->DrawCylinderMesh();

    SetTransformations(glm::vec3(10.0f, 7.0f, 13.0f), 0, 90.0f, 0, glm::vec3(40.8f, 2.0f, 4.9f));
    m_basicMeshes->DrawBoxMesh();

    // Street Kiosks
    SetTransformations(glm::vec3(5.0f, 3.0f, 5.0f), 0, 0, 0, glm::vec3(16.0f, 0.26f, 28.0f));
    m_basicMeshes->DrawCylinderMesh();

    SetTransformations(glm::vec3(5.0f, 3.0f, 5.0f), 0, 0, 0, glm::vec3(-25.0f, 0.26f, 28.0f));
    m_basicMeshes->DrawCylinderMesh();

    // Raised Cafe Plazas
    SetTransformations(glm::vec3(8.0f, 1.5f, 14.6f), 0, 90.0f, 0, glm::vec3(34.9f, 14.0f, 25.8f));
    m_basicMeshes->DrawBoxMesh();

    SetTransformations(glm::vec3(8.0f, 1.5f, 20.0f), 0, 90.0f, 0, glm::vec3(24.0f, 14.0f, 41.3f));
    m_basicMeshes->DrawBoxMesh();

    // Lamp Posts & Glowing Lightbulbs
    glm::vec3 lampPositions[] = {
        glm::vec3(-16.0f, 8.0f, 5.2f),
        glm::vec3(17.7f, 6.5f, -1.2f),
        glm::vec3(25.7f, 9.5f, 52.2f),
        glm::vec3(-35.7f, 9.5f, 52.2f),
        glm::vec3(7.7f, 6.5f, -36.2f)
    };

    for (int i = 0; i < 5; ++i)
    {
        // 1. Draw the structural lamp post base/pole
        SetTransformations(glm::vec3(0.3f, 8.0f, 0.3f), 0, 0, 0, glm::vec3(lampPositions[i].x, lampPositions[i].y - 4.0f, lampPositions[i].z));
        SetShaderTexture("railing");
        SetShaderMaterial("metal");
        m_basicMeshes->DrawCylinderMesh();

        // 2. CRITICAL FIX: Turn off scene lighting calculations temporarily so the light source bulb properties 
        // glow unshaded and are not affected by surrounding room light math structures.
        if (m_pShaderManager != nullptr)
        {
            m_pShaderManager->setBoolValue(g_UseLightingName, false);
        }

        SetTransformations(glm::vec3(0.8f, 0.8f, 0.8f), 0, 0, 0, lampPositions[i]);
        SetShaderColor(1.0f, 0.95f, 0.8f, 1.0f); // Pure white/yellow emission values
        m_basicMeshes->DrawSphereMesh();

        // 3. Turn lighting back on for subsequent structural meshes
        if (m_pShaderManager != nullptr)
        {
            m_pShaderManager->setBoolValue(g_UseLightingName, true);
        }
    }
}

/***********************************************************
 *  RenderBuildings()
 ***********************************************************/
void SceneManager::RenderBuildings()
{
    // Building Walls
    SetShaderTexture("wall");
    SetShaderMaterial("wall");

    // La Bonne Affaire (Facade)
    SetTransformations(glm::vec3(35.0f, 34.0f, 36.0f), 0, 90.0f, 0, glm::vec3(45.5f, 17.0f, -16.8f));
    m_basicMeshes->DrawBoxMesh();
    // Cafe Plaza Wall
    SetTransformations(glm::vec3(30.0f, 11.2f, 6.0f), 0, 90.0f, 0, glm::vec3(50.3f, 5.5f, 14.5f));
    m_basicMeshes->DrawBoxMesh();
    // Back Wall Divider
    SetTransformations(glm::vec3(30.0f, 17.0f, 10.0f), 0, 90.0f, 0, glm::vec3(58.5f, 8.0f, 14.5f));
    m_basicMeshes->DrawBoxMesh();
    // Hotel Check-In Wall
    SetTransformations(glm::vec3(25.0f, 30.0f, 36.0f), 0, 90.0f, 0, glm::vec3(45.5f, 18.0f, 41.4f));
    m_basicMeshes->DrawBoxMesh();
    // Hotel Wall
    SetTransformations(glm::vec3(25.0f, 44.0f, 20.0f), 0, 90.0f, 0, glm::vec3(-47.0f, 25.2f, 41.4f));
    m_basicMeshes->DrawBoxMesh();
    // Police Station Facade
    SetTransformations(glm::vec3(35.0f, 34.0f, 10.0f), 0, 90.0f, 0, glm::vec3(-55.8f, 17.0f, -10.8f));
    m_basicMeshes->DrawBoxMesh();

    // Roofs for the buildings
    SetShaderMaterial("roof");

    // La Bonne Affaire Roof
    SetTransformations(glm::vec3(36.4f, 35.4f, 16.5f), -90.0f, 0, 0, glm::vec3(45.5f, 42.2f, -16.8f));
    SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
    m_basicMeshes->DrawPrismMesh();
    // Hotel Check-In Roof
    SetTransformations(glm::vec3(36.5f, 25.4f, 10.5f), -90.0f, 0, 0, glm::vec3(45.5f, 38.3f, 41.4f));
    SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
    m_basicMeshes->DrawPrismMesh();
    // Hotel Roof 
    SetTransformations(glm::vec3(20.5f, 25.5f, 10.5f), -90.0f, 0, 0, glm::vec3(-47.0f, 52.4f, 41.4f));
    SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
    m_basicMeshes->DrawPrismMesh();
    // Police Station Roof
    SetTransformations(glm::vec3(10.5f, 35.5f, 6.5f), -90.0f, 0, 0, glm::vec3(-55.8f, 37.3f, -10.8f));
    SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
    m_basicMeshes->DrawPrismMesh();
}