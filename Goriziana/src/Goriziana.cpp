#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <glad/glad.h>

#include <glfw/glfw3.h>

#ifdef _WINDOWS_
#error windows.h was included!
#endif

#include <utils/shader.h>
#include <utils/camera.h>
#include <utils/model.h>
#include <utils/physics.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <bullet/btBulletDynamicsCommon.h>
#include "BulletDebugDrawer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define NR_LIGHTS 2

using namespace std;

/********** struct CHARACTER **********/
struct Character {
	//Campo utilizzato per memorizzare l'ID della texture del glyph
	GLuint textureID;
	//Campo in cui è salvata la dimensione del glyph
	glm::ivec2 size;
	//Campo in cui è salvato l'offset dalla baseline all'angolo in alto a sinistra
	glm::ivec2 bearing;
	//Campo in cui è salvato l'offset dal punto advance al carattere successivo
	GLuint advance;
};

//Dimensioni della finestra dell'applicazione
const GLuint SCR_WIDTH = 1280, SCR_HEIGHT = 720;

//Camera      posx   posy  posz   upx   upy   upz   yaw   pitch
Camera camera(-8.5f, 7.6f, -2.2f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

//Variabili utilizzate per implementare una Camera FPS
GLfloat lastX = (float) SCR_WIDTH / 2.0f;
GLfloat lastY = (float) SCR_HEIGHT / 2.0f;
GLfloat firstMouse = true;
double mouseX, mouseY;

//Deltatime per uniformare la velocità di movimento
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//Posizione ottimale della luce del sole
glm::vec3 lightDirs[] = { glm::vec3(-6.0f, 10.0f, -9.0f), glm::vec3(10.0f, 10.0f, 10.0f) };

//Pesi della componente diffusive, speculari e ambientali per shaders
//GLfloat Ks = 0.5f;
//GLfloat Ka = 0.2f;
GLfloat Kd = 0.8f;
GLfloat F0[] = { 2.0f, 0.1f };
GLfloat m = 0.3f;

//Dimensione della sfera
glm::vec3 sphereSize = glm::vec3(0.5f, 0.5f, 0.5f);

//Posizione delle biglie del gioco
glm::vec3 poolBallPos[] = {
	glm::vec3(-5.5f, 6.62f, -2.2f), // biglia bianca
	glm::vec3(-5.5f, 6.62f, 2.2f), // biglia gialla
	glm::vec3(5.5f, 6.62f, 0.0f) // biglia rossa
};

glm::vec3 poolPinPos[] = {
	glm::vec3(1.0f, 6.23f, 0.0f), // birillo in alto
	glm::vec3(0.0f, 6.23f, -1.0f), // birillo a sinistra
	glm::vec3(0.0f, 6.23f, 0.0f), // birillo al centro
	glm::vec3(0.0f, 6.23f, 1.0f), // birillo a destra
	glm::vec3(-1.0f, 6.23f, 0.0f), // birillo in basso
};

glm::vec3 poolPlanePos = glm::vec3(0.0f, 0.0f, 0.0f);

//Registra gli eventi che modificano le dimensioni della finestra
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//Funzioni di utility
GLuint load_texture(const char* path);
GLuint load_cubemap(vector<string> faces);
void draw_model_notexture(Shader &shaderNT, Model &ball, btRigidBody* bodyWhite, btRigidBody* bodyRed, btRigidBody* bodyYellow);
void draw_model_texture(Shader &shaderT, Model &table, Model &pin, vector<btRigidBody*> vectorPin);
void draw_skybox(Shader &shaderSB, Model &box, GLuint texture);
bool check_idle_ball(btVector3 linearVelocity);
void create_dictionary(FT_Face face);
void render_text(Shader &shader, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

//Funzioni per gestire il gioco
void throw_ball(btRigidBody* ball);

//Libreria per la simulazione fisica
Physics poolSimulation;
//Classe che eredita tutte le componenti per eseguire il debug della libreria fisica
BulletDebugDrawer debugger;
//Vettore contenente i btRigidBody associati alle biglie dei giocatori
vector<btRigidBody*> playersBall;
//Vettore contenente i btRigidBody associati ai birilli della scena
vector<btRigidBody*> vectorPin;
//Map contenente i caratteri pre-caricati per la scrittura del testo
map<GLchar, Character> dictionary;

glm::vec3 selectedBallPos(0.0f);

glm::mat4 projection(1.0f);
glm::mat4 view(1.0f);
glm::mat4 model(1.0f);
glm::mat3 normal(1.0f);

bool debugMode = true;

bool checkShoot = false;
// Variabile booleana che utilizzo per switchare tra i due giocatori.
// false=0 primo giocatore, biglia bianca.
// true=1 secondo giocatore, biglia gialla
bool player = false;

int main() {
	//INIZIALIZZO GLFW
	if (!glfwInit()) {
		cout << "Errore nell'inizializzazione di GLFW!\n" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//CREO LA FINESTRA

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Goriziana", nullptr, nullptr);

	if (!window) {
		cout << "Errore nella creazione della finestra!" << endl;
		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	//SETTO LE FUNZIONI DI CALLBACK CHE SI OCCUPANO DI GESTIRE LE INTERAZIONI DELL'UTENTE

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//Per avere una maggior capacità di movimento, impostare l'ultimo parametro a GLFW_CURSOR_DISABLED.
	//Per visualizzare il puntatore, impostare l'ultimo parametro a GLFW_CURSOR_HIDDEN
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// GLAD cerca di caricare il contesto impostato da GLFW
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		cout << "Errore nell'inizializzazione del contesto OpenGL!" << endl;
		return -1;
	}

	//SETTO LE OPZIONI DI RENDERING DI OPENGL
	glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//SETTO IL CURSORE AL CENTRO DELLA SCHERMATA
	glfwSetCursorPos(window, (double) (SCR_WIDTH / 2), (double) (SCR_HEIGHT / 2));

	//SETTO LE COMPONENTI PER LA LIBRERIA DI TEXTRENDERING
	FT_Library library;
	if (FT_Init_FreeType(&library))
		cout << "Errore nell'inizializzazione della libreria FreeType!" << endl;

	FT_Face face;
	if (FT_New_Face(library, "font/arial.ttf", 0, &face))
		cout << "Errore nel caricamento del font!" << endl;

	FT_Set_Pixel_Sizes(face, 0, 36);

	//Setto questo valore per accettare anche texture da 1 byte (Di default OpenGL accetta solo texture che abbiano una dimensione multipla di 4 byte).
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//Carico tutti i 128 caratteri in memoria, generando la relativa texture, in modo da avere il dizionario completo a rapido accesso.
	create_dictionary(face);

	//Libero la memoria occupata dalle componenti
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	//VETTORE UTILIZZATO PER CARICARE LA CUBEMAP
	vector<string> faces = { "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg", "skybox/bottom.jpg", "skybox/front.jpg", "skybox/back.jpg" };

	//UTILIZZO LA CLASSE SHADER CREATA PER COMPILARE IL VS ED IL FS, E LINKARLI NEL PS
	Shader shaderNoTexture("shaders/shaderNoTextureCT.vert", "shaders/shaderNoTextureCT.frag");
	Shader shaderTexture("shaders/shaderTextureCT.vert", "shaders/shaderTextureCT.frag");
	Shader shaderDebugger("shaders/shaderDebug.vert", "shaders/shaderDebug.frag");
	Shader shaderSkybox("shaders/shaderSkybox.vert", "shaders/shaderSkybox.frag");
	Shader shaderText("shaders/shaderText.vert", "shaders/shaderText.frag");

	//UTILIZZO LA CLASSE MODEL CREATA PER CARICARE E VISUALIZZARE IL MODELLO 3D
	Model modelTable("models/table/gTable.obj");
	Model modelBall("models/ball/ball.obj");
	Model modelPin("models/pin/scaledPin.obj");
	Model modelSkybox("models/cube/cube.obj");

	//CREO IL CORPO RIGIDO DA ASSEGNARE AL TAVOLO
	glm::vec3 bodyTablePos = glm::vec3(0.0f, 6.02f, 0.0f);
	glm::vec3 bodyTableSize = glm::vec3(12.3f, 0.1f, 5.5f);
	glm::vec3 bodyTableRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	btRigidBody* bodyTable = poolSimulation.createRigidBody(0, bodyTablePos, bodyTableSize, bodyTableRotation, 0.0, 0.6, 0.0);

	//CREO I BORDI DEL TAVOLO
	//LATO LUNGO POSTERIORE
	glm::vec3 bodyTableLSPos = glm::vec3(0.0f, 6.8f, -5.5f);
	glm::vec3 bodyTableLSSize = glm::vec3(12.3f, 1.0f, 0.1f);
	glm::vec3 bodyTableLSRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableLSPos, bodyTableLSSize, bodyTableLSRotation, 0.0, 0.5, 0.7);

	//LATO LUNGO ANTERIORE
	bodyTableLSPos = glm::vec3(0.0f, 6.8f, 5.5f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableLSPos, bodyTableLSSize, bodyTableLSRotation, 0.0, 0.5, 0.7);

	//LATO CORTO SINISTRO
	glm::vec3 bodyTableSSPos = glm::vec3(-12.13f, 6.8f, 0.0f);
	glm::vec3 bodyTableSSSize = glm::vec3(0.1f, 1.0f, 5.5f);
	glm::vec3 bodyTableSSRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableSSPos, bodyTableSSSize, bodyTableSSRotation, 0.0, 0.5, 0.7);

	//LATO CORTO DESTRO
	bodyTableSSPos = glm::vec3(12.2f, 6.8f, 0.0f);

	bodyTable = poolSimulation.createRigidBody(0, bodyTableSSPos, bodyTableSSSize, bodyTableSSRotation, 0.0, 0.5, 0.7);

	//CREO IL CORPO RIGIDO DA ASSEGNARE AI BIRILLI
	// Dimensione rigibody Cylinder per modello birillo
	glm::vec3 bodyPinSize = glm::vec3(0.07f, 0.2f, 0.07f);

	glm::vec3 bodyPinRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 5; i++) {
		btRigidBody* bodyPin = poolSimulation.createRigidBody(2, poolPinPos[i], bodyPinSize, bodyPinRotation, 0.1, 0.4, 0.0);
		vectorPin.push_back(bodyPin);
	}

	//CREO IL CORPO RIGIDO DA ASSEGNARE ALLE BIGLIE
	glm::vec3 bodyBallRadius = sphereSize;
	glm::vec3 bodyBallRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	btRigidBody* bodyBallWhite = poolSimulation.createRigidBody(1, poolBallPos[0], bodyBallRadius, bodyBallRotation, 1.0, 0.7, 0.4);
	btRigidBody* bodyBallYellow = poolSimulation.createRigidBody(1, poolBallPos[1], bodyBallRadius, bodyBallRotation, 1.0, 0.7, 0.4);
	btRigidBody* bodyBallRed = poolSimulation.createRigidBody(1, poolBallPos[2], bodyBallRadius, bodyBallRotation, 1.0, 0.7, 0.4);

	//Lo uso per evitare che la biglia salti
	bodyBallWhite->setLinearFactor(btVector3(1, 0, 1));
	bodyBallYellow->setLinearFactor(btVector3(1, 0, 1));
	bodyBallRed->setLinearFactor(btVector3(1, 0, 1));

	bodyBallWhite->setAngularFactor(0.1);
	bodyBallYellow->setAngularFactor(0.1);
	bodyBallRed->setAngularFactor(1.0);

	//Inserisco le biglie all'interno del vettore per gestire i giocatori
	playersBall.push_back(bodyBallWhite);
	playersBall.push_back(bodyBallYellow);

	//CARICO LE TEXTURE
	//Carico le texture per lo skybox
	GLuint textureSkybox = load_cubemap(faces);

	//INIZIALIZZO LA ORTHOGRAPHIC MATRIX PER IL TEXTRENDERING
	projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	shaderText.Use();
	shaderText.setMat4("projectionMatrix",projection);

	//INIZIALIZZO LA PROJECTION MATRIX
	projection = glm::perspective(45.0f, (float) SCR_WIDTH / (float) SCR_HEIGHT, 1.0f, 10000.0f);

	//CREO LE VARIABILI DI SUPPORTO
	// imposto il delta di tempo massimo per aggiornare la simulazione fisica
	GLfloat maxSecPerFrame = 1.0f / 60.0f;

	selectedBallPos = poolBallPos[0];
	camera.setObjectPos(selectedBallPos);

	btTransform transform;
	btVector3 linearVelocity, origin, temp;

	glm::vec3 position;
	GLfloat playerIndexOffset = 40.0f;
	GLfloat matrix[16];

	int counterPoint[] = {0, 0};

	//AVVIO IL RENDER LOOP
	while (!glfwWindowShouldClose(window)) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.31f, 0.76f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		//INIZIALIZZO LA VIEW MATRIX
		view = camera.GetViewMatrix();

		if (debugMode)
			debugger.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		else
			debugger.setDebugMode(btIDebugDraw::DBG_NoDebug);

		//SETTO LA SIMULAZIONE FISICA ED IL DEBUGGER
		poolSimulation.dynamicsWorld->setDebugDrawer(&debugger);

		debugger.SetMatrices(&shaderDebugger, projection, view, model);
		poolSimulation.dynamicsWorld->debugDrawWorld();

		poolSimulation.dynamicsWorld->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 10);

		//RENDERIZZO GLI OGGETTI DELLA SCENA
		draw_model_notexture(shaderNoTexture, modelBall, bodyBallWhite, bodyBallRed, bodyBallYellow);

		draw_model_texture(shaderTexture, modelTable, modelPin, vectorPin);

		draw_skybox(shaderSkybox, modelSkybox, textureSkybox);

		render_text(shaderText, "*", 15.0f, 670.0f - playerIndexOffset * player, 1.0f, glm::vec3(1.0f));
		render_text(shaderText, "Giocatore 1 | ", 40.0f, 675.0f, 1.0f, glm::vec3(1.0f));
		render_text(shaderText, to_string(counterPoint[0]), 250.0f, 675.0f, 1.0f, glm::vec3(1.0f));
		render_text(shaderText, "Giocatore 2 | ", 40.0f, 635.0f, 1.0f, glm::vec3(1.0f));
		render_text(shaderText, to_string(counterPoint[1]), 250.0f, 635.0f, 1.0f, glm::vec3(1.0f));

		model = mat4(1.0f);

		//GESTISCO IL CAMBIO GIOCATORE
		linearVelocity = playersBall[player]->getLinearVelocity();

		if (check_idle_ball(linearVelocity) && checkShoot) {
			// Non appena la biglia del giocatore si ferma, passo all'altro giocatore, spostando la camera sull'altra biglia

			playersBall[!player]->getMotionState()->getWorldTransform(transform);
			origin = transform.getOrigin();

			position = glm::vec3(origin.getX(), origin.getY(), origin.getZ());

			camera.setObjectPos(position);

			view = camera.MoveCamera(position);

			//Controllo i birilli caduti, per l'assegnamento dei punti, e li riposiziono
			for(int i = 0; i < 5; i++){

				vectorPin[i]->getMotionState()->getWorldTransform(transform);
				transform.getOpenGLMatrix(matrix);

				if (abs(matrix[4]) > abs(matrix[5])){
					cout << "Pin[" << i << "] caduto!" << endl;
					counterPoint[player]++;
				}

				temp.setValue(poolPinPos[i].x, poolPinPos[i].y, poolPinPos[i].z);

				transform.setIdentity();
				transform.setOrigin(temp);

				vectorPin[i]->getMotionState()->setWorldTransform(transform);
				vectorPin[i]->setWorldTransform(transform);
			}

			checkShoot = false;

			player = !player;
		}

		glfwSwapBuffers(window);
	}

	//PULISCO LA MEMORIA
	shaderTexture.Delete();
	shaderNoTexture.Delete();
	shaderDebugger.Delete();
	shaderSkybox.Delete();
	shaderText.Delete();

	poolSimulation.Clear();

	glfwTerminate();

	return 0;
}

//GESTISCO GLI INPUT DA TASTIERA
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	// if ESC is pressed, close the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// if D is pressed, activate/deactivate DrawDebugger from Bullet Physics
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		debugMode = !debugMode;
}

//GESTISCO LA CREAZIONE DELLA FINESTRA
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

//GESTISCO I MOVIMENTI DEL MOUSE
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;

		firstMouse = false;
	}

	GLfloat xOffset = xpos - lastX;

	lastX = xpos;
	lastY = ypos;

	view = camera.RotateAroundPoint(xOffset, glm::vec3(0.0f, 1.0f, 0.0f));

	mouseX = xpos;
	mouseY = ypos;
}

//GESTISCO GLI INPUT DEL MOUSE
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		//cout << "Left button pressed at " << currentFrame << endl;
		throw_ball(playersBall[player]);
	}
}

//FUNZIONE UTILIZZATA PER IL LANCIO DELLA BIGLIA
//Applico un impulso al centro della biglia, calcolando la direzione mediante la posizione del mouse.
void throw_ball(btRigidBody* ball) {
	// Se la palla non è ancora ferma, l'altro giocatore non può tirare.
	if (!checkShoot) {
		glm::mat4 screenToWorld = glm::inverse(projection * view);

		GLfloat shootInitialSpeed = 20.0f;

		GLfloat x = (mouseX / SCR_WIDTH) * 2 - 1,
				y = -(mouseY / SCR_HEIGHT) * 2 + 1;

		btVector3 impulse, relPos;

		glm::vec4 mousePos = glm::vec4(x, y, 1.0f, 1.0f);

		glm::vec4 direction = glm::normalize(screenToWorld * mousePos) * shootInitialSpeed;

		impulse = btVector3(direction.x, direction.y, direction.z);

		relPos = btVector3(1.0, 1.0, 1.0);

		//cout << "Impulse: " << impulse.getX() << " - " << impulse.getY() << " - " << impulse.getZ() << endl;

		ball->activate(true);
		ball->applyImpulse(impulse, relPos);

		checkShoot = true;
	}
}

//CARICO LE TEXTURE
//Carico un'immagine e creo texture OpengGL
GLuint load_texture(char const * path) {
	GLuint textureID;
	GLint width, height, nrComponents;

	glGenTextures(1, &textureID);

	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data) {
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
	} else {
		cout << "Texture failed to load at path: " << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}

//Carico le immagini per formare una Cubemap
GLuint load_cubemap(vector<string> faces) {
	GLuint textureID;
	GLint width, height, nrChannels;

	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (GLuint i = 0; i < faces.size(); i++) {
		unsigned char *face = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (face) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, face);
			stbi_image_free(face);
		} else {
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

//RENDERIZZO GLI OGGETTI DELLA SCENA
//Imposto lo shader e renderizzo i modelli degli oggetti senza texture
void draw_model_notexture(Shader &shaderNT, Model &ball, btRigidBody* bodyWhite, btRigidBody* bodyRed, btRigidBody* bodyYellow) {
	GLfloat matrix[16];
	btTransform transform;

	shaderNT.Use();

	//RENDERIZZO LE BIGLIE DA BILIARDO
	//INIZIO DALLA BIANCA

	//COMPONENTI PER SHADER BLINN-PHONG
//	shaderNT.setVec3("material.ambient", 0.25, 0.20725, 0.20725);
//	shaderNT.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
//	shaderNT.setVec3("material.specular", 0.296648, 0.296648, 0.296648);
//
//	shaderNT.setFloat("material.shininess", 0.088);
//
//	shaderNT.setFloat("Kd", Kd);
//	shaderNT.setFloat("Ka", Ka);
//	shaderNT.setFloat("Ks", Ks);

	//COMPONENTI PER SHADER COOK-TORRANCE
	for (GLuint i = 0; i < NR_LIGHTS; i++) {
		string number = to_string(i);
		shaderNT.setVec3(("lightVectors[" + number + "]").c_str(), lightDirs[i]);
		shaderNT.setFloat(("F0[" + number + "]").c_str(), F0[i]);
	}

	shaderNT.setFloat("m", m);
	shaderNT.setFloat("Kd", Kd);

	shaderNT.setVec3("diffuseColor", 1.0f, 1.0f, 1.0f);

	shaderNT.setMat4("projectionMatrix", projection);

	shaderNT.setMat4("viewMatrix", view);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyWhite->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	normal = glm::inverseTranspose(glm::mat3(view * model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix", normal);

	ball.Draw(shaderNT);

	//RENDERIZZO LA BIGLIA ROSSA
	shaderNT.setVec3("diffuseColor", 1.0f, 0.0f, 0.0f);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyRed->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	// se casto a mat3 una mat4, in automatico estraggo la sottomatrice 3x3 superiore sinistra
	normal = glm::inverseTranspose(glm::mat3(view * model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix", normal);

	ball.Draw(shaderNT);

	//RENDERIZZO LA BIGLIA GIALLA
	shaderNT.setVec3("diffuseColor", 1.0f, 1.0f, 0.0f);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	bodyYellow->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix(matrix);

	model = glm::make_mat4(matrix) * glm::scale(model, sphereSize);
	// se casto a mat3 una mat4, in automatico estraggo la sottomatrice 3x3 superiore sinistra
	normal = glm::inverseTranspose(glm::mat3(view * model));

	shaderNT.setMat4("modelMatrix", model);
	shaderNT.setMat3("normalMatrix", normal);

	ball.Draw(shaderNT);
}

//Imposto lo shader e renderizzo i modelli degli oggetti con texture
void draw_model_texture(Shader &shaderT, Model &table, Model &pin, vector<btRigidBody*> vectorPin) {
	GLfloat matrix[16];
	btTransform transform;

	//RENDERIZZO I MODELLI CON TEXTURE
	//INIZIO DAL TAVOLO

	//COMPONENTI PER SHADER BLINN-PHONG
//	shaderT.setVec3("material.specular", specularColor);
//	shaderT.setFloat("material.shininess", shininess);
//
//	shaderT.setFloat("Kd", Kd);
//	shaderT.setFloat("Ka", Ka);
//	shaderT.setFloat("Ks", Ks);

	//COMPONENTI PER SHADER COOK-TORRANCE
	shaderT.Use();

	for (GLuint i = 0; i < NR_LIGHTS; i++) {
		string number = to_string(i);
		shaderT.setVec3(("lightVectors[" + number + "]").c_str(), lightDirs[i]);
//		shaderT.setFloat(("m[" + number + "]").c_str(), m[i]);
//		shaderT.setFloat(("F0[" + number + "]").c_str(), F0[i]);
	}

	shaderT.setFloat("m", 0.6);
	shaderT.setFloat("F0", 4.0);
	shaderT.setFloat("Kd", 1.0);

	shaderT.setFloat("repeat", 10.0f);

	shaderT.setMat4("projectionMatrix", projection);

	shaderT.setMat4("viewMatrix", view);

	model = glm::mat4(1.0f);
	normal = glm::mat3(1.0f);

	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.15f));
	model = glm::scale(model, glm::vec3(25.0f, 25.0f, 25.0f));
	normal = glm::inverseTranspose(glm::mat3(view * model));

	shaderT.setMat4("modelMatrix", model);
	shaderT.setMat3("normalMatrix", normal);

	table.Draw(shaderT);

	//RENDERIZZO I BIRILLI
	shaderT.setFloat("m", 0.4);
	shaderT.setFloat("F0", 2.0);
	shaderT.setFloat("Kd", 0.7);
	shaderT.setFloat("repeat", 1.0f);

	for (int i = 0; i < 5; i++) {
		model = glm::mat4(1.0f);
		normal = glm::mat3(1.0f);

		vectorPin[i]->getMotionState()->getWorldTransform(transform);
		transform.getOpenGLMatrix(matrix);

		model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
		// Scala per modello cilindro
		//model = glm::make_mat4(matrix) * glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
		// Scala per modello birillo
		model = glm::make_mat4(matrix) * glm::scale(model, glm::vec3(0.027f, 0.027f, 0.027f));
		normal = glm::inverseTranspose(glm::mat3(view * model));

		shaderT.setMat4("modelMatrix", model);
		shaderT.setMat3("normalMatrix", normal);

		pin.Draw(shaderT);
	}
}

//Imposto lo shader e renderizzo la Cubemap
void draw_skybox(Shader &shaderSB, Model &box, GLuint texture) {
	glDepthFunc(GL_LEQUAL);
	shaderSB.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

	shaderSB.setMat4("projectionMatrix", projection);
	shaderSB.setMat4("viewMatrix", view);
	shaderSB.setInt("skyboxTexture", 0);

	box.Draw(shaderSB);

	glDepthFunc(GL_LESS);
}

//FUNZIONE UTILIZZATA PER CONTROLLARE LA SITUAZIONE DI UNA BIGLIA
//Se la linearVelocity assume un valore per cui la biglia è ferma, restituisco true in modo che venga cambiato giocatore/biglia
bool check_idle_ball(btVector3 linearVelocity) {
	if (abs(linearVelocity.getX()) < 0.01 && abs(linearVelocity.getY()) < 0.01 && abs(linearVelocity.getZ()) < 0.01)
		return true;

	return false;
}

//FUNZIONE UTILIZZATA PER CARICARE IN MEMORIA I 128 CARATTERI
void create_dictionary(FT_Face face) {
	//Carico i primi 128 caratteri del codice ASCII
	for (GLubyte c = 0; c < 128; c++) {

		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			cout << "Errore nel caricamento del Glyph!" << endl;
			continue;
		}

		GLuint texture;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,
		GL_RED,
		GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x };
		dictionary.insert(pair<GLchar, Character>(c, character));
	}
}

//FUNZIONE DI TEXTRENDERING
void render_text(Shader &shader, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.Use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	//Itero sui caratteri del testo che voglio scrivere
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = dictionary[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		//Renderizzo il glyph c
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Shift di 6 bit per andare al successivo carattere
		x += (ch.advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
