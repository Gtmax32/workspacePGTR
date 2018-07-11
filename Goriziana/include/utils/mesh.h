/*
Classe Mesh
- Alloca e inizializza i buffer (VBO, VAO, EBO), e imposta come OpenGL deve interpretare i dati nei buffer 
- Carica ed applica le texture
*/

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utils/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

/*
 * Variabile globale che rappresenta la struttura dati per i vertici
 */
struct Vertex {
    glm::vec3 Position;

    glm::vec3 Normal;

    glm::vec2 TexCoords;

    glm::vec3 Tangent;

    glm::vec3 Bitangent;
};

/*
 * Variabile globale che rappresenta la struttura dati per le textures
 */
struct Texture {
    GLuint id;
    string type;
    string path;
};

/********** classe MESH **********/
class Mesh {
public:
    // Attributo che memorizza i vertici della mesh
    vector<Vertex> vertices;
    // Attributo che memorizza gli indici dei vertici della mesh
    vector<GLuint> indices;
    // Attributo che memorizza le texture da assegnare alle facce
    vector<Texture> textures;
    
	// Attributo che memorizza il Vertex Attribut Object, utilizzato per renderizzare la mesh
	GLuint VAO;

    /*
     * Costruttore
     * Prende in input i vertici, gli indici dei vertici e le texture in modo da renderizzare la mesh.
     * Chiama il metodo setupMesh per settare i parametri iniziali (VAO, VBO ed EBO).
     */
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures) { // @suppress("Class members should be properly initialized")
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        this->setupMesh();
    }

    /*
     * Metodo utilizzato per renderizzare effettivamente la mesh mediante lo shader in input.
     * Prende in input i seguenti valori:
     * - shader: Shader, rappresenta lo shader compilato con tutte le informazioni per la corretta renderizzazione.
     */
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
				ss << diffuseNr++;
			else if(name == "texture_specular")
					ss << specularNr++;
				 else if(name == "texture_normal")
						 ss << normalNr++;
					  else if(name == "texture_height")
							  ss << heightNr++;

            shader.setFloat(("material." + name + number).c_str(), i);

            glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
        }
        
        // Rende attivo il VAO
        glBindVertexArray(VAO);
		// Renderizza i dati presenti nel VAO appena collegato
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		// Scollega il VAO
        glBindVertexArray(0);

        for (GLuint i = 0; i < this->textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
	
	/*
	 * Metodo che, nel momento della chiusura dell'applicazione, dealloca i buffer utilizzati.
	 */
    void Delete(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
    // Attributi che rappresentano il Vertex Buffer Object e Element Buffer Object
    GLuint VBO, EBO;

    /*
     * Metodo utilizzato per inizializzare VAO, VBO e EBO
     */
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
