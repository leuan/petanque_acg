// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include "dependente\glew\glew.h"

// Include GLFW
#include "dependente\glfw\glfw3.h"

// Include GLM
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"

// Include sphere
#include "dependente/sphere/Sphere.h"

#include "shader.hpp"

// Include Camera
#include "Camera/camera.h"

// variables
GLFWwindow* window;
const int width = 1024, height = 768;
bool gameStarted = false;

// lighting
glm::vec3 lightPos(-3.0f, 0.0f, 3.0f);
glm::vec3 lightColor(1.0f);

//Time
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//Add the camera parameters here and create a camera object
glm::vec3 cameraRedPos = glm::vec3(0.0f, 100.0f, 400.0f);
glm::vec3 cameraRedDir = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraRedUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraBluePos = glm::vec3(0.0f, 100.0f, -400.0f);
glm::vec3 cameraBlueDir = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraBlueUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera cam(cameraRedPos, cameraRedDir, cameraRedUp);


//sphere variables
glm::vec3 spherePos(0.0, 0.0, 0.0);
int sectorCount = 72; //72
int stackCount = 36; //36
float radius = 0.5f;
Sphere sphere(radius, sectorCount, stackCount, false);

//jack variables
glm::vec4 jackColor(1.0f, 1.0f, 0.0f, 1.0f);
glm::vec3 jackPos(0.0f);

//animation variables
float dist;
float animDur = 1;
float initialDist;
int animatedBallIndex = -1;
bool inAnimation = false;
float animDirection = 1;
float animAngle;
float currentAngle;
float frameAngle;

//board variables
glm::vec4 boardColor(0.2706f, 0.1569f, 0.102f, 1.0f);
glm::vec3 boardPos(0.0f, -0.5f, 11.0f);
glm::vec3 boardSize(6.5f, 0.1f, 22.0f);

//ball variables
#define TEAM_BLUE 0
#define TEAM_RED 1
glm::vec4 teamRedColor(1.0f, 0.0f, 0.0f, 1.0f);
glm::vec4 teamBlueColor(0.0f, 0.0f, 1.0f, 1.0f);
glm::vec3 initialPos(0.0f, -6.0f, 0.0f);

struct ball{
	glm::mat4 model;
	glm::vec3 pos = initialPos;
	glm::vec4 color ;
	float distFromJack;
	int team;
	bool inHand = false;
	bool wasThrown = false;
};

struct ball balls[6];

int teamBlueBallCount = 0;
int teamRedBallCount = 0;
int turn = TEAM_RED;

void showScore();

void switchCamera();

void placeJack() {
	std::srand(time(0)); //reinitialize random number generator seed
	jackPos.x = (float)(std::rand() % 1000) / 100 - 5;
	jackPos.z = (float)(std::rand() % 1800) / 100 - 9;
	jackPos.y = 0.0f;
	std::cout << "Jack position: " << jackPos.x << ", " << jackPos.z << "\n";
}

void initGame() {
	//initialize ball array
	for (int i = 0; i < 6; ++i) {
		balls[i].pos = initialPos;
		balls[i].model = glm::mat4(1.0f);
		balls[i].inHand = false;
		balls[i].wasThrown = false;
		teamBlueBallCount = 0;
		teamRedBallCount = 0;
		turn = TEAM_RED;
		animatedBallIndex = -1;
		animDirection = 1;
		inAnimation = false;
		if (i < 3) { //first 3 balls are team red and the next are blue
			balls[i].team = TEAM_RED;
			balls[i].color = teamRedColor;
		}
		else {
			balls[i].team = TEAM_BLUE;
			balls[i].color = teamBlueColor;
		}
	}
	turn = TEAM_RED;
	teamBlueBallCount = 0;
	teamRedBallCount = 0;
	gameStarted = true;
	std::cout << "Game initialized\n";
}

void showNextBall() {
	if (!gameStarted) {
		return;
	}
	if ((teamBlueBallCount > 2 && turn == TEAM_BLUE) || (teamRedBallCount > 2 && turn == TEAM_RED)) {
		//todo: end game
		showScore();
		return;
	}
	if (turn == TEAM_RED) {
		switchCamera();
		balls[teamRedBallCount].pos.z = 10.0f;
		balls[teamRedBallCount].pos.y = 0.0f;
		balls[teamRedBallCount].inHand = true;
		std::cout << "Red ball in hand!\n";
	}
	else {
		switchCamera();
		balls[3 + teamBlueBallCount].pos.z = -10.0f;
		balls[3 + teamBlueBallCount].pos.y = 0.0f;
		balls[3 + teamBlueBallCount].inHand = true;
		std::cout << "Blue ball in hand!\n";
	}
}

void moveLeft(float speed) {
	if (turn == TEAM_RED) {
		if (!balls[teamRedBallCount].inHand || balls[teamRedBallCount].pos.x < -6) {
			return;
		}
		balls[teamRedBallCount].pos.x = balls[teamRedBallCount].pos.x - (speed * deltaTime * 5);
	}
	else {
		if (!balls[3 + teamBlueBallCount].inHand || !balls[3 + teamBlueBallCount].pos.x > 6) {
			return;
		}
		balls[3 + teamBlueBallCount].pos.x = balls[3 + teamBlueBallCount].pos.x + speed * (deltaTime * 5);
	}
}

void moveRight(float speed) {
	if (turn == TEAM_RED) {
		if (!balls[teamRedBallCount].inHand || balls[teamRedBallCount].pos.x > 6) {
			return;
		}
		balls[teamRedBallCount].pos.x = balls[teamRedBallCount].pos.x + speed * (deltaTime * 5);
	}
	else {
		if (!balls[3 + teamBlueBallCount].inHand || balls[3 + teamBlueBallCount].pos.x < -6) {
			return;
		}
		balls[3 + teamBlueBallCount].pos.x = balls[3 + teamBlueBallCount].pos.x - speed * (deltaTime * 5);
	}
}

void throwBall() {
	if (turn == TEAM_RED) {
		if (!balls[teamRedBallCount].inHand) {
			return;
		}
		animatedBallIndex = teamRedBallCount;
		animDirection = 1;
		teamRedBallCount++;
		turn = TEAM_BLUE;
		animAngle = 180;
	}
	else {
		if (!balls[3 + teamBlueBallCount].inHand) {
			return;
		}
		animatedBallIndex = 3 + teamBlueBallCount;
		animDirection = -1;
		teamBlueBallCount++;
		turn = TEAM_RED;
		animAngle = -180;
	}
	dist = (float)(std::rand() % 2000) / 100;
	initialDist = dist;
	inAnimation = true;
	balls[animatedBallIndex].wasThrown = true;
	currentAngle = 0;

}

void showScore() {
	int scoreArr[6];
	for (int i = 0; i < 6; i++) {
		scoreArr[i] = i; //initialize array
		//calculate distance
		balls[i].distFromJack = glm::sqrt(glm::pow(balls[i].pos.x - jackPos.x, 2.0f) + glm::pow(balls[i].pos.z - jackPos.z, 2.0f));
	}
	//sort array by distFromJack
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5 - i; j++) {
			if (balls[scoreArr[j]].distFromJack > balls[scoreArr[j + 1]].distFromJack) {
				int aux = scoreArr[j];
				scoreArr[j] = scoreArr[j + 1];
				scoreArr[j + 1] = aux;
			}
		}
	}
	int blueCount = 0, redCount = 0;
	for (int i = 0; i < 3; i++) {
		if (scoreArr[i] > 2) {
			++blueCount;
		}
		else {
			++redCount;
		}
	}
	
	//for (int i = 0; i < 6; i++) {
	//	std::cout << scoreArr[i] << " ";

	//}
	//std::cout << "\n";


	if (blueCount > redCount) {
		std::cout << "Blue Team Wins!\n";
	}
	else {
		std::cout << "Red Team Wins!\n";
	}

}

void switchCamera() {
	if (turn == TEAM_RED) {
		cam.setCameraPosition(cameraRedPos);
		cam.setCameraViewDirection(cameraRedDir);
		cam.setCameraUp(cameraRedUp);
	}
	else {
		cam.setCameraPosition(cameraBluePos);
		cam.setCameraViewDirection(cameraBlueDir);
		cam.setCameraUp(cameraBlueUp);
	}
}


void window_callback(GLFWwindow* window, int new_width, int new_height)
{
	glViewport(0, 0, new_width, new_height);
}

void processInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		moveLeft(1);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		moveRight(1);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		initGame();
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		showNextBall();
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		throwBall();
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "Petanque", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	//specify the size of the rendering window
	glViewport(0, 0, width, height);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Create and compile our GLSL program from the shaders
	// More about this in the following labs
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
	GLuint programID2 = LoadShaders("LightVertexShader.vertexshader", "LightFragmentShader.fragmentshader");

	//vertices and indices used for board
	float verticesCube[] = {
		-1.0f, -1.0f, 0.0f,  // bottom left
		 1.0f, -1.0f, 0.0f,  // bottom right
		-1.0f,  1.0f, 0.0f,  // top left 
		 1.0f,  1.0f, 0.0f,  // top right

		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f
	};

	unsigned int indicesCube[] = {  
		0, 1, 2,  
		1, 3, 2, 
		2, 3, 7,
		2, 7, 6,
		1, 7, 3,
		1, 5, 7,
		6, 7, 4,
		7, 5, 4,
		0, 4, 1,
		1, 4, 5,
		2, 6, 4,
		0, 2, 4
	};

	//Uncomment this to make the rotation animation visible
	//also change stack count and sector count to something low like 10;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	// A Vertex Array Object (VAO) is an object which contains one or more Vertex Buffer Objects and is designed to store 
	// the information for a complete rendered object.
	GLuint	vboSphere, vaoSphere, iboSphere,
		vboBoard, vaoBoard, iboBoard;

	//Bindings for sphere

	glGenVertexArrays(1, &vaoSphere);
	glGenBuffers(1, &vboSphere);
	glGenBuffers(1, &iboSphere);

	glBindVertexArray(vaoSphere);

	glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
	glBufferData(GL_ARRAY_BUFFER, sphere.getVertexSize(), sphere.getVertices(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboSphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.getIndexSize(), sphere.getIndices(), GL_STATIC_DRAW);

	//set attribute pointers
	glVertexAttribPointer(
		0,                  // attribute 0, must match the layout in the shader.
		3,                  // size of each attribute
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		3 * sizeof(float),                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	//Unbinding VAO
	glBindVertexArray(0);

	//Bindings for board
	glGenVertexArrays(1, &vaoBoard);
	glGenBuffers(1, &vboBoard);
	glGenBuffers(1, &iboBoard);

	glBindVertexArray(vaoBoard);

	glBindBuffer(GL_ARRAY_BUFFER, vboBoard);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBoard);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube), indicesCube, GL_STATIC_DRAW);

	//set attribute pointers
	glVertexAttribPointer(
		0,                  // attribute 0, must match the layout in the shader.
		3,                  // size of each attribute
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		3 * sizeof(float),                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	//Unbinding VAO
	glBindVertexArray(0);


	glfwSetFramebufferSizeCallback(window, window_callback);

	//Change accordingly for camera
	glm::mat4 view;
	//view = glm::lookAt(cam.getCameraPosition(), cam.getCameraViewDirection(), cam.getCameraUp());

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(80.0f), (float)width / (float)height, 0.1f, 10000.0f);

	//set callback
	glfwSetKeyCallback(window, key_callback);

	placeJack();

	switchCamera();

	// Check if the window was closed
	while (!glfwWindowShouldClose(window))
	{

		//update camera
		processInput(window);
		view = glm::lookAt(cam.getCameraPosition(), cam.getCameraViewDirection(), cam.getCameraUp());

		//calculate delta time 
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame; 

		// Swap buffers
		glfwSwapBuffers(window);

		// Check for events
		glfwPollEvents();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		//BOARD START

		glm::mat4 modelBoard = glm::mat4(1.0f);
		modelBoard = glm::translate(modelBoard, boardPos);
		modelBoard = glm::scale(modelBoard, boardSize);

		//calculate MVP matrix
		glm::mat4 matrixBoard = projection * view * modelBoard;

		unsigned int transformLoc3 = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc3, 1, GL_FALSE, glm::value_ptr(matrixBoard));

		unsigned int transformLoc4 = glGetUniformLocation(programID, "color");
		glUniform4fv(transformLoc4, 1, glm::value_ptr(boardColor));

		//!!! bind the appropriate VAO before drawing
		glBindVertexArray(vaoBoard);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//BOARD END
		
		//JACK START

		glm::mat4 modelSphere(1.0f);
		modelSphere = glm::translate(modelSphere, jackPos);

		//calculate MVP matrix
		glm::mat4 matrixSphere = projection * view * modelSphere;

		transformLoc3 = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc3, 1, GL_FALSE, glm::value_ptr(matrixSphere));

		transformLoc4 = glGetUniformLocation(programID, "color");
		glUniform4fv(transformLoc4, 1, glm::value_ptr(jackColor));

		//!!! bind the appropriate VAO before drawing
		glBindVertexArray(vaoSphere);

		glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, 0);
		//JACK END

		//BALL START
		if (dist <= 0) {
			balls[animatedBallIndex].inHand = false;
			inAnimation = false;
			animatedBallIndex = -1;
		}
		if (inAnimation) {
			float frameDist = initialDist * deltaTime / animDur;
			balls[animatedBallIndex].pos.z = balls[animatedBallIndex].pos.z - frameDist * animDirection;
			dist -= frameDist;
		}

		for (int i = 0; i < 6; i++) {
			balls[i].model = glm::mat4(1.0f);
			balls[i].model = glm::translate(balls[i].model, balls[i].pos);
			if (inAnimation && i == animatedBallIndex) {
				frameAngle = animAngle * deltaTime / animDur;
				currentAngle += frameAngle;
				balls[animatedBallIndex].model = glm::rotate(balls[animatedBallIndex].model, currentAngle, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			
			//calculate MVP matrix
			glm::mat4 matrixBall = projection * view * balls[i].model;

			unsigned int transformLocBall = glGetUniformLocation(programID, "transform");
			glUniformMatrix4fv(transformLocBall, 1, GL_FALSE, glm::value_ptr(matrixBall));

			unsigned int transformLocColor = glGetUniformLocation(programID, "color");
			glUniform4fv(transformLocColor, 1, glm::value_ptr(balls[i].color));

			//!!! bind the appropriate VAO before drawing
			glBindVertexArray(vaoSphere);

			glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, 0);
		}
		//BALL END
		
	}

	// Cleanup VBO
	glDeleteBuffers(1, &vboSphere);
	glDeleteBuffers(1, &vboBoard);
	glDeleteBuffers(1, &iboSphere);
	glDeleteBuffers(1, &iboBoard);
	glDeleteVertexArrays(1, &vaoSphere);
	glDeleteVertexArrays(1, &vaoBoard);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}