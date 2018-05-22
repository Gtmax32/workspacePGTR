/*
Classe Camera
- crea sistema di riferimento della camera
- gestisce i movimenti di camera (FPS-style) alla pressione dei tasti WASD e al movimento del mouse

*/
#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include <glad/glad.h>

// GLM viene utilizzata per creare la matrice di vista e per gestire le trasformazioni di camera
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;

// Possibili movimenti della camera
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Impostazioni di default di camera
const float YAW         = -90.0f;
const float PITCH       = -90.0f;
// Parametri per gestione interazione con il mouse
const float SPEED       =  5.0f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

const vec3 OFFSET(-3.0f, 0.0f, 0.0f);
/********** classe CAMERA **********/
class Camera {
public:
    // Attributi della Camera
    vec3 Position;
    vec3 Front;
    // Vettore UP locale della Camera
	vec3 Up;
    vec3 Right;
    // Vettore UP del mondo, serve per il calcolo iniziale del vettore Right
	vec3 WorldUp;
    // Angoli di Eulero
    float Yaw;
    float Pitch;
    // Parametri della Camera
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    vec3 selectedBallPos;

    // Costruttore completo
    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        // Inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
		this->updateCameraVectors();
    }
	
    // Costruttore semplificato
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		
        this->Position = vec3(posX, posY, posZ);
        this->WorldUp = vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        // Inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
		this->updateCameraVectors();
    }

    // Restituisce la view matrix calcolata usando gli angeli di Eulero e la LookAt Matrix
    mat4 GetViewMatrix() {
        return lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    mat4 lookAtObject(){
    	return lookAt(this->selectedBallPos + OFFSET, this->selectedBallPos + this->Front, this->Up);
    }

    // Aggiorna la posizione della camera in base alla pressione dei tasti W,A,S e D
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        
		if (direction == FORWARD)
            this->Position += this->Front * velocity;
        
		if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        
		if (direction == LEFT)
            this->Position -= this->Right * velocity;
        
		if (direction == RIGHT)
            this->Position += this->Right * velocity;
		
		//this->Position.y = 20.0f; // <-- this one-liner keeps the user at the ground level (xz plane)
    }

    // Aggiorna la posizione della camera in base al movimento del mouse
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        // Applica il parametro di sensitivita' per pesare lo spostamento
		xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;
		
		// Aggiorna gli angoli di rotazione su Y e X
        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Applica un vincolo sulla rotazione su X, per evitare di ribaltare la vista
        if (constrainPitch) {
			
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            
			if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Aggiorna i vettori Front, Right e Up utilizzando gli angoli di Eulero appena aggiornati
        this->updateCameraVectors();
    }

    // Processa l'interazione della rotellina del mouse con la scena. Permette di effettuare uno zoom.
    void ProcessMouseScroll(float yoffset) {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        
		if (Zoom <= 1.0f)
            Zoom = 1.0f;
        
		if (Zoom >= 45.0f)
            Zoom = 45.0f;
    }
	
	// Ruota la camera attorno al vettore Up
	void RotateCamera(float deltaTime){
		GLfloat velocity = this->MovementSpeed * deltaTime;

		this->Yaw += velocity;

		this->updateCameraVectors();
	}

	// Ruota la camera attorno ad un punto nello spazio
	mat4 RotateAroundPoint(vec3 objectPoint, GLfloat angle, vec3 axis){
		GLfloat velocity = this->MovementSpeed * angle;

		vec3 oldPosition = this->Position;

		vec3 direction = normalize(this->Position - objectPoint);
		vec3 newDirection = normalize(rotateY(direction, velocity));

		mat4 matrix = translate(mat4(1.0f), -newDirection);
		matrix = rotate(matrix, velocity, axis);
		matrix = translate(matrix, direction);

		this->Position = matrix * vec4(oldPosition, 1.0f);
		//this->Front = -direction;

		//this->Yaw += velocity;

		std::cout << to_string(this->Position) << std::endl;

		return lookAt(objectPoint, objectPoint + this->Front, this->Up);
	}

	void setObjectPos(vec3 objPos) {
		this->selectedBallPos = objPos;
	}

	vec3 getObjectPos() {
		return this->selectedBallPos;
	}

private:
    // Aggiorna il sistema di riferimento della camera
    void updateCameraVectors() {
        // Calcola il nuovo vettore Front, tramite calcoli trigonometrici legati agli angoli di Yaw e Pitch
        vec3 front;
        front.x = cos(radians(this->Yaw)) * cos(radians(this->Pitch));
        front.y = sin(radians(this->Pitch));
        front.z = sin(radians(this->Yaw)) * cos(radians(this->Pitch));
        this->Front = normalize(front);
        // Una volta calcolata la nuova direzione di vista, ricalcolo il vettore Right come prodotto vettoriale tra Front e la UP del mondo
		this->Right = normalize(cross(this->Front, this->WorldUp));
		// Calcolo il vettore UP locale della camera, come prodotto vettoriale tra Front e Right
        this->Up = normalize(cross(this->Right, this->Front));
    }
};
#endif
