#include <iostream>
#include <fstream>
#include <string>
#include <vector>
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW
#include <glfw/glfw3.h>

// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
#error windows.h was included!
#endif

#include <utils/shader_v2.h>
#include <utils/camera_v3.h>
#include <utils/model_v3.h>
#include <utils/physics_v1.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Libreria per il caricamento delle immagini
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// Libreria per la simulazione fisica
#include <bullet/btBulletDynamicsCommon.h>
#include "BulletDebugDrawer.h"

#define NR_LIGHTS 3

using namespace std;

// Dimensioni della finestra dell'applicazione
const GLuint SCR_WIDTH = 1280, SCR_HEIGHT = 720;

// Camera     posx   posy  posz   upx   upy   upz   yaw   pitch
Camera camera(-8.5f, 7.6f, -2.2f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
//Camera camera(-8.0f, 9.0f, -2.2f, 6.0f, 1.0f, 0.0f);
const GLfloat ROTATION_ANGLE = 5.0f;

// Variabili utilizzate per implementare una Camera FPS
GLfloat lastX = (float)SCR_WIDTH / 2.0f;
GLfloat lastY = (float)SCR_HEIGHT / 2.0f;
GLfloat firstMouse = true;
double mouseX, mouseY;

// Deltatime per uniformare la velocità di movimento
GLfloat currentFrame = 0.0f;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// posizioni delle mie pointlight
glm::vec3 lightPositions[] = {
		glm::vec3(-5.0f, 15.0f, 0.0f),
		glm::vec3(0.0f, 15.0f, 0.0f),
		glm::vec3(5.0f, 15.0f, 0.0f),
};

// Uniform da passare agli shader
glm::vec3 specularColor(1.0f, 1.0f, 1.0f);
glm::vec3 ambientColor(0.1f, 0.1f, 0.1f);
// pesi della componente diffusive, speculari e ambientali
GLfloat Kd = 0.8f;
GLfloat Ks = 0.5f;
GLfloat Ka = 0.1f;
// componente di shininess per shader Phong e Blinn-Phong
GLfloat shininess = 25.0f;

// parametri per l'attenuazione negli shader Phong e Blinn-Phong
GLfloat constant = 1.0f;
GLfloat linear = 0.09f;
GLfloat quadratic = 0.032f;

//Dimensione della sfera
glm::vec3 sphereSize = glm::vec3(0.5f, 0.5f, 0.5f);

//Posizione delle biglie del gioco
glm::vec3 poolBallPos[] = {
		glm::vec3(-5.5f, 7.8f, -2.2f), // biglia bianca
		glm::vec3(5.5f, 7.8f, 0.0f), // biglia rossa
		glm::vec3(-5.5f, 7.8f, 2.2f) // biglia gialla
};

glm::vec3 poolPlanePos = glm::vec3(0.0f, 0.0f, 0.0f);

// Registra gli eventi che modificano le dimensioni della finestra
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Funzioni di utility
GLuint loadTexture(const char* path);
GLuint loadCubemap(vector<string> faces);
void draw_model_notexture(Shader &shaderNT, Model &ball, btRigidBody* bodyWhite, btRigidBody* bodyRed, btRigidBody* bodyYellow);
void draw_model_texture(Shader &shaderT, Model &plane, btRigidBody* bodyPlane, GLuint texture, Model &table, Model &pin);
void draw_skybox(Shader &shaderSB, Model &box, GLuint texture);
bool checkIdleBall(btVector3 linearVelocity);

// Funzioni per gestire il gioco
void throw_ball(btRigidBody* ball);

Physics poolSimulation;
BulletDebugDrawer debugger;
btRigidBody* bodyBallWhite;

glm::vec3 selectedBallPos(0.0f);

glm::mat4 projection(1.0f);
glm::mat4 view(1.0f);
glm::mat4 model(1.0f);
glm::mat3 normal(1.0f);

bool checkShoot = false;

int main(){
	//INIZIALIZZO GLFW
	if (! glfwInit()){
		cout << "Errore nell'inizializzazione di GLFW!\n" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//CREO LA FINESTRA

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Goriziana",nullptr,nullptr);

	if (!window){
		cout <<"Errore nella creazione della finestra!" << endl;
		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	//SETTO LE FUNZIONI DI CALLBACK CHE SI OCCUPANO DI GESTIRE LE INTERAZIONI DELL'UTENTE

	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	//Per avere una maggior capacità di movimento, impostare l'ultimo parametro a GLFW_CURSOR_DISABLED.
	//Per visualizzare il puntatore, impostare l'ultimo parametro a GLFW_CURSOR_HIDDEN
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// GLAD cerca di caricare il contesto impostato da GLFW
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		cout << "Errore nell'inizializzazione del contesto OpenGL!" << endl;
		return -1;
	}

	//SETTO IL DEPTH TEST
	glEnable(GL_DEPTH_TEST);

	//SETTO IL CURSORE AL CENTRO DELLA SCHERMATA
	glfwSetCursorPos(window, (double)(SCR_WIDTH/2), (double)(SCR_HEIGHT/2));

	//VETTORE UTILIZZATO PER CARICARE LA CUBEMAP
	vector<string> faces = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};

	//UTILIZZO LA CLASSE SHADER CREATA PER COMPILARE IL VS ED IL FS, E LINKARLI NEL PS
	Shader shaderNoTexture("shaders/shaderNoTexture.vert", "shaders/shaderNoTexture.frag");
	Shader shaderTexture("shaders/shaderTexture.vert","shaders/shaderTexture.frag");
	Shader shaderDebugger("shaders/shaderDebug.vert", "shaders/shaderDebug.frag");
	Shader shaderSkybox("shaders/shaderSkybox.vert", "shaders/shaderSkybox.frag");

	//UTILIZZO LA CLASSE MODEL CREATA PER CARICARE E VISUALIZZARE IL MODELLO 3D
	//Model modelRoom("../../table/resource/Room.obj");
	Model modelTable("models/table/pooltable.obj");
	Model modelBall("models/ball/ball.obj");
	Model modelPin("models/pin/High_Poly.obj");
	Model modelPlane("models/plane/plane.obj");
	Model modelSkybox("models/cube/cube.obj");

	//CREO IL CORPO RIGIDO DA ASSEGNARE AL PIANO
	glm::vec3 bodyPlaneSize = glm::vec3(10.0f, 0.1f, 10.0f);
	glm::vec3 bodyPlaneRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	btRigidBody* bodyPlane = poolSimulation.createRigidBody(0, poolPlanePos, bodyPlaneSize, bodyPlaneRotation, 0.0, 0.8, 0.3);

	//CREO IL CORPO RIGIDO DA ASSEGNARE AL TAVOLO
	glm::vec3 bodyTablePos = glm::vec3(0.0f, 6.48f, 0.0f);
	glm::vec3 bodyTableSize = glm::vec3(12.0f, 0.1f, 5.2f);
	glm::vec3 bodyTableRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	btRigidBody* bodyTable = poolSimulation.createRigidBody(0, bodyTablePos, bodyTableSize, bodyTableRotation, 0.0, 0.3, 0.3);

	//CREO I BORDI DEL TAVOLO
	//LATO LUNGO POSTERIORE
	glm::vec3 bodyTableLSPos = glm::vec3(0.0f, 6.8f, -5.1f);
	glm::vec3 bodyTableLSSize = glm::vec3(12.0f, 1.0f, 0.1f);
	glm::vec3 bodyTableLSRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableLSPos, bodyTableLSSize, bodyTableLSRotation, 0.0, 0.3, 0.3);

	//LATO LUNGO ANTERIORE
	bodyTableLSPos = glm::vec3(0.0f, 6.8f, 5.1f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableLSPos, bodyTableLSSize, bodyTableLSRotation, 0.0, 0.3, 0.3);

	//LATO CORTO SINISTRO
	glm::vec3 bodyTableSSPos = glm::vec3(-11.7f, 6.8f, 0.0f);
	glm::vec3 bodyTableSSSize = glm::vec3(0.1f, 1.0f, 5.2f);
	glm::vec3 bodyTableSSRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableSSPos, bodyTableSSSize, bodyTableSSRotation, 0.0, 0.3, 0.3);

	//LATO CORTO DESTRO
	bodyTableSSPos = glm::vec3(11.8f, 6.8f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableSSPos, bodyTableSSSize, bodyTableSSRotation, 0.0, 0.3, 0.3);

	//CREO IL CORPO RIGIDO DA ASSEGNARE ALLE BIGLIE
	glm::vec3 bodyBallRadius = sphereSize;
	glm::vec3 bodyBallRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bodyBallWhite = poolSimulation.createRigidBody(1, poolBallPos[0], bodyBallRadius, bodyBallRotation, 1.0, 0.3, 0.3);

	btRigidBody* bodyBallRed = poolSimulation.createRigidBody(1, poolBallPos[1], bodyBallRadius, bodyBallRotation, 1.0, 0.3, 0.3);
	btRigidBody* bodyBallYellow = poolSimulation.createRigidBody(1, poolBallPos[2], bodyBallRadius, bodyBallRotation, 1.0, 0.3, 0.3);

	//Carico la texture per il pavimento
	GLuint textureFloor = loadTexture("textures/floor.jpg");

	//Carico le texture per lo skybox
	GLuint textureSkybox = loadCubemap(faces);

	// imposto il delta di tempo massimo per aggiornare la simulazione fisica
	GLfloat maxSecPerFrame = 1.0f / 60.0f;

	debugger.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	poolSimulation.dynamicsWorld->setDebugDrawer(&debugger);

//  Codice per la versione camera.h
//	camera.setPerspective(45.0f,(float)SCR_WIDTH/(float)SCR_HEIGHT, 1.0f, 10000.0f);
//	projection = camera.getProjectionMatrix();

	// Codice per la versione camera_v3.h
	projection = glm::perspective(45.0f, (float)SCR_WIDTH/(float)SCR_HEIGHT, 1.0f, 10000.0f);

	selectedBallPos = poolBallPos[0];
	camera.setObjectPos(selectedBallPos);

	// File di log inserito per debuggare la biglia
	//ofstream out("log.txt");

	//cout << "LinearSleepThreshold: " << bodyBallWhite->getLinearSleepingThreshold() << "\nAngularSleepThreshold: " << bodyBallWhite->getAngularSleepingThreshold() << endl;

	btTransform transform;
	btVector3 linearVelocity, angularVelocity, origin;
	glm::vec3 position;

	//AVVIO IL RENDER LOOP
	while(!glfwWindowShouldClose(window)){
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.31f, 0.76f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		view = camera.GetViewMatrix();

		debugger.SetMatrices(&shaderDebugger, projection, view, model);
		poolSimulation.dynamicsWorld->debugDrawWorld();

		poolSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 10);

		draw_model_notexture(shaderNoTexture, modelBall, bodyBallWhite, bodyBallRed, bodyBallYellow);

		draw_model_texture(shaderTexture, modelPlane, bodyPlane, textureFloor, modelTable, modelPin);

		draw_skybox(shaderSkybox, modelSkybox, textureSkybox);

		model = mat4(1.0f);

		linearVelocity = bodyBallWhite->getLinearVelocity();
		//angularVelocity = bodyBallWhite->getAngularVelocity();

//		out << currentFrame << endl;
//		out << "LinearVelocity:\n( " << linearVelocity.getX() << ", " << linearVelocity.getY() << ", " << linearVelocity.getZ() << " )\n" << endl;
//		out << "AngularVelocity:\n( " << angularVelocity.getX() << ", " << angularVelocity.getY() << ", " << angularVelocity.getZ() << " )\n" << endl;

		if(checkIdleBall(linearVelocity) && checkShoot){
			bodyBallWhite->getMotionState()->getWorldTransform(transform);
			origin = transform.getOrigin();

			position = glm::vec3(origin.getX(), origin.getY(), origin.getZ());

			camera.MoveCamera(position);
			//view = camera.RotateAroundPoint(position, -180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			camera.setObjectPos(position);

			checkShoot = false;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	shaderTexture.Delete();
	shaderNoTexture.Delete();
	shaderDebugger.Delete();
	shaderSkybox.Delete();

	poolSimulation.Clear();

	// Chiudo il file di log
	//out.close();

	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

//	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		camera.ProcessKeyboard(FORWARD, deltaTime);
//
//	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		camera.ProcessKeyboard(BACKWARD, deltaTime);
//
//	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		view = camera.RotateAroundPoint(selectedBallPos, -ROTATION_ANGLE, glm::vec3(0.0f, 1.0f, 0.0f));
//
//	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		view = camera.RotateAroundPoint(selectedBallPos, ROTATION_ANGLE, glm::vec3(0.0f, 1.0f, 0.0f));
//
//	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
//		throw_ball(bodyBallWhite);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0,0,width,height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	if(firstMouse){
		lastX = xpos;
		lastY = ypos;

		firstMouse = false;
	}

	GLfloat xOffset = xpos - lastX;
	//GLfloat yOffset = lastY - ypos; // Inverto la sottrazione per l'asse è negativo in questo caso

	lastX = xpos;
	lastY = ypos;

	view = camera.RotateAroundPoint(selectedBallPos, xOffset, glm::vec3(0.0f, 1.0f, 0.0f));

	//camera.ProcessMouseMovement(xOffset, yOffset);

	mouseX = xpos;
	mouseY = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		cout << "Left button pressed at " << currentFrame << endl;
		throw_ball(bodyBallWhite);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    	cout << "Right button pressed!" << endl;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	//camera.ProcessMouseScroll(yoffset);
}

void throw_ball(btRigidBody* ball){
	glm::mat4 screenToWorld = glm::inverse(projection * view);

	GLfloat shootInitialSpeed = 1.0f;

	GLfloat x = (mouseX / SCR_WIDTH) * 2 - 1,
			y = -(mouseY / SCR_HEIGHT) * 2 + 1;

	btVector3 impulse, origin;
	btTransform transform;

	glm::vec4 ballPos;

	//ball->getMotionState()->getWorldTransform(transform);

	//origin = transform.getOrigin();

	//ballPos = glm::vec4(origin.getX(), origin.getY(), origin.getZ(), 1.0f);

	glm::vec4 mousePos = glm::vec4(x, y, 1.0f, 1.0f);

	glm::vec4 direction = glm::normalize(screenToWorld * mousePos);
	direction *= shootInitialSpeed;

	//glm::vec3 direction = glm::normalize(mousePos - ballPos) * shootInitialSpeed;

	impulse = btVector3(direction.x, direction.y, direction.z);

	cout << "Impulse: " << impulse.getX() << " - " << impulse.getY() << " - " << impulse.getZ() << endl;
	//Lo uso per evitare che la biglia salti
	ball->setLinearFactor(btVector3(1, 0, 1));

	ball->applyCentralImpulse(impulse);

	//cout << "\nX: " << x << " - Y: " << y << "\nBall X: " << ballPos.x << " - Y: " << ballPos.y << " - Z: " << ballPos.z << endl;

	checkShoot = true;
}

// Carico un'immagine e creo texture OpengGL
GLuint loadTexture(char const * path){
	GLuint textureID;
	GLint width, height, nrComponents;

	glGenTextures(1, &textureID);

	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data){
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else{
		cout << "Texture failed to load at path: " << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}

// Carico le immagini per formare una Cubemap
GLuint loadCubemap(vector<string> faces){
	GLuint textureID;
	GLint width, height, nrChannels;

	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (GLuint i = 0; i < faces.size(); i++){
        unsigned char *face = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (face){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, face);
            stbi_image_free(face);
        }
        else {
            cout << "Cubemap texture failed to load at path: " << faces[i] << endl;
            stbi_image_free(face);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

// Imposto lo shader e renderizzo i modelli degli oggetti senza texture
void draw_model_notexture(Shader &shaderNT, Model &ball, btRigidBody* bodyWhite, btRigidBody* bodyRed, btRigidBody* bodyYellow){
//	glm::mat4 model(1.0f);
//	glm::mat4 normal(1.0f);
//	glm::mat4 whitePos(1.0f);

	GLfloat matrix[16];
	btTransform transform;

	shaderNT.Use();

	//RENDERIZZO LE BIGLIE DA BILIARDO
	//INIZIO DALLA BIANCA
	shaderNT.setVec3("diffuseColor", 1.0f, 1.0f, 1.0f);
	shaderNT.setVec3("ambientColor", ambientColor);
	shaderNT.setVec3("specularColor", specularColor);

	// Per ogni luce nello shaderNT, passo la posizione corrispondente
	for(GLuint i = 0; i < NR_LIGHTS; i++){
		string number = to_string(i);
		shaderNT.setVec3(("lights[" + number + "]").c_str(), lightPositions[i]);
	}

	shaderNT.setFloat("Kd",Kd);
	shaderNT.setFloat("Ka", Ka);
	shaderNT.setFloat("Ks", Ks);

	shaderNT.setFloat("constant",constant);
	shaderNT.setFloat("linear", linear);
	shaderNT.setFloat("quadratic", quadratic);
	shaderNT.setFloat("shininess", shininess);

	shaderNT.setMat4("projectionMatrix", projection);

	shaderNT.setMat4("viewMatrix", view);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyWhite->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

//	model = glm::translate(model, poolBallPos[0]);
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	normal = glm::inverseTranspose(glm::mat3(view*model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix",normal);

	ball.Draw(shaderNT);

	//RENDERIZZO LA BIGLIA ROSSA
	shaderNT.setVec3("diffuseColor", 1.0f, 0.0f, 0.0f);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyRed->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

//	model = glm::translate(model, poolBallPos[1]);
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	// se casto a mat3 una mat4, in automatico estraggo la sottomatrice 3x3 superiore sinistra
	normal = glm::inverseTranspose(glm::mat3(view*model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix",normal);

	ball.Draw(shaderNT);

	//RENDERIZZO LA BIGLIA GIALLA
	shaderNT.setVec3("diffuseColor", 1.0f, 1.0f, 0.0f);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyYellow->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

//	model = glm::translate(model, poolBallPos[2]);
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	// se casto a mat3 una mat4, in automatico estraggo la sottomatrice 3x3 superiore sinistra
	normal = glm::inverseTranspose(glm::mat3(view*model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix",normal);

	ball.Draw(shaderNT);
}

// Imposto lo shader e renderizzo i modelli degli oggetti con texture
void draw_model_texture(Shader &shaderT, Model &plane, btRigidBody* bodyPlane, GLuint texture, Model &table, Model &pin){
//	glm::mat4 model(1.0f);
//	glm::mat4 normal(1.0f);

	//RENDERIZZO IL TAVOLO
	shaderT.Use();

	shaderT.setVec3("ambientColor", ambientColor);
	shaderT.setVec3("specularColor", specularColor);

	// Per ogni luce nello shaderT, passo la posizione corrispondente
	for(GLuint i = 0; i < NR_LIGHTS; i++){
		string number = to_string(i);
		shaderT.setVec3(("lights[" + number + "]").c_str(), lightPositions[i]);
	}

	shaderT.setFloat("Kd",Kd);
	shaderT.setFloat("Ka", Ka);
	shaderT.setFloat("Ks", Ks);

	shaderT.setFloat("constant",constant);
	shaderT.setFloat("linear", linear);
	shaderT.setFloat("quadratic", quadratic);
	shaderT.setFloat("shininess", shininess);

	shaderT.setFloat("repeat", 1.0f);

	shaderT.setMat4("projectionMatrix", projection);

	shaderT.setMat4("viewMatrix", view);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	model = glm::translate(model, glm::vec3(0.0f, 0.1f, -0.15f));
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 25.0f));
	normal = glm::inverseTranspose(glm::mat3(view*model));

	shaderT.setMat4("modelMatrix", model);
	shaderT.setMat3("normalMatrix",normal);

	table.Draw(shaderT);

	//RENDERIZZO I BIRILLI
	model = glm::mat4();
	normal = glm::mat4();

	model = glm::translate(model, glm::vec3(0.0f, 11.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	normal = glm::inverseTranspose(glm::mat3(view*model));

	shaderT.setMat4("modelMatrix", model);
	shaderT.setMat3("normalMatrix",normal);

	pin.Draw(shaderT);

	//RENDERIZZO IL PIANO
//	bodyPlane->getMotionState()->getWorldTransform(transform);
//	transform.getOpenGLMatrix(matrix);
//
//	shaderT.setInt("tex",0);
//	shaderT.setFloat("repeat",2.0f);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, texture);
//
//	model = glm::mat4(1.0f);
//	normal = glm::mat4(1.0f);
//
//	//model = glm::translate(model, poolPlanePos);
//	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//
//	model = glm::scale(model, glm::vec3(500.0f, 500.0f, 1.0f));
//	normal = glm::inverseTranspose(glm::mat3(view*model));
//
//	shaderT.setMat4("modelMatrix", model);//model
//	shaderT.setMat3("normalMatrix",normal);
//
//	plane.Draw(shaderT);
}

// Imposto lo shader e renderizzo la Cubemap
void draw_skybox(Shader &shaderSB, Model &box, GLuint texture){
	glDepthFunc(GL_LEQUAL);
	shaderSB.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

	shaderSB.setMat4("projectionMatrix",projection);
	shaderSB.setMat4("viewMatrix",view);
	shaderSB.setInt("skyboxTexture",0);

	box.Draw(shaderSB);

	glDepthFunc(GL_LESS);
}

bool checkIdleBall(btVector3 linearVelocity){
	if(abs(linearVelocity.getX()) < 0.01 && abs(linearVelocity.getY()) < 0.01 && abs(linearVelocity.getZ()) < 0.01)
		return true;

	return false;
}


