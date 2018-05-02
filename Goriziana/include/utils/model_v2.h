/*
Classe Model v2
- implementazione classe per caricamento modello OBJ tramite libreria Assimp
- crea le strutture dati per la creazione e inizializzazione degli VBO, VAO e EBO

NB1) in questa versione della classe sono caricate e applicate le texture eventualmente definite nel modello, come esportate dal SW di modellazione

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

// includo la libreria per il caricamento delle immagini
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// Includo la classe Mesh, che gestirà la parte "OpenGL" del caricamento dei modelli
// (ossia la creazione e allocazione di memoria per VBO, VAO, EBO )
#include <utils/mesh_v2.h>

GLint TextureFromFile(const char* path, string directory);


/////////////////// classe MODEL ///////////////////////
class Model 
{
public:
    // un vector con le texture del modello
    vector<Texture> textures_loaded;    
    // alla fine del caricamento avrò un vector di istanze della classe Mesh
    vector<Mesh> meshes;
    // la directory del modello (serve per il caricamento delle texture, se il modello ne è provvisto)
    string directory;
    
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

        // recupera la directory dal path del modello
        this->directory = path.substr(0, path.find_last_of('/'));

        
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
        // vettore di tutte le texture del modello
        vector<Texture> textures;

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

        // processo i materiali definiti nel file del modello
        if(mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // We assume a convention for sampler names in the shaders. Each diffuse texture should be named
            // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
            // Same applies to other texture as the following list summarizes:
            // Diffuse: texture_diffuseN
            // Specular: texture_specularN
            // Normal: texture_normalN

            // 1. Diffuse maps
            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. Specular maps
            vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            // 3. Normal maps
            std::vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            // 4. Height maps
            std::vector<Texture> heightMaps = this->loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }
        
        // creo una istanza della classe Mesh passando le liste di vertici e facce appena create
        return Mesh(vertices, indices, textures);
    } 

    // Carica (se non ancora caricate) le texture definite dai materiali del modello (se definiti)
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // se la texture era già stata caricata, salta a quella successiva
            GLboolean skip = false;
            for(GLuint j = 0; j < textures_loaded.size(); j++)
            {
                if(textures_loaded[j].path == str)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // If texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};

// caricamento della texture da file su disco, e creazione Texture Unit di OpenGL
GLint TextureFromFile(const char* path, string directory)
{
     //Generate texture ID and load texture data 
    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = stbi_load(filename.c_str(), &width, &height, 0, STBI_rgb);
    
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);    

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    return textureID;
}
