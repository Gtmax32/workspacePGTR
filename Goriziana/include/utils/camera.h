/*
Classe Camera
- crea sistema di riferimento della camera
- gestisce i movimenti di camera (FPS-style) alla pressione dei tasti WASD e al movimento del mouse

NB) adattamento di https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/camera.h

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano
*/

#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <glad/glad.h>
// utilizzerò GLM per creare la matrice di vista e per gestire le trasformazioni di camera
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Possibili movimenti di camera
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Impostazioni di default di camera
// Orientamento iniziale della camera su Y e X (su Z non viene considerato)
const GLfloat YAW        = -90.0f; //Y
const GLfloat PITCH      =  0.0f; //X

// Parametri per gestione movimento del mouse
const GLfloat SPEED      =  3.0f;
const GLfloat SENSITIVTY =  0.25f;

/////////////////// classe CAMERA ///////////////////////
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 WorldFront;
    glm::vec3 Up; // vettore UP locale della camera
    glm::vec3 Right;
    glm::vec3 WorldUp; // vettore UP del mondo -> serve per il calcolo iniziale del vettore Right
    // definisce se la camera rimane "attaccata" al suolo o se segue strettamente la direzione Front corrente
    // (anche se il risultato comporta "volare" nella scena)
    // NB) la versione nella sua forma attuale funziona solo per terreni piani.
    GLboolean onGround;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;

    //////////////////////////////////////////   
    // Costruttore completo
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
    GLfloat yaw = YAW, GLfloat pitch = PITCH, GLboolean onGround = GL_TRUE) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->onGround = onGround;
        //inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
        this->updateCameraVectors();
    }

    // Costruttore semplificato
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), GLboolean onGround = GL_TRUE) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY)
    {
        this->Position = position;
        this->WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->Yaw = YAW;
        this->Pitch = PITCH;
        this->onGround = onGround;
        //inizializza il sistema di riferimento della camera sulla base dei vettori passati al costruttore
        this->updateCameraVectors();
    }
    

    //////////////////////////////////////////
    // Ritorna la matrice di vista corrente
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    //////////////////////////////////////////
    // Sulla base dell'input di tastiera, aggiorna la posizione della camera
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == FORWARD)
            this->Position += (this->onGround ? this->WorldFront : this->Front) * velocity;
        if (direction == BACKWARD)
            this->Position -= (this->onGround ? this->WorldFront : this->Front) * velocity;
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
    }

    //////////////////////////////////////////
    // Sulla base del movimento del mouse, aggiorna l'orientamento della camera
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        // applica il parametro di sensitività per pesare lo spostamento
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        // aggiorna gli angoli di rotazione su Y e X
        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Applica un constraint sulla rotazione su X, per evitare di ribaltare la vista
        // NB) in caso si avessero tutte e 3 le rotazioni, questo constraint permette di evitare gimbal lock
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        //aggiorna il sistema di riferimento della camera sulla base dei nuovi valori di rotazione della camera
        this->updateCameraVectors();
    }

   

private:
    //////////////////////////////////////////
    // Aggiorna il sistema di riferimento della camera
    void updateCameraVectors()
    {
        // Calcola il nuovo vettore Front, tramite calcoli trigonometrici legati agli angoli di Yaw e Pitch 
        // https://learnopengl.com/#!Getting-started/Camera
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->WorldFront = this->Front = glm::normalize(front);
        // il vettore per la direzione Front nel caso di camera "ancorata" al suolo, ha stesse 
        // componenti del Front locale, ma con componente y a 0.
        this->WorldFront.y = 0.0f;
        // Una volta calcolata la nuova direzione di vista, ricalcolo il vettore Right come prodotto vettoriale tra Front e la UP del mondo
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
        // Calcolo il vettore UP locale della camera, come prodotto vettoriale tra Front e Right
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
