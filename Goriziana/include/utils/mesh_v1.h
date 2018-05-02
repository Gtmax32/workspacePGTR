/*
Classe Mesh v1
- Alloca e inizializza i buffer (VBO, VAO, EBO), e imposta come OpenGL deve interpretare i dati nei buffer 

NB1) in questa versione della classe non sono caricate e applicate le texture

NB2) adattamento di https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/mesh.h

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano
*/

#pragma once

using namespace std;

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// GL Includes
#include <glad/glad.h> // Contains all the necessery OpenGL includes
// Utilizzerò le strutture dati di GLM per scrivere i dati nei buffer VBO, VAO e EBO
#include <glm/glm.hpp>

// Struttura dati per i vertici
struct Vertex {
    // Coordinate vertici
    glm::vec3 Position;
    // Normali
    glm::vec3 Normal;
    // Coordinate texture
    glm::vec2 TexCoords;
    // Tangenti
    glm::vec3 Tangent;
    // Bitangenti
    glm::vec3 Bitangent;
};

/////////////////// classe MESH ///////////////////////
class Mesh {
public:
    // vettori per i dati di vertici e indici dei vertici per le facce
    vector<Vertex> vertices;
    vector<GLuint> indices;
    // VAO
    GLuint VAO;

    //////////////////////////////////////////   
    // Costruttore
    Mesh(vector<Vertex> vertices, vector<GLuint> indices)
    {
        this->vertices = vertices;
        this->indices = indices;

        // Inizializzo i buffer di OpenGL
        this->setupMesh();
    }

    //////////////////////////////////////////

    // Renderizza il modello 
    void Draw(Shader shader) 
    {
        // Rende "attivo" il VAO
        glBindVertexArray(this->VAO);
        // Renderizza i dati nel VAO appena collegato
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        // Scollega il VAO
        glBindVertexArray(0);
    }

    //////////////////////////////////////////    

    // in chiusura di applicazione, dealloca i buffer
    void Delete()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
    // VBO e EBO
    GLuint VBO, EBO;

    //////////////////////////////////////////
    // Inizializzo i buffer objects\array
    // Una breve descrizione del loro ruolo e di come sono collegati può essere trovata a:
    // https://learnopengl.com/#!Getting-started/Hello-Triangle
    // (in vari punti nella pagina), oppure qui:
    // http://www.informit.com/articles/article.aspx?p=1377833&seqNum=8 
    void setupMesh()
    {
        // Creo i buffer
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        // Rende "attivo" il VAO
        glBindVertexArray(this->VAO);
        // Carico i dati nel VBO - devo indicare la dimensione dei dati, e il puntatore alla struttura dati che li contiene
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  
        // Carico i dati nell' EBO - devo indicare la dimensione dei dati, e il puntatore alla struttura dati che li contiene
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Setto nel VAO i puntatori ai vari attributi del vertice (con relativi offset all'interno della struttura dati)
        // Posizioni dei vertici
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Normali
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Coordinate texture
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
        // Tangenti
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
        // Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};



