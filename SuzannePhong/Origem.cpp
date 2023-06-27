#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "stb_image.h"

using namespace std;

struct Vertex {
	float x, y, z;
};

struct Normal {
	float nx, ny, nz;
};

struct TextureCoordinate {
	float u, v;
};

struct FaceVertex {
	int vertexIndex, uvIndex, normalIndex;
};

struct Material {
	string name;
	float Ns;
	float Ka[3];
	float Ks[3];
	float Ke[3];
	float Ni;
	float d;
	int illum;
	string map_Kd;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
Material parseMTL(const string& filename);
int setupGeometry();
int loadTexture(string path);
vector<float> parseObjFile(const string& filename);

const string objFile = "../../3D_Models/Suzanne/SuzanneTriTextured.obj";
const string mtlFile = "../../3D_Models/Suzanne/SuzanneTriTextured.mtl";
const GLuint WIDTH = 1000, HEIGHT = 1000;
bool rotateX,
rotateY,
rotateZ = false;
int verticesQty;

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Iluminacao -- Rafael!", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	Shader shader("../shaders/shader.vs", "../shaders/shader.fs");
	GLuint VAO = setupGeometry();
	glUseProgram(shader.ID);
	glUniform1i(glGetUniformLocation(shader.ID, "tex_buffer"), 0);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view", value_ptr(view));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));
	glm::mat4 model = glm::mat4(1);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	shader.setMat4("model", glm::value_ptr(model));
	Material material = parseMTL(mtlFile);
	GLuint textureId = loadTexture(material.map_Kd);
	shader.setVec3("ka", material.Ka[0], material.Ka[1], material.Ka[2]);
	shader.setVec3("kd", material.Ke[0], material.Ke[1], material.Ke[2]);
	shader.setVec3("ks", material.Ks[0], material.Ks[1], material.Ks[2]);
	shader.setFloat("q", material.Ns);
	shader.setVec3("lightPosition", 15.0f, 15.0f, 2.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLineWidth(10);
		glPointSize(20);
		float angle = (GLfloat)glfwGetTime();
		model = glm::mat4(1);
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		shader.setMat4("model", glm::value_ptr(model));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, verticesQty);
		glBindVertexArray(0);
		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

int setupGeometry()
{
	vector<float> vertices = parseObjFile(objFile);
	verticesQty = vertices.size() / 11;
	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return VAO;
}

vector<float> parseObjFile(const string& filename)
{
	vector<Vertex> vertices;
	vector<Normal> normals;
	vector<TextureCoordinate> textures;
	vector<vector<FaceVertex>> faces;
	ifstream file(filename);
	string line;
	while (getline(file, line))
	{
		cout << line << endl;
		if (line.empty() || line[0] == '#') continue;
		istringstream iss(line);
		string keyword;
		iss >> keyword;
		if (keyword == "v")
		{
			Vertex vertex;
			iss >> vertex.x >> vertex.y >> vertex.z;
			vertices.push_back(vertex);
		}
		else if (keyword == "vn")
		{
			Normal normal;
			iss >> normal.nx >> normal.ny >> normal.nz;
			normals.push_back(normal);
		}
		else if (keyword == "vt")
		{
			TextureCoordinate texture;
			iss >> texture.u >> texture.v;
			textures.push_back(texture);
		}
		else if (keyword == "f")
		{
			vector<FaceVertex> faceVertices;
			string faceVertexStr;
			while (iss >> faceVertexStr)
			{
				istringstream fvIss(faceVertexStr);
				string vertexIndexStr, uvIndexStr, normalIndexStr;
				getline(fvIss, vertexIndexStr, '/');
				getline(fvIss, uvIndexStr, '/');
				getline(fvIss, normalIndexStr, '/');
				int vertexIndex = stoi(vertexIndexStr) - 1;
				int uvIndex = stoi(uvIndexStr) - 1;
				int normalIndex = stoi(normalIndexStr) - 1;
				faceVertices.push_back({ vertexIndex, uvIndex, normalIndex });
			}
			faces.push_back(faceVertices);
		}
	}
	vector<float> vertexArray;
	for (const auto& faceVertices : faces)
	{
		for (const auto& fv : faceVertices)
		{
			const Vertex& vertex = vertices[fv.vertexIndex];
			const TextureCoordinate& texture = textures[fv.uvIndex];
			const Normal& normal = normals[fv.normalIndex];
			vertexArray.push_back(vertex.x);
			vertexArray.push_back(vertex.y);
			vertexArray.push_back(vertex.z);
			vertexArray.push_back(0.0f);// r
			vertexArray.push_back(0.0f);// g
			vertexArray.push_back(0.0f);// b
			vertexArray.push_back(texture.u);
			vertexArray.push_back(texture.v);
			vertexArray.push_back(normal.nx);
			vertexArray.push_back(normal.ny);
			vertexArray.push_back(normal.nz);
		}
	}
	return vertexArray;
}

Material parseMTL(const string& filename)
{
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Failed to open MTL file: " << filename << endl;
		return {};
	}
	Material material;
	string line;
	while (getline(file, line)) {
		istringstream iss(line);
		string token;
		iss >> token;
		if (token == "newmtl") iss >> material.name;
		if (token == "Ns") iss >> material.Ns;
		if (token == "Ka") iss >> material.Ka[0] >> material.Ka[1] >> material.Ka[2];
		if (token == "Ks") iss >> material.Ks[0] >> material.Ks[1] >> material.Ks[2];
		if (token == "Ke") iss >> material.Ke[0] >> material.Ke[1] >> material.Ke[2];
		if (token == "Ni") iss >> material.Ni;
		if (token == "d") iss >> material.d;
		if (token == "illum") iss >> material.illum;
		if (token == "map_Kd") iss >> material.map_Kd;
	}
	return material;
}

int loadTexture(string path)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	bool isPng = nrChannels != 3;
	if (!data) {
		cout << "Failed to load texture" << endl;
		return -1;
	}
	isPng
		? glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data)
		: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}
}
