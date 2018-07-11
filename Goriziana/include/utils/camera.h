/*
Classe Camera
- Crea sistema di riferimento della camera
- Gestisce i movimenti di camera al movimento del mouse
- Permette la rotazione intorno ad un oggetto fornendo una camera in terza persona
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace glm;

// Impostazioni di default di camera
// Angolo di Eulero che indica la rotazione attorno all'asse y
const float YAW         = -90.0f;
// Angolo di Eulero che indica la rotazione attorno all'asse x
const float PITCH       = -90.0f;
// Parametri per gestione interazione con il mouse
const float SPEED       =  5.0f;
const float SENSITIVITY =  0.01f;
const float ZOOM        =  45.0f;

/********** classe CAMERA **********/
class Camera {
public:
	// Attributo che indica la posizione dell'oggetto Camera
    vec3 Position;
    // Attributo che rappresenta il vettore verso cui è rivolto l'oggetto Camera
    vec3 Front;
    // Attributo che rappresenta il vettore Up locale dell'oggetto Camera
	vec3 Up;
	// Attributo che rappresenta il vettore Right dell'oggetto Camera
	vec3 Right;
	// Attributo che rappresenta il vettore Up del mondo dell'oggetto Camera, utilizzato per calcolare il vettore Right inizialmente
	vec3 WorldUp;
    // Attributo che rappresenta l'angolo di Eulero di rotazione intorno all'asse y
    float Yaw;
    // Attributo che rappresenta l'angolo di Eulero di rotazione intorno all'asse x
    float Pitch;
    // Attributo che rappresenta la sensibilità del movimento del mouse all'interno della scena
    float MouseSensitivity;
    // Attributo utilizzato per memorizzare la posizione della biglia da osservare
    vec3 selectedBallPos;

    /*
     * Costruttore
     * E' possibile indicare tutti i parametri desiderati o altrimenti lasciare quelli di default.
     */
    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY) {
		
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;

		this->updateCameraVectors();
    }
	
    /* Costruttore
     * E' necessario specificare tutti i parametri della camera.
     */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(vec3(0.0f, 0.0f, -1.0f)), MouseSensitivity(SENSITIVITY) {
		
        this->Position = vec3(posX, posY, posZ);
        this->WorldUp = vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;

		this->updateCameraVectors();
    }

    /*
     * Metodo che calcola la view matrix usando gli angeli di Eulero ed il metodo lookAt della libreria glm.
     * Restituisce la view matrix cosi calcolata.
     */
    mat4 GetViewMatrix() {
        return lookAt(this->Position, this->Position + this->Front, this->Up);
    }

	/*
	 * Metodo utilizzato per ruotare la camera attorno ad un punto nello spazio
	 * Prende in input i seguenti valori:
	 * - angle: GLfloat, angolo di rotazione della camera. Ad ogni frame, se il mouse è in movimento, ruotera' di questo valore.
	 * - axis: glm::vec3, asse intorno a cui deve ruotare la camera
	 * Restituisce la view matrix calcolata in base alla rotazione appena effettuata.
	 */
	mat4 RotateAroundPoint(GLfloat angle, vec3 axis){
		GLfloat velocity = this->MouseSensitivity * (-angle);

		vec3 direction = this->selectedBallPos - vec3(0.0f);

		mat4 matrix = translate(mat4(1.0f), direction);
		matrix = rotate(matrix, velocity, axis);
		matrix = translate(matrix, -direction);

		this->Position = matrix * vec4(this->Position, 1.0f);
		this->Front = matrix * vec4(this->Front, 0.0f);

		return lookAt(this->Position, this->Position + this->Front, this->Up);
	}

	/*
	 * Metodo set dell'attributo selectedBallPos.
	 */
	void setObjectPos(vec3 objPos) {
		this->selectedBallPos = objPos;
	}

	/*
	 * Metodo get dell'attributo selectedBallPos.
	 */
	vec3 getObjectPos() {
		return this->selectedBallPos;
	}

	/*
	 * Metodo utilizzato per spostare la camera nel momento si cambia giocatore.
	 * Restituisce la view matrix calcolata in base allo spostamento effettuato.
	 */
	mat4 MoveCamera(vec3 newPosition){
		float distance = 3.0f;

		vec3 hipotenuse = newPosition - this->Position;
		vec3 direction = hipotenuse - this->Front * distance;
		direction.y += 1.0f;

		mat4 matrix = translate(mat4(1.0f), direction);

		this->Position = matrix * vec4(this->Position, 1.0f);

		return lookAt(this->Position, this->Position + this->Front, this->Up);
	}

private:
    /*
     * Metodo che aggiorna il sistema di riferimento della camera, partendo dalle componenti attuali.
     */
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
