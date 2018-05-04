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

/********** classe CAMERA **********/
class Camera {
public:
    // Attributi della Camera
    glm::vec3 Position;
    glm::vec3 Front;
    // Vettore UP locale della Camera
	glm::vec3 Up;
    glm::vec3 Right;
    // Vettore UP del mondo, serve per il calcolo iniziale del vettore Right
	glm::vec3 WorldUp;
    // Angoli di Eulero
    float Yaw;
    float Pitch;
    // Parametri della Camera
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Costruttore completo
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        // Inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
		this->updateCameraVectors();
    }
	
    // Costruttore semplificato
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        // Inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
		this->updateCameraVectors();
    }

    // Restituisce la view matrix calcolata usando gli angeli di Eulero e la LookAt Matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
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
        // Applica il parametro di sensitività per pesare lo spostamento
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
	
	// Ruota la camera per ottenere una visuale dall'alto
	void RotateCamera(){
		
	}

private:
    // Aggiorna il sistema di riferimento della camera
    void updateCameraVectors() {
        // Calcola il nuovo vettore Front, tramite calcoli trigonometrici legati agli angoli di Yaw e Pitch
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        // Una volta calcolata la nuova direzione di vista, ricalcolo il vettore Right come prodotto vettoriale tra Front e la UP del mondo
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
		// Calcolo il vettore UP locale della camera, come prodotto vettoriale tra Front e Right
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
#endif
