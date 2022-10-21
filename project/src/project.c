#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cglm/cglm.h>

// Forward declarations
void err_callback(int error, const char* desc);
static void key_callback(GLFWwindow* windows, int key, int scancode, int action, int mods);
void geom();

// Structs
struct vertex {
	float position[3];
	float color[4];	
};

// Variables
struct vertex vertices[] = {
		{{0.0f,   0.5f,  -3.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}, 
		{{0.5f,   -0.5f,   -3.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
	       	{{-0.5f,   -0.5f,   -3.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
	       	{{0.0f,   -0.5f,  -3.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
};
unsigned int indices[] = {
	0, 1, 2,
	2, 3, 1
};
unsigned int VAO;
unsigned int EBO;
int success;
int wireframe = 0;
char infoLog[512];
unsigned int vertexShader;
const char* vertexShaderSrc =
	"#version 460 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec4 aCol;\n"
	"layout (location = 0) uniform mat4 uTransform;\n"
	"layout (location = 1) uniform mat4 uView;\n"
	"layout (location = 2) uniform mat4 uProjection;\n"
    	"out vec4 fColor;\n"
	"void main(){\n"
	"	fColor = aCol;\n"
    	"	gl_Position = uProjection * uView * (uTransform * vec4(aPos, 1.0f));\n"
    	"//	gl_Position = vec4(aPos, 1.0f);\n"
	"}";
unsigned int fragShader;
const char* fragShaderSrc =
	"#version 460 core\n"
	"in vec4 fColor;\n"
	"out vec4 FragColor;\n"
	"void main(){\n"
	"	FragColor = fColor;\n"
	"}";
unsigned int shaderProgram;
float rotation = 0.0f;
mat4 transform;
mat4 view;
mat4 projection;
vec3 position;
vec3 scale;
vec3 camPos;
vec3 dir;

// Main function
int main(int argc, char* argv[]){
	// Init GLFW library
	if(!glfwInit()){
		// ERR
		printf("ERROR: Initialising GLFW failed!\n");
		return -1;
	}
	glfwSetErrorCallback(err_callback);
	
	// Create a window!
	GLFWwindow* window = glfwCreateWindow(640, 480, "Project", NULL, NULL);
	if(!window){
		// ERR
		printf("ERROR: Window creation failed!\n");
		return -1;
	}
	
	// Make the window OpenGL context current
	glfwMakeContextCurrent(window);
	
	// Load OpenGL API with Glad
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	
	// Set Keyboard input callback
	glfwSetKeyCallback(window, key_callback);
	
	// Where shall we draw
	glViewport(0, 0, 640, 480);

	// Compile Vertex Shaders
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const char * const*) &vertexShaderSrc, NULL);
	glCompileShader(vertexShader);

	// Check if compilation was successful
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);	
	if(!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR (Vertex Shader): %s\n", infoLog);
		return -1;
	}

	// Compile Frag Shader
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, (const char * const*) &fragShaderSrc, NULL);
        glCompileShader(fragShader);

        // Check if compilation was successful
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if(!success){
                glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
                printf("ERROR (Fragment Shader): %s\n", infoLog);
                return -1;
        }

	// Create Shader Program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	// Check if the program was created successfully
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}

	// Initialise Buffers
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Linking Vertex Attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, color));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	// Bind Vertex Array
        glBindVertexArray(VAO);

	// Use program and Delete Shaders after Linking
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);

	// While window is open...keep running
	while(!glfwWindowShouldClose(window)){
		// Running
		glClearColor(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		geom();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		GLenum err;
      		if((err = glGetError()) != GL_NO_ERROR){
         		printf("OpenGL error: %d\n", err);
      		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}	

	// Terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();	
	return 0;
}

// Mathy stuffs
void geom(){
	glm_mat4_identity(transform);
	glm_mat4_identity(view);
	glm_mat4_identity(projection);

	//vec3 rotate = {0.0f, -0.5f, -2.0f};

	//View
	//glm_translate(view, rotate);
	
	// Projection Matrix
	//glm_perspective_default((float)(640.0f/480.0f), projection);
	glm_perspective(glm_rad(90.0f), (float)(640.0f/480.0f), 0.1f, 100.0f, projection);

	// Upload Uniforms
	glUniformMatrix4fv(0, 1, GL_FALSE, (float *)transform);
	glUniformMatrix4fv(1, 1, GL_FALSE, (float *)view);
	glUniformMatrix4fv(2, 1, GL_FALSE, (float *)projection);

	return;
}

// Get keyboard input
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}else if(key == GLFW_KEY_W && action == GLFW_PRESS){
		if(wireframe){
			wireframe = 0;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}else{
			wireframe = 1;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
	return;
}

// Error callback function
void err_callback(int error, const char* desc){
	fprintf(stderr, "ERROR: %s\n", desc);
	return;
}
