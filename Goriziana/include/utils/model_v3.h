/*
Classe Model v3
- implementazione classe per caricamento modello OBJ tramite libreria Assimp
- crea le strutture dati per la creazione e inizializzazione degli VBO, VAO e EBO
- carica ed applica le texture eventualmente definite nel modello, come esportate dal SW di modellazione
*/

#ifndef MODEL_H
#define MODEL_H

// Contiene tutte gli include necessari ad OpenGL
#include <glad/glad.h> 
// Utilizzo le strutture dati di GLM per convertire i dati dalla struttura dati di Assimp nella struttura dati adatta ai buffer VBO, VAO e EBO
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Libreria per il caricamento delle immagini
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <utils/mesh_v3.h>
#include <utils/shader_v2.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

GLuint TextureFromFile(const char *path, const string &directory, bool gamma = false);

/********** classe MODEL **********/
class Model {
public:
    // Vector contenente le texture del modello. Il salvataggio è ottimizzato in modo che una stessa texture non venga letta due volte.
    vector<Texture> textures_loaded;
	// Vector di mesh
    vector<Mesh> meshes;
	// Directory del modello
    string directory;
    bool gammaCorrection;

    // Costruttore
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma) {
        this->loadModel(path);
    }

    // Renderizza il modello chiamando il metodo di rendering delle istanze della classe Mesh
    void Draw(Shader shader){
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(shader);
    }
	
	// Distruttore. Alla chiusura dell'applicazione, fa deallocare la memoria dei buffer alle istanze della classe Mesh
    virtual ~Model(){
        for(GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Delete();
    }
    
private:

    // Carica il modello usando Assimp, e processa i nodi per ottenere un vector di istanze di Mesh
    void loadModel(string const &path){
        // Legge il file utilizzando la classe Importer della libreria
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
		
        // Controlla eventuali errori
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
		
        // Recupera la directory dal path del modello
        this->directory = path.substr(0, path.find_last_of('/'));

        // Avvia il processing dei nodi della struttura dati di Assimp
        this->processNode(scene->mRootNode, scene);
    }

    // Processa ricorsivamente i nodi della struttura dati di Assimp
    void processNode(aiNode *node, const aiScene *scene){
        // Processa ogni mesh presente nel nodo corrente
        for(GLuint i = 0; i < node->mNumMeshes; i++) {
            // L'oggetto "node" contiene solo indici per gli oggetti della scena, che sono mantenuti in "scene".
            // La classe aiNode è solo un mezzo per indicare uno o più mesh all'interno della scena
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			
			// Avvia il processing della mesh di Assimp tramite il metodo processMesh.
            // Il risultato (una istanza della classe Mesh) verrà aggiunta al vector delle mesh.
            this->meshes.push_back(processMesh(mesh, scene));
        }
        
		// Ripete ricorsivamente per tutti i figli del nodo corrente
        for(GLuint i = 0; i < node->mNumChildren; i++){
            processNode(node->mChildren[i], scene);
        }
    }
	
	// Gestione della mesh di Assimp per ottenere una "mesh OpenGL", 
    // ossia creazione ed allocazione dei buffer da inviare alla scheda grafica in fase di rendering GPU
    Mesh processMesh(aiMesh *mesh, const aiScene *scene){
        
		// Vettori per i dati di vertici e indici dei vertici per le facce
        vector<Vertex> vertices;
        vector<GLuint> indices;
        // Vettore di tutte le texture del modello
		vector<Texture> textures;

        // Analizzo ognuno dei vertici che compongono la mesh in considerazione
        for(GLuint i = 0; i < mesh->mNumVertices; i++){
            Vertex vertex;
			
			// Il tipo di dato usato da assimp per un vettore a 3 dimensioni non è uguale a quello che serve in fase di allocazione di memoria per i buffer,
            // pertanto vengono convertite tutte le strutture dati di assimp in strutture dati di GLM, che sono uguali a quelle che servono per le operazioni OpengGL
            glm::vec3 vector;
            
			// Coordinate di posizione dei vertici
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            
			// Coordinate delle Normali
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            
			// Coordinate delle texture
			// Se il modello ha le coordinate texture, vengono assegnate ad una struttura dati GLM, altrimenti vengono settate a 0.
            if(mesh->mTextureCoords[0]){
                glm::vec2 vec;
                // Si assume che il modello abbia un solo insieme di coordinate texture.
                // In realtà, è possibile avere fino a 8 diverse coordinate texture: per altri modelli e formati, bisognerò adattare questo codice.
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
            // Aggiunge il vertice al vector dei vertici
			vertices.push_back(vertex);
        }
		
        // Ricava gli indici dei vertici che compongono le facce della mesh
        for(GLuint i = 0; i < mesh->mNumFaces; i++){
            aiFace face = mesh->mFaces[i];
            
            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
		
        // Processa i materiali definiti nel file del modello
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // Si assume la convenzione per gli uniform sampler negli shader, dove ogni texture di un certo tipo prende il nome di texture_typeN ed N rappresenta il numero del sampler progressivo da 1 a MAX_SAMPLER_NUMBER.
		// Tipo di texture -> Nome sampler
		//         diffuse -> texture_diffuseN
        //        specular -> texture_specularN
        //          normal -> texture_normalN
		//           heigh -> texture_heightN

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
        
        // Restituisce un'istanza della classe Mesh, avente le liste di vertici e facce appena create
        return Mesh(vertices, indices, textures);
    }

    // Carica (se non ancora caricate) le texture definite dai materiali del modello (se definiti)
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName){
        vector<Texture> textures;
		
        for(GLuint i = 0; i < mat->GetTextureCount(type); i++){
            
			aiString str;
            mat->GetTexture(type, i, &str);
            // Controlla se la texture era già stata caricata
            GLboolean skip = false;
            
			for(GLuint j = 0; j < textures_loaded.size(); j++){
				
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
					// E' già stata caricata la texture con lo stesso path quindi procede alla successiva
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if(!skip){
				
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); 
            }
        }
        return textures;
    }
};

// Caricamento della texture da un file su disco, e creazione Texture Unit di OpenGL
GLuint TextureFromFile(const char *path, const string &directory, bool gamma){
    string filename = string(path);
    filename = directory + '/' + filename;

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data){
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

    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        
    }
	
	stbi_image_free(data);
	
    return textureID;
}
#endif