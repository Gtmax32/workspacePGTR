/*
Classe Model v1
- implementazione classe per caricamento modello OBJ tramite libreria Assimp
- crea le strutture dati per la creazione e inizializzazione degli VBO, VAO e EBO

NB1) in questa versione della classe non sono caricate e applicate le texture

NB2) adattamento di https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/model.h

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
#include <map>
#include <vector>

// GL Includes
#include <glad/glad.h> // Contains all the necessery OpenGL includes
// Utilizzerò le strutture dati di GLM per convertire i dati dalla struttura dati di Assimp nella struttura dati adatta ai buffer VBO, VAO e EBO
#include <glm/glm.hpp>

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Includo la classe Mesh, che gestirà la parte "OpenGL" del caricamento dei modelli
// (ossia la creazione e allocazione di memoria per VBO, VAO, EBO )
#include <utils/mesh_v1.h>

/////////////////// classe MODEL ///////////////////////
class Model 
{
public:
    // alla fine del caricamento avrò un vector di istanze della classe Mesh
    vector<Mesh> meshes;
    
    //////////////////////////////////////////

    // Costruttore
    Model(const string& path)
    {
        this->loadModel(path);
    }

    //////////////////////////////////////////

    // Renderizza il modello chiamando il metodo di rendering delle istanze della classe Mesh
    void Draw(Shader shader)
    {
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(shader);
    }

    //////////////////////////////////////////

    // Distruttore. Alla chiusura dell'applicazione, fa deallocare la memoria dei buffer alle istanze della classe Mesh
    virtual ~Model()
    {
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Delete();
    }

    
private:

    //////////////////////////////////////////

    // Carico il modello usando Assimp, e processo i nodi per ottenere un vector di istanze di Mesh
    void loadModel(string path)
    {
        // Carico il file tramite Assimp
        // NB: è possibile indicare una serie di operazioni da far fare ad Assimp dopo il caricamento, se necessario.
        // Dettagli sui vari flag si trovano qui: http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410
        // IMPORTANTE: il calcolo di Tangenti e Bitangenti è possibile solo se il modello è dotato di COORDINATE TEXTURE.
        // In caso negativo, il calcolo non viene fatto (ma non è riportato nessun errore nel controllo successivi!)
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        // Controllo per errori (vedere commento precedente)
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        
        // Avvio il processing dei nodi della struttura dati di Assimp
        this->processNode(scene->mRootNode, scene);
    }

    //////////////////////////////////////////

    // Processo ricorsivamente i nodi della struttura dati di Assimp
    void processNode(aiNode* node, const aiScene* scene)
    {
        // processo ogni mesh presente nel nodo corrente
        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {
            // l'oggetto "node" contiene solo indici per gli oggetti della scena, che sono mantenuti in "scene".
            // In pratica, la classe aiNode è solo un mezzo per indicare uno o più mesh all'interno della scena
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
            // avvio il processing della mesh di Assimp tramite il metodo processMesh.
            // Il risultato (una istanza della classe Mesh) verrà aggiunta al vector delle mesh.
            this->meshes.push_back(this->processMesh(mesh, scene));			
        }
        // Ripeto ricorsivamente per tutti i figli del nodo corrente
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }

    }

    //////////////////////////////////////////

    // Processamento della mesh di Assimp per ottenere una "mesh OpenGL",
    // ossia la creazione e allocazione dei buffer da inviare alla scheda grafica in fase di rendering GPU

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // vettori per i dati di vertici e indici dei vertici per le facce
        vector<Vertex> vertices;
        vector<GLuint> indices;

        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // il tipo di dato usato da assimp per un vettore a 3 dimensioni non è uguale a quello che serve in fase di allocazione di memoria per i buffer
            // converto quindi tutte le strutture dati di assimp in strutture dati di GLM, che sono uguali a quelle che servono per le operazioni OpengGL
            glm::vec3 vector; 
            // Coordinate vertici
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // Normali
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // Texture Coordinates
            
            // Se il modello ha le coordinate texture, le assegno a una struttura dati GLM,
            // altrimenti le setto a 0.
            // Se ci sono le coordinate texture, Assimp può calcolare tangenti e bitangenti, altrimenti saranno inizializzate a 0
            if(mesh->mTextureCoords[0]) 
            {
                glm::vec2 vec;
                // In questo esempio, assumo che il modello abbia un solo insieme di coordinate texture.
                // In realtà, è possibile avere fino a 8 diverse coordinate texture: per altri modelli e formati, questo codice va adattato e modificato
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            
                // Tangenti
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // Bitangenti
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else{
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                cout << "WARNING::ASSIMP:: MODEL WITHOUT UV COORDINATES -> TANGENT AND BITANGENT ARE = 0" << endl;
            }
            // aggiungo il vertice alla lista    
            vertices.push_back(vertex);
        }
        
        // Ricavo gli indici dei vertici che compongono le facce della mesh
        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        
        // creo una istanza della classe Mesh passando le liste di vertici e facce appena create
        return Mesh(vertices, indices);
    } 
};