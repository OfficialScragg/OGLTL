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
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void geom(float y, float x, float z);
static void keyMovement(GLFWwindow* window);

// Structs
struct vertex {
	float position[3];
	float color[4];	
};

// Variables
float WIDTH = 1080.0f;
float HEIGHT = 720.0f;
struct vertex vertices[] = {
		{{0.0f,   2.0f,  -0.5f}, {1.0f, 0.0f, 0.0f, 0.0f}}, // back 
		{{-0.5f,   2.0f,   0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}}, // left
	       	{{0.5f,   2.0f,   0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}}, // right
	       	{{0.0f,   3.0f,  0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}}, // top
		{{100.0f, 0.0f, 100.0f}, {0.2f, 0.8f, 0.2f, 1.0f}},
		{{100.0f, 0.0f, -100.0f}, {0.2f, 0.8f, 0.2f, 1.0f}},
		{{-100.0f, 0.0f, 100.0f}, {0.2f, 0.8f, 0.2f, 1.0f}},
		{{-100.0f, 0.0f, -100.0f}, {0.2f, 0.8f, 0.2f, 1.0f}}
};
unsigned int indices[] = {
	0, 2, 1,
	3, 0, 1,
	3, 2, 0,
	3, 1, 2,
	4, 5, 6,
	6, 5, 7
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
	"	if(fColor != vec4(0.2f, 0.8f, 0.2f, 1.0f)){\n"
    	"		gl_Position = uProjection * uView * (uTransform * vec4(aPos, 1.0f));\n"
	"	}else{\n"
	"		gl_Position = uProjection * uView * vec4(aPos, 1.0f);\n"
	"	}\n"
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
float rot_y = 0.0f;
float rot_x = 0.0f;
float rot_z = 0.0f;
float cam_y = 0.0f;
// Matrices
mat4 transform;
mat4 view;
mat4 projection;
vec3 position;
vec3 scale;
// Camera 
vec3 camPos = {0.0f, 1.0f, 5.0f};
vec3 dir = {0.0f, 0.0f, -1.0f};
vec3 camUp = {0.0f, 1.0f, 0.0f};
float yaw = -90.0f;
float pitch = 90.0f;
float camSpeed = 2.0f;
float deltaSpeed = 0.0f;
// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;
// Mouse
float lastX = 400;
float lastY = 300;
int firstMouse = 1;

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Project", NULL, NULL);
	if(!window){
		// ERR
		printf("ERROR: Window creation failed!\n");
		return -1;
	}
	
	// Make the window OpenGL context current
	glfwMakeContextCurrent(window);
	
	// Load OpenGL API with Glad
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	
	// Setup Keyboard and Mouse Input
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  	
	glfwSetCursorPosCallback(window, mouse_callback);

	// Where shall we draw
	glViewport(0, 0, WIDTH, HEIGHT);

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

	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// While window is open...keep running
	while(!glfwWindowShouldClose(window)){
		// Time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		deltaSpeed = (float)(camSpeed * deltaTime);
		// Running
		glClearColor(100.0f / 255.0f, 180.0f / 255.0f, 255.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		keyMovement(window);
		geom(rot_y, rot_x, rot_z);
		rot_y+=1.2f;
		rot_z+=2.3f;
		rot_x+=0.9f;
		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
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
void geom(float y, float x, float z){
	// Transform parameters
	vec3 pivot = {0.0f, 0.0f, -3.0f};
	vec3 y_axis = {0.0f, 1.0f, 0.0f};
	vec3 x_axis = {1.0f, 0.0f, 0.0f};
	vec3 z_axis = {0.0f, 0.0f, 1.0f};

	// Set matrices to identities
	glm_mat4_identity(transform);
	glm_mat4_identity(view);
	glm_mat4_identity(projection);
	
	// Transform Matrix
	glm_scale_uni(transform, 1.0f);
	glm_rotate_at(transform, pivot, glm_rad(y), y_axis);
	glm_rotate_at(transform, pivot, glm_rad(x), x_axis);
	glm_rotate_at(transform, pivot, glm_rad(z), z_axis);
	
	// View
	glm_look(camPos, dir, camUp, view);

	// Projection Matrix
	glm_perspective(glm_rad(90.0f), (float)(WIDTH/HEIGHT), 0.1f, 200.0f, projection);

	// Upload Uniforms
	glUniformMatrix4fv(0, 1, GL_FALSE, (float *)transform);
	glUniformMatrix4fv(1, 1, GL_FALSE, (float *)view);
	glUniformMatrix4fv(2, 1, GL_FALSE, (float *)projection);
	return;
}

// Keyboard movement
static void keyMovement(GLFWwindow* window){
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		glm_vec3_muladds(dir, deltaSpeed, camPos);
        }

        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		vec3 res = {0.0f, 0.0f, 0.0f};
                glm_vec3_scale(dir, deltaSpeed, res);
                glm_vec3_sub(camPos, res, camPos);
	}
	
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                vec3 res1 = {0.0f, 0.0f, 0.0f};
                vec3 res2 = {0.0f, 0.0f, 0.0f};
                vec3 res3 = {0.0f, 0.0f, 0.0f};
                glm_vec3_cross(dir, camUp, res1);
                glm_vec3_normalize(res1);
                glm_vec3_muladds(res1, deltaSpeed, res2);
                glm_vec3_sub(camPos, res2, res3);
                glm_vec3_copy(res3, camPos);
        }

        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                vec3 res1 = {0.0f, 0.0f, 0.0f};
                vec3 res2 = {0.0f, 0.0f, 0.0f};
                vec3 res3 = {0.0f, 0.0f, 0.0f};
                glm_vec3_cross(dir, camUp, res1);
                glm_vec3_normalize(res1);
                glm_vec3_muladds(res1, deltaSpeed, res2);
                glm_vec3_add(camPos, res2, res3);
                glm_vec3_copy(res3, camPos);
	}
	return;
}

// Get mouse input
static void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	if (firstMouse){
        	lastX = xpos;
        	lastY = ypos;
        	firstMouse = 0;
    	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sense = 0.1f;
	xoffset *= sense;
	yoffset *= sense;

	yaw += xoffset;
	pitch += yoffset;

	// Clamp values
	if(pitch > 89.0f){
		pitch = 89.0f;
	}else if(pitch < -89.0f){
		pitch = -89.0f;
	}
	// Apply direction changes
	dir[0] = (float)(cos(glm_rad(yaw)) * cos(glm_rad(pitch)));
        dir[1] = sin(glm_rad(pitch));
        dir[2] = (float)(sin(glm_rad(yaw)) * cos(glm_rad(pitch)));
	glm_vec3_normalize(dir);

	return;
}

// Get keyboard input
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}else if(key == GLFW_KEY_X && action == GLFW_PRESS){
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
