#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "CommonValues.h"
#include "Mesh.h"
#include "Shader.h"
#include "GLWindow.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
#include "Model.h"

#include <assimp/Importer.hpp>

const float toRadians = 3.14159265f / 180.0f;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
GLWindow mainWindow;
Camera camera;

Material shinyMaterial;
Material dullMaterial;

Model skull;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

Texture brickTexture;
Texture dirtyTexture;
Texture plainTexture;
Texture brick2Texture;
Texture roadTexture;
//Texture stingTexture1;
//Texture stingTexture2;
//Texture stingTexture3;
//Texture stingTexture4;
//Texture stingTexture5;
//Texture stingTexture6;
//Texture stingTexture7;
//Texture stingTexture8;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

GLfloat rotationAngle = 0.0f;
glm::vec3 skullPosition(-20.0f, 0.0f, 0.0f); // начальна€ позици€ куба
GLfloat skullMovement = 0.005f; // скорость движени€ куба

void FramebufferResize(GLFWwindow* window, float height, float width);

// Vertex Shader
static const char* vShader = "shaders/shader.vert";

// Fragment Shader
static const char* fShader = "shaders/shader.frag";

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};


void calcAverageNormals(unsigned int* indices,
	unsigned int indiceCount,
	GLfloat* vertices,
	unsigned int verticesCount,
	unsigned int vLength,
	unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;

		glm::vec3 v1(vertices[in1] - vertices[in0],
			vertices[in1 + 1] - vertices[in0 + 1],
			vertices[in1 + 2] - vertices[in0 + 2]);

		glm::vec3 v2(vertices[in2] - vertices[in0],
			vertices[in2 + 1] - vertices[in0 + 1],
			vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset;
		in1 += normalOffset;
		in2 += normalOffset;

		vertices[in0] += normal.x;
		vertices[in0 + 1] += normal.y;
		vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x;
		vertices[in1 + 1] += normal.y;
		vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x;
		vertices[in2 + 1] += normal.y;
		vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticesCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x;
		vertices[nOffset + 1] = vec.y;
		vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,  // Side
		1, 3, 2,  // Side
		2, 3, 0,  // Front
		0, 1, 2 }; // base

	GLfloat vertices[] = {
		//   x    y     z    u      v   nx    ny    nz
		-1.0f, -1.0f, -0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom Left
		0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // Bottom Back (Middle)
		1.0f, -1.0f, -0.6f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Bottom Right
		0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f };   // Top

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, // back left
		10.0f, 0.0f, -10.0f, 10.0f, 0.0f, 0.0f, -1.0f, 0.0f, // back right
		-10.0f, 0.0f, 10.0f, 0.0f, 10.0f, 0.0f, -1.0f, 0.0f, // front left
		10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.0f, 0.0f, // front right
	};

	unsigned int floorIndices[] = {
		0, 2, 1, // tri 1
		1, 2, 3	 // tri 2
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

// Create a cube
void CreateCube()
{
	unsigned int indices[] = {
		0, 1, 2, // Front - tstartUpop triangle
		1, 2, 3, // Front
		4, 5, 6, // Back - top triangle
		5, 6, 7, // Back
		1, 5, 3, // Right Side
		3, 5, 7, // Right Side Back
		0, 4, 6, // Left
		0, 6, 2, // Left
		0, 4, 5, // startUp
		0, 1, 5, // Top
		2, 6, 7, // Bottom
		2, 3, 7	 // Bottom
	};

	GLfloat vertices[] = {
		//  x    y     z    u      v   nx    ny    nz
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // 0 Left Top Front
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,	  // 1 Right Top Front
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 2 Left Bottom Front
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 3 Right Bottom Front
		-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // 4 Left Top Back
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,	  // 5 Right Top Back
		-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 6 Left Bottom Back
		1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }; // 7 Right Bottom Back*/

	Mesh* cube1 = new Mesh();
	cube1->CreateMesh(vertices, indices, 64, 36);
	meshList.push_back(cube1);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	Shader* skybox = new Shader();
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = GLWindow(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateCube();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f,
		5.0f,
		0.5f);

	brickTexture = Texture((char*)"textures/bricks_001.png");
	brickTexture.LoadTextureA();

	dirtyTexture = Texture((char*)"textures/gxblue.jpg");
	dirtyTexture.LoadTextureA();

	roadTexture = Texture((char*)"textures/road.jpg");
	roadTexture.LoadTexture();

	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	skull = Model();
	skull.LoadModel("models/CraneoOBJ.obj");

	brick2Texture = Texture((char*)"textures/wall.jpg");
	brick2Texture.LoadTexture();

	dirtyTexture.UseTexture();




	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,	 // colour and
		0.3f, 0.6f,		 // 0.3, 0.4 Ambient intensity and Intensity of light for diffuse
		0.0f, 0.0f, -1.0f); // Position of light

	unsigned int pointLightCount = 0;

	pointLights[0] = PointLight(0.0f, 0.0f, 1.0f,
		0.3f, 0.1f,
		4.0f, 0.0f, 0.0f,
		0.3f, 0.2f, 0.1f);

	pointLightCount++;

	pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
		0.3f, 0.1f,
		-4.0f, 2.0f, 0.0f,
		0.3f, 0.1f, 0.1f);

	pointLightCount++;

	unsigned int spotLightCount = 0;

	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);

	spotLightCount++;

	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);

	spotLightCount++;

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0;

	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string facesCubemap[6] = {
		"texture/skybox.jpg",
		"texture/skybox.jpg",
		"texture/skybox.jpg",
		"texture/skybox.jpg",
		"texture/skybox.jpg",
		"texture/skybox.jpg"
	};
	// Creates the cubemap texture object
	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}


	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

	// wireframe
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	bool isFlashActive = false;


	// main loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;	
		lastTime = currentTime;

		//clear the windowglm::value_ptr(projection)
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		shaderList[0].UseShader();

		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;

		if (mainWindow.getsKeys()[GLFW_KEY_SPACE]) {
			isFlashActive = !isFlashActive; // ѕереключение состо€ни€ при каждом нажатии пробела
		}

		if (isFlashActive) {
			spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());
		}


		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		glUniformMatrix4fv(shaderList[0].GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(shaderList[0].GetViewLocation(), 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(shaderList[0].GetEyePositionLocation(), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// Triangle 1
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -2.5f));
		glUniformMatrix4fv(uniformModel = shaderList[0].GetModelLocation(), 1, GL_FALSE, glm::value_ptr(model));
		brickTexture.UseTexture();
		shinyMaterial.UseMaterial(shaderList[0].GetSpecularIntensityLocation(), shaderList[0].GetShininess());
		meshList[0]->RenderMesh();

		// Triangle 2
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(3.0f, 0.0f, -2.5f));
		model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtyTexture.UseTexture();
		dullMaterial.UseMaterial(shaderList[0].GetSpecularIntensityLocation(), shaderList[0].GetShininess());
		meshList[1]->RenderMesh();

		//Floor
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, -2.5f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		roadTexture.UseTexture();
		shinyMaterial.UseMaterial(shaderList[0].GetSpecularIntensityLocation(), shaderList[0].GetShininess());
		meshList[2]->RenderMesh();

		//skull
		model = glm::mat4(1.0f);
		skullPosition.y += skullMovement + deltaTime;
		if (skullPosition.y < -1.0f) {
			skullPosition.y = -1.0f;
			skullMovement *= -1.0f;
		}
		else if (skullPosition.y > 2.0f) {
			skullPosition.y = 2.0f;
			skullMovement *= -1.0f;
		}
		model = glm::translate(model, skullPosition);
		//model = glm::rotate(model, 60 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(shaderList[0].GetSpecularIntensityLocation(), shaderList[0].GetShininess());
		skull.RenderModel();

		rotationAngle += 0.01f;


		glUseProgram(0);
		//skybox.render();
		mainWindow.swapBuffers();

		glfwPollEvents();
	}

	return 0;
}
