/*
Classe Mesh v3
- Alloca e inizializza i buffer (VBO, VAO, EBO), e imposta come OpenGL deve interpretare i dati nei buffer 
- Carica ed applica le texture
*/

#ifndef MESH_H
#define MESH_H

// Contiene tutte gli include necessari ad OpenGL
#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utils/shader_v2.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

// Struttura dati per i vertici
struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

// Struttura dati per le texture
struct Texture {
    GLuint id;
    string type;
    string path;
};

/********** classe MESH **********/
class Mesh {
public:
    // Vettori per i dati di vertici e indici dei vertici per le facce
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    
	// Vertex Array Object
	GLuint VAO;

    // Costruttore
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // Inizializzo i buffer di OpenGL: VBO e EBO
        this->setupMesh();
    }

    // Renderizzo il modello
    void Draw(Shader shader) {
        GLuint diffuseNr  = 1;
        GLuint specularNr = 1;
        GLuint normalNr   = 1;
        GLuint heightNr   = 1;
        
		for(GLuint i = 0; i < this->textures.size(); i++) {
            // Attiva la texture unit adatta
			glActiveTexture(GL_TEXTURE0 + i);
            // Recupera il numero della texture, partendo dalla convenzione "texture_diffuseN"
            stringstream ss;
			string number;
            string name = textures[i].type;
            if(name == "texture_diffuse")
				ss << diffuseNr++; // Transfer GLuint to stream
			else if(name == "texture_specular")
					ss << specularNr++; // Transfer GLuint to stream
				 else if(name == "texture_normal")
						 ss << normalNr++; // Transfer GLuint to stream
					  else if(name == "texture_height")
							  ss << heightNr++; // Transfer GLuint to stream

			// Setta l'uniform sampler2D con la corretta texture unit
            shader.setFloat(("material." + name + number).c_str(), i);
            // e li associa
            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }
        
        // Rende attivo il VAO
        glBindVertexArray(VAO);
		// Renderizza i dati presenti nel VAO appena collegato
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		// Scollega il VAO
        glBindVertexArray(0);

        // E' buona pratica settare tutto a default, una volta che è terminata la configurazione
        for (GLuint i = 0; i < this->textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
	
	// Nel momento della chiusura dell'applicazione, dealloca i buffer
    void Delete(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
    // VBO e EBO
    GLuint VBO, EBO;

    // Inizializzo i buffer objects\array
    void setupMesh(){
        // Crea i buffer
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);
		
		// Rende attivo il VAO
        glBindVertexArray(VAO);
        // Carica i dati nel VBO - devo indicare la dimensione dei dati, e il puntatore alla struttura dati che li contiene
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  
		// Carica i dati nell' EBO - devo indicare la dimensione dei dati, e il puntatore alla struttura dati che li contiene
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
#endif