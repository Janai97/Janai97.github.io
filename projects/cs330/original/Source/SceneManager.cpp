///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
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
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// free up the allocated memory
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}
	// clear the collection of defined materials
	m_objectMaterials.clear();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/wall.jpg",  //image path
		"wall"                                       // tag = "wall"
	);
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/stairs.jpg",  //image path
		"stairs"                                       // tag = "stairs"
	);
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/railing.jpg",  //image path
		"railing"                                       // tag = "railing"
	);
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/path.jpg",  //image path
		"path"                                       // tag = "path"
	);
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/ground.jpg",  //image path
		"ground"                                       // tag = "ground"
	);
	bReturn = CreateGLTexture("../7-1_FinalProjectMilestones/textures/roof.jpg",  //image path
		"roof"                                       // tag = "roof"
	);

	BindGLTextures();
}
/***********************************************************
  *  DefineObjectMaterials()
  *
  *  This method is used for configuring the various material
  *  settings for all of the objects within the 3D scene.
  ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Wall material: blue hour, faint warm
	OBJECT_MATERIAL wall;
	wall.ambientColor = glm::vec3(0.30f, 0.29f, 0.32f);
	wall.ambientStrength = 0.09f;
	wall.diffuseColor = glm::vec3(0.46f, 0.43f, 0.51f);
	wall.specularColor = glm::vec3(0.11f, 0.13f, 0.17f);
	wall.shininess = 6.0f;
	wall.tag = "wall";
	m_objectMaterials.push_back(wall);

	// Stair material: low ambient, slightly glossy
	OBJECT_MATERIAL stairs;
	stairs.ambientColor = glm::vec3(0.22f, 0.21f, 0.28f);
	stairs.ambientStrength = 0.09f;
	stairs.diffuseColor = glm::vec3(0.38f, 0.36f, 0.47f);
	stairs.specularColor = glm::vec3(0.09f, 0.10f, 0.14f);
	stairs.shininess = 8.0f;
	stairs.tag = "stairs";
	m_objectMaterials.push_back(stairs);

	// Ground material: even lower ambient, less contrast
	OBJECT_MATERIAL ground;
	ground.ambientColor = glm::vec3(0.19f, 0.19f, 0.27f);
	ground.ambientStrength = 0.08f;
	ground.diffuseColor = glm::vec3(0.28f, 0.29f, 0.39f);
	ground.specularColor = glm::vec3(0.10f, 0.11f, 0.14f);
	ground.shininess = 4.0f;
	ground.tag = "ground";
	m_objectMaterials.push_back(ground);

	// Metal: cold reflection, more subtle ambient
	OBJECT_MATERIAL metal;
	metal.ambientColor = glm::vec3(0.23f, 0.23f, 0.27f);
	metal.ambientStrength = 0.09f;
	metal.diffuseColor = glm::vec3(0.34f, 0.35f, 0.43f);
	metal.specularColor = glm::vec3(0.45f, 0.46f, 0.54f);
	metal.shininess = 24.0f;
	metal.tag = "metal";
	m_objectMaterials.push_back(metal);

	// Railing: dark, low reflection
	OBJECT_MATERIAL railing;
	railing.ambientColor = glm::vec3(0.20f, 0.23f, 0.33f);
	railing.ambientStrength = 0.14f;
	railing.diffuseColor = glm::vec3(0.28f, 0.40f, 0.60f);
	railing.specularColor = glm::vec3(0.50f, 0.55f, 0.65f);
	railing.shininess = 64.0f;
	railing.tag = "railing";
	m_objectMaterials.push_back(railing);

	// Roof: dark slate, slight glossy
	OBJECT_MATERIAL roof;
	roof.ambientColor = glm::vec3(0.19f, 0.23f, 0.35f);
	roof.ambientStrength = 0.12f;
	roof.diffuseColor = glm::vec3(0.25f, 0.34f, 0.48f);
	roof.specularColor = glm::vec3(0.18f, 0.22f, 0.33f);
	roof.shininess = 36.0f;
	roof.tag = "roof";
	m_objectMaterials.push_back(roof);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->use();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// --- MOONLIGHT
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.30f, -1.0f, -0.10f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.05f, 0.09f, 0.18f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.10f, 0.20f, 0.40f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.25f, 0.33f, 0.50f);

	// --- ** LAMP POST LIGHTS
	// [ LEFT STAIRS ]
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[0].position", -16.0f, 8.0f, 5.2f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.02f, 0.015f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.20f, 0.18f, 0.10f);
	// Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[0].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[0].linear", 0.22f);
	m_pShaderManager->setFloatValue("pointLights[0].quadratic", 0.20f);
	// [ LA BONNE AFFAIRE ]
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[1].position", 17.7f, 6.5f, -1.2f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.02f, 0.015f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.20f, 0.18f, 0.10f);
	//Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[1].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[1].linear", 0.22f);
	m_pShaderManager->setFloatValue("pointLights[1].quadratic", 0.20f);
	// [ HOTEL CHECK-IN ]
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[2].position", 25.7f, 9.5f, 52.2f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.02f, 0.015f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.20f, 0.18f, 0.10f);
	//Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[2].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[2].linear", 0.22f);
	m_pShaderManager->setFloatValue("pointLights[2].quadratic", 0.20f);
	// [ HOTEL ]
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[3].position", -35.7f, 9.5f, 52.2f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.02f, 0.015f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 0.20f, 0.18f, 0.10f);
	//Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[3].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[3].linear", 0.22f);
	m_pShaderManager->setFloatValue("pointLights[3].quadratic", 0.20f);
	// [ WALKWAY ]
	m_pShaderManager->setBoolValue("pointLights[4].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[4].position", 7.7f, 6.5f, -36.2f);
	m_pShaderManager->setVec3Value("pointLights[4].ambient", 0.02f, 0.015f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[4].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[4].specular", 0.20f, 0.18f, 0.10f);
	//Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[4].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[4].linear", 0.22f);
	m_pShaderManager->setFloatValue("pointLights[4].quadratic", 0.20f);
	// [ MOON ]
	m_pShaderManager->setBoolValue("pointLights[5].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[5].position", 60.0f, 100.0f, -80.3f);
	m_pShaderManager->setVec3Value("pointLights[5].ambient", 0.05f, 0.09f, 0.20f);
	m_pShaderManager->setVec3Value("pointLights[5].diffuse", 0.08f, 0.07f, 0.04f);
	m_pShaderManager->setVec3Value("pointLights[5].specular", 0.20f, 0.18f, 0.10f);
	//Attenuation factors
	m_pShaderManager->setFloatValue("pointLights[5].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[5].linear", 0.035f);
	m_pShaderManager->setFloatValue("pointLights[5].quadratic", 0.015f);
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// define the materials for objects in the scene
	DefineObjectMaterials();
	// add and define the light sources for the scene
	SetupSceneLights();
	// load the textures for the 3D scene
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
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- *** GROUND PLANE *** ---
	scaleXYZ = glm::vec3(60.5f, 1.0f, 60.5f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("ground");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawPlaneMesh();

	// --- *** MOON *** ---
	// [ GLOW ]
	scaleXYZ = glm::vec3(23.0f, 23.0f, 1.0f);
	positionXYZ = glm::vec3(60.0f, 100.0f, -80.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.95f, 0.95f, 1.0f, 1.0f);
	m_basicMeshes->DrawSphereMesh();
	// [ MOON ]
	scaleXYZ = glm::vec3(20.0f, 20.0f, 1.0f);
	positionXYZ = glm::vec3(60.0f, 100.0f, -80.3f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.70f, 0.85f, 1.0f, 1.0f);
	SetShaderMaterial("glass");
	m_basicMeshes->DrawSphereMesh();

	// --- *** SIDEWALK AND WALKWAY *** ---
	// [ WALKWAY ]
	// 1
	scaleXYZ = glm::vec3(59.7f, 0.4f, 12.0f);
	positionXYZ = glm::vec3(-30.0f, 0.5f, 15.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("path");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawBoxMesh();
	// 2
	scaleXYZ = glm::vec3(54.7f, 0.4f, 12.0f);
	positionXYZ = glm::vec3(0.0f, 0.5f, -12.5f);
	SetTransformations(scaleXYZ, 0, 90, 0, positionXYZ);
	SetShaderTexture("path");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawBoxMesh();
	// 3
	scaleXYZ = glm::vec3(59.7f, 0.4f, 12.0f);
	positionXYZ = glm::vec3(-30.0f, 0.5f, -40.8f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("path");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawBoxMesh();
	// CYLINDER TO MAKE CURVED SHAPE
	scaleXYZ = glm::vec3(6.3f, 0.4f, 6.3f);
	positionXYZ = glm::vec3(-0.3f, 0.3f, 14.7f);
	SetTransformations(scaleXYZ, 0, 90, 0, positionXYZ);
	SetShaderTexture("path");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawCylinderMesh();
	// 2
	scaleXYZ = glm::vec3(6.3f, 0.4f, 6.3f);
	positionXYZ = glm::vec3(-0.3f, 0.3f, -40.5f);
	SetTransformations(scaleXYZ, 0, 90, 0, positionXYZ);
	SetShaderTexture("path");
	SetShaderMaterial("ground");
	m_basicMeshes->DrawCylinderMesh();

	// --- *** STAIRS *** ---
	// [ LEFT ]
	const int steps = 16;
	const float h = 0.5f, d = 0.8f;
	for (int i = 0; i < steps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = h * 0.5f + i * h;
		float centerZ = 5.2f - i * d;

		// STEP TREAD
		scaleXYZ = glm::vec3(6.0f, h, d);
		positionXYZ = glm::vec3(-23.0f, centerY, centerZ);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ RIGHT ]
	const int rightSteps = 6;
	for (int i = 0; i < rightSteps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = h * 0.5f + i * h;
		float centerZ = -5.2f + i * d;

		// STEP TREAD
		scaleXYZ = glm::vec3(20.0f, h, d);
		positionXYZ = glm::vec3(-2.0f, centerY, centerZ + 30.0f);
		SetTransformations(scaleXYZ, 0, 180.0f, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ BRIDGE ]
	const int bridgeSteps = 4;
	for (int i = 0; i < bridgeSteps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = h * 0.5f + i * h;
		float centerZ = -5.2f + i * d;

		// STEP TREAD
		scaleXYZ = glm::vec3(4.0f, h, d);
		positionXYZ = glm::vec3(-47.0f, centerY + 7.5f, centerZ + 9.5f);
		SetTransformations(scaleXYZ, 0, 180.0f, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ LA BONNE AFFAIRE ]
	const int lbaSteps = 10;
	for (int i = 0; i < lbaSteps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = h * 0.5f + i * h;
		float centerZ = -5.2f + i * d;
		// STEP TREAD
		scaleXYZ = glm::vec3(6.0f, h, d);
		positionXYZ = glm::vec3(22.5f, centerY, centerZ - 16.6f);
		SetTransformations(scaleXYZ, 0, 180.0f, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ BACK ]
	const int backSteps = 11;
	const float backH = 0.8f, backD = 0.5f;
	for (int i = 0; i < backSteps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = backH * 1.3f + i * backH;
		float centerZ = 5.2f + i * backD;

		// STEP TREAD
		scaleXYZ = glm::vec3(d, h, 6.0f);
		positionXYZ = glm::vec3(centerY + 37.9f, centerZ + 0.4f, 3.8f);
		SetTransformations(scaleXYZ, 0, -180.0f, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ HOTEL STAIRS ]
	const int hotelSteps = 8;
	for (int i = 0; i < hotelSteps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = backH * 1.3f + i * backH;
		float centerZ = -5.2f - i * backD;

		// STEP TREAD
		scaleXYZ = glm::vec3(d, h, 10.0f);
		positionXYZ = glm::vec3(centerY - 37.9f, centerZ + 12.0f, 40.8f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// [ HOTEL CHECK-IN STAIRS ]
	const int hotel2Steps = 8;
	for (int i = 0; i < hotel2Steps; ++i)
	{
		// CENTERING THE STAIRS
		float centerY = backH * 1.3f + i * backH;
		float centerZ = -5.2f - i * backD;

		// STEP TREAD
		scaleXYZ = glm::vec3(d, h, 4.0f);
		positionXYZ = glm::vec3(centerY + 40.9f, centerZ + 19.5f, 25.8f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		SetShaderTexture("stairs");
		SetShaderMaterial("stairs");
		m_basicMeshes->DrawCylinderMesh();
	}
	// --- *** STAIR GAP COVERS *** ---
	// [ LEFT ]
	scaleXYZ = glm::vec3(12.0f, 14.0f, 12.0f);
	positionXYZ = glm::vec3(-23.0f, -0.9f, -4.0f);
	SetTransformations(scaleXYZ, -58.0f, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ LA BONNE AFFAIRE ]
	scaleXYZ = glm::vec3(12.0f, 14.0f, 12.0f);
	positionXYZ = glm::vec3(22.6f, -3.6f, -17.2f);
	SetTransformations(scaleXYZ, 58.0f, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ BACK ]
	scaleXYZ = glm::vec3(12.0f, 14.0f, 12.0f);
	positionXYZ = glm::vec3(44.1f, 1.9f, 3.9f);
	SetTransformations(scaleXYZ, 58.0f, 90.0f, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ HOTEL ]
	scaleXYZ = glm::vec3(18.0f, 14.0f, 18.0f);
	positionXYZ = glm::vec3(-37.9f, -0.6f, 40.8f);
	SetTransformations(scaleXYZ, 32.0f, 90.0f, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();

	// --- *** CONNECTING BRIDGE *** ---
	scaleXYZ = glm::vec3(8.0f, 1.0f, 22.6f);
	positionXYZ = glm::vec3(-47.0f, 9.2f, 17.7f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();

	// --- *** FLOORS *** ---
	// [ LEFT STAIRS ]
	// ABOVE
	scaleXYZ = glm::vec3(38.0f, 8.5f, 20.0f);
	positionXYZ = glm::vec3(-36.0f, 4.0f, -16.8f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// NEXT TO
	scaleXYZ = glm::vec3(26.0f, 8.5f, 11.0f);
	positionXYZ = glm::vec3(-42.0f, 4.0f, -1.4f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ RIGHT STAIRS ]
	// ABOVE
	scaleXYZ = glm::vec3(25.0f, 3.5f, 120.6f);
	positionXYZ = glm::vec3(3.3f, 1.42f, 41.5f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ LA BONNE AFFAIRE ]
	// ABOVE
	scaleXYZ = glm::vec3(11.5f, 5.5f, 11.5f);
	positionXYZ = glm::vec3(22.3f, 2.5f, -8.8f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ BACK ]
	// BELOW (CYLINDER)
	scaleXYZ = glm::vec3(7.0f, 5.5f, 7.0f);
	positionXYZ = glm::vec3(34.2f, 0.0f, 2.9f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	// BELOW 
	scaleXYZ = glm::vec3(10.0f, 7.0f, 13.0f);
	positionXYZ = glm::vec3(40.8f, 2.0f, 4.9f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ KIOSKS ]
	// LEFT
	scaleXYZ = glm::vec3(5.0f, 3.0f, 5.0f);
	positionXYZ = glm::vec3(16.0f, 0.26f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	// RIGHT
	scaleXYZ = glm::vec3(5.0f, 3.0f, 5.0f);
	positionXYZ = glm::vec3(-25.0f, 0.26f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	// [ CAFE ]
	scaleXYZ = glm::vec3(8.0f, 1.5f, 14.6f);
	positionXYZ = glm::vec3(34.9f, 14.0f, 25.8f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();
	// [ CAFE 2 ]
	scaleXYZ = glm::vec3(8.0f, 1.5f, 20.0f);
	positionXYZ = glm::vec3(24.0f, 14.0f, 41.3f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("stairs");
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawBoxMesh();

	// --- *** BUILDING WALLS *** ---
	SetShaderTexture("wall");
	SetShaderMaterial("wall");
	// [ LA BONNE AFFAIRE ]
	scaleXYZ = glm::vec3(35.0f, 34.0f, 36.0f);
	positionXYZ = glm::vec3(45.5f, 17.0f, -16.8f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// [ CAFE ]
	scaleXYZ = glm::vec3(30.0f, 11.2f, 6.0f);
	positionXYZ = glm::vec3(50.3f, 5.5f, 14.5f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// BACK
	scaleXYZ = glm::vec3(30.0f, 17.0f, 10.0f);
	positionXYZ = glm::vec3(58.5f, 8.0f, 14.5f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// [ HOTEL CHECK-IN ]
	scaleXYZ = glm::vec3(25.0f, 30.0f, 36.0f);
	positionXYZ = glm::vec3(45.5f, 18.0f, 41.4f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// [ HOTEL ]
	scaleXYZ = glm::vec3(25.0f, 44.0f, 20.0f);
	positionXYZ = glm::vec3(-47.0f, 25.2f, 41.4f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// [ POLICE STATION ] PURPOSELY THIN - JUST A FACADE BUILDING TO TIE THE SCENE TOGETHER
	scaleXYZ = glm::vec3(35.0f, 34.0f, 10.0f);
	positionXYZ = glm::vec3(-55.8f, 17.0f, -10.8f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// --- ROOFS ---
	// [ LA BONNE AFFAIRE ] 
	scaleXYZ = glm::vec3(36.4f, 35.4f, 16.5f);
	positionXYZ = glm::vec3(45.5f, 42.2f, -16.8f);
	SetTransformations(scaleXYZ, -90.0f, 0, 0, positionXYZ);
	SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
	SetShaderMaterial("roof");
	m_basicMeshes->DrawPrismMesh();
	// [ HOTEL CHECK-IN ] 
	scaleXYZ = glm::vec3(36.5f, 25.4f, 10.5f);
	positionXYZ = glm::vec3(45.5f, 38.3f, 41.4f);
	SetTransformations(scaleXYZ, -90.0f, 0, 0, positionXYZ);
	SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
	SetShaderMaterial("roof");
	m_basicMeshes->DrawPrismMesh();
	// [ HOTEL ] 
	scaleXYZ = glm::vec3(20.5f, 25.5f, 10.5f);
	positionXYZ = glm::vec3(-47.0f, 52.4f, 41.4f);
	SetTransformations(scaleXYZ, -90.0f, 0, 0, positionXYZ);
	SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
	SetShaderMaterial("roof");
	m_basicMeshes->DrawPrismMesh();
	// [ POLICE STATION ] 
	scaleXYZ = glm::vec3(10.5f, 35.5f, 6.5f);
	positionXYZ = glm::vec3(-55.8f, 37.3f, -10.8f);
	SetTransformations(scaleXYZ, -90.0f, 0, 0, positionXYZ);
	SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
	SetShaderMaterial("roof");
	m_basicMeshes->DrawPrismMesh();
	// [ CAFE ]
	scaleXYZ = glm::vec3(15.0f, 8.5f, 7.0f);
	positionXYZ = glm::vec3(61.0f, 15.6f, 14.5f);
	SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
	SetShaderColor(0.23f, 0.32f, 0.43f, 1.0f);
	SetShaderMaterial("roof");
	m_basicMeshes->DrawSphereMesh();

	// --- *** KIOSKS *** ---
	// [ LEFT ]
	// BASE
	m_pShaderManager->setIntValue(g_UseTextureName, false);
	scaleXYZ = glm::vec3(4.5f, 9.0f, 4.5f);
	positionXYZ = glm::vec3(16.0f, 1.5f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.28f, 0.40f, 0.60f, 1.0f);
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	//TOP
	SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
	scaleXYZ = glm::vec3(5.5f, 1.0f, 5.5f);
	positionXYZ = glm::vec3(16.0f, 10.0f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawSphereMesh();
	// [ RIGHT ]
	// BASE
	scaleXYZ = glm::vec3(4.5f, 9.0f, 4.5f);
	positionXYZ = glm::vec3(-25.0f, 1.5f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.28f, 0.40f, 0.60f, 1.0f);
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	// TOP
	SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
	scaleXYZ = glm::vec3(5.5f, 1.0f, 5.5f);
	positionXYZ = glm::vec3(-25.0f, 10.0f, 28.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawSphereMesh();
	// [ MIDDLE ] 
	// BASE
	scaleXYZ = glm::vec3(3.5f, 9.0f, 3.5f);
	positionXYZ = glm::vec3(-34.6f, 8.5f, -5.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.28f, 0.40f, 0.60f, 1.0f);
	SetShaderMaterial("stairs");
	m_basicMeshes->DrawCylinderMesh();
	// TOP
	SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
	scaleXYZ = glm::vec3(4.5f, 1.0f, 4.5f);
	positionXYZ = glm::vec3(-34.6f, 18.0f, -5.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawSphereMesh();

	// --- RAILINGS ---
	{
		SetShaderColor(0.28f, 0.40f, 0.60f, 1.0f);
		SetShaderMaterial("railing");
		// [ BRIDGE ]
		// RAIL 1
		scaleXYZ = glm::vec3(0.1f, 1.5f, 22.5f);
		positionXYZ = glm::vec3(-43.2f, 10.4f, 17.7f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 2
		scaleXYZ = glm::vec3(0.1f, 1.5f, 22.5f);
		positionXYZ = glm::vec3(-50.8f, 10.4f, 17.7f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();

		// [ BALCONY ] 
		// RAIL 1
		scaleXYZ = glm::vec3(0.1f, 1.5f, 20.0f);
		positionXYZ = glm::vec3(-17.0f, 9.0f, -16.7f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 2
		scaleXYZ = glm::vec3(0.1f, 1.5f, 14.1f);
		positionXYZ = glm::vec3(-36.1f, 9.0f, 4.1f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 3
		scaleXYZ = glm::vec3(0.1f, 1.5f, 4.1f);
		positionXYZ = glm::vec3(-43.1f, 9.5f, 5.4f);
		SetTransformations(scaleXYZ, -32.0f, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 4
		scaleXYZ = glm::vec3(0.1f, 1.5f, 38.0f);
		positionXYZ = glm::vec3(-35.9f, 9.0f, -26.8f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 5
		scaleXYZ = glm::vec3(0.1f, 1.5f, 14.1f);
		positionXYZ = glm::vec3(-17.0f, 5.5f, -1.0f);
		SetTransformations(scaleXYZ, 32.0f, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();

		// [ OCEAN SIDE ]
		// RAIL 1
		scaleXYZ = glm::vec3(0.1f, 1.5f, 65.5f);
		positionXYZ = glm::vec3(-4.9f, 3.8f, 53.95f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();

		// [ LA BONNE AFFAIRE ]
		// RAIL 1
		scaleXYZ = glm::vec3(0.1f, 1.5f, 11.3f);
		positionXYZ = glm::vec3(16.7f, 6.0f, -8.9f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 2
		scaleXYZ = glm::vec3(0.1f, 1.5f, 11.3f);
		positionXYZ = glm::vec3(22.4f, 6.0f, -3.2f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 3
		scaleXYZ = glm::vec3(0.1f, 1.5f, 11.3f);
		positionXYZ = glm::vec3(16.7f, 3.0f, -19.0f);
		SetTransformations(scaleXYZ, -32.0f, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();

		// [ CAFE ]
		// RAIL 1
		scaleXYZ = glm::vec3(0.1f, 1.5f, 12.0f);
		positionXYZ = glm::vec3(47.4f, 11.8f, 15.7f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 2
		scaleXYZ = glm::vec3(0.1f, 1.5f, 12.0f);
		positionXYZ = glm::vec3(42.5f, 8.7f, 9.8f);
		SetTransformations(scaleXYZ, -32.0f, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 3
		scaleXYZ = glm::vec3(0.1f, 1.5f, 32.0f);
		positionXYZ = glm::vec3(53.6f, 17.3f, 16.7f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 4
		scaleXYZ = glm::vec3(0.1f, 1.5f, 7.0f);
		positionXYZ = glm::vec3(45.2f, 13.8f, 21.8f);
		SetTransformations(scaleXYZ, 32.0f, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 5
		scaleXYZ = glm::vec3(0.1f, 1.5f, 14.5f);
		positionXYZ = glm::vec3(35.3f, 15.5f, 21.8f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 6*
		scaleXYZ = glm::vec3(0.1f, 1.5f, 7.9f);
		positionXYZ = glm::vec3(27.95f, 15.5f, 25.8f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 6
		scaleXYZ = glm::vec3(0.1f, 1.5f, 19.9f);
		positionXYZ = glm::vec3(20.0f, 15.5f, 41.3f);
		SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 7
		scaleXYZ = glm::vec3(0.1f, 1.5f, 7.9f);
		positionXYZ = glm::vec3(24.0f, 15.5f, 51.2f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
		// RAIL 8
		scaleXYZ = glm::vec3(0.1f, 1.5f, 7.9f);
		positionXYZ = glm::vec3(24.0f, 15.5f, 31.4f);
		SetTransformations(scaleXYZ, 0, 90.0f, 0, positionXYZ);
		m_basicMeshes->DrawBoxMesh();
	}

	// --- *** LAMP POSTS *** ---
	// [ LEFT STAIRS ]
	//  POLE
	scaleXYZ = glm::vec3(0.2f, 8.0f, 0.2f);
	positionXYZ = glm::vec3(-16.0f, 0.15f, 5.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawCylinderMesh();
	// BOTTOM OF LAMP
	scaleXYZ = glm::vec3(0.9f, 0.2f, 0.9f);
	positionXYZ = glm::vec3(-16.0f, 6.5f, 5.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// TOP OF LAMP
	scaleXYZ = glm::vec3(1.15f, 0.2f, 1.15f);
	positionXYZ = glm::vec3(-16.0f, 7.9f, 5.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// HEAD
	m_pShaderManager->setBoolValue(g_UseLightingName, false);
	SetShaderColor(1.0f, 0.96f, 0.85f, 1.0f);
	scaleXYZ = glm::vec3(0.8f, 1.5f, 0.8f);
	positionXYZ = glm::vec3(-16.0f, 6.5f, 5.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// [ LA BONNE AFFAIRE ]
	//  POLE
	scaleXYZ = glm::vec3(0.2f, 8.0f, 0.2f);
	positionXYZ = glm::vec3(17.7f, 0.15f, -1.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawCylinderMesh();
	// BOTTOM OF LAMP
	scaleXYZ = glm::vec3(0.9f, 0.2f, 0.9f);
	positionXYZ = glm::vec3(17.7f, 6.5f, -1.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// TOP OF LAMP
	scaleXYZ = glm::vec3(1.15f, 0.2f, 1.15f);
	positionXYZ = glm::vec3(17.7f, 7.9f, -1.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// HEAD
	m_pShaderManager->setBoolValue(g_UseLightingName, false);
	SetShaderColor(1.0f, 0.96f, 0.85f, 1.0f);
	scaleXYZ = glm::vec3(0.8f, 1.5f, 0.8f);
	positionXYZ = glm::vec3(17.7f, 6.5f, -1.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// [ WALKWAY ]
	//  POLE
	scaleXYZ = glm::vec3(0.2f, 8.0f, 0.2f);
	positionXYZ = glm::vec3(7.7f, 0.15f, -36.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawCylinderMesh();
	// BOTTOM OF LAMP
	scaleXYZ = glm::vec3(0.9f, 0.2f, 0.9f);
	positionXYZ = glm::vec3(7.7f, 6.5f, -36.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// TOP OF LAMP
	scaleXYZ = glm::vec3(1.15f, 0.2f, 1.15f);
	positionXYZ = glm::vec3(7.7f, 7.9f, -36.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// HEAD
	m_pShaderManager->setBoolValue(g_UseLightingName, false);
	SetShaderColor(1.0f, 0.96f, 0.85f, 1.0f);
	scaleXYZ = glm::vec3(0.8f, 1.5f, 0.8f);
	positionXYZ = glm::vec3(7.7f, 6.5f, -36.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// [ HOTEL CHECK-IN ]
	//  POLE
	scaleXYZ = glm::vec3(0.2f, 8.0f, 0.2f);
	positionXYZ = glm::vec3(25.7f, 3.15f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawCylinderMesh();
	// BOTTOM OF LAMP
	scaleXYZ = glm::vec3(0.9f, 0.2f, 0.9f);
	positionXYZ = glm::vec3(25.7f, 9.5f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// TOP OF LAMP
	scaleXYZ = glm::vec3(1.15f, 0.2f, 1.15f);
	positionXYZ = glm::vec3(25.7f, 10.9f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// HEAD
	m_pShaderManager->setBoolValue(g_UseLightingName, false);
	SetShaderColor(1.0f, 0.96f, 0.85f, 1.0f);
	scaleXYZ = glm::vec3(0.8f, 1.5f, 0.8f);
	positionXYZ = glm::vec3(25.7f, 9.5f, 52.2);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// [ HOTEL ]
	//  POLE
	scaleXYZ = glm::vec3(0.2f, 8.0f, 0.2f);
	positionXYZ = glm::vec3(-35.7f, 3.15f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawCylinderMesh();
	// BOTTOM OF LAMP
	scaleXYZ = glm::vec3(0.9f, 0.2f, 0.9f);
	positionXYZ = glm::vec3(-35.7f, 9.5f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// TOP OF LAMP
	scaleXYZ = glm::vec3(1.15f, 0.2f, 1.15f);
	positionXYZ = glm::vec3(-35.7f, 10.9f, 52.2f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("railing");
	SetShaderMaterial("railing");
	m_basicMeshes->DrawSphereMesh();
	// HEAD
	m_pShaderManager->setBoolValue(g_UseLightingName, false);
	SetShaderColor(1.0f, 0.96f, 0.85f, 1.0f);
	scaleXYZ = glm::vec3(0.8f, 1.5f, 0.8f);
	positionXYZ = glm::vec3(-35.7f, 9.5f, 52.2);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	m_basicMeshes->DrawCylinderMesh();
	m_pShaderManager->setBoolValue(g_UseLightingName, true);
}
