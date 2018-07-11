/*
Classe Shader
- implementazione classe per caricamento Vertex Shader, Fragment Shader e creazione Program Shader
*/

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/********** classe SHADER **********/
class Shader{

private:
	string shaderName;
	
public:
	// Attributo che conterra' l'ID con cui verra' salvato il Program Shader in memoria
    GLuint ID;

    /*
     * Costruttore
     * Prende in input i seguenti valori:
     * - vertexPath: char*, stringa che memorizza il path del file .vert
     * - fragmentPath: char*, stringa che memorizza il path del file .frag
     * - geometryPath: char*, stringa che memorizza il path del file .geom, impostato di default a nullptr.
     */
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr){
		// Passo 0: salvo il nome dello shader, in modo da avere informazioni piu' dettagliate negli errori
		char* tmp = (char*) calloc(50, sizeof(char));
		
		strcpy(tmp, vertexPath);		
		shaderName = strtok(tmp, "/");
		shaderName = strtok(nullptr, ".");

		// Passo 1: carico i sorgenti degli shader dal path passato come parametro
        string vertexCode;
        string fragmentCode;
        string geometryCode;
        ifstream vShaderFile;
        ifstream fShaderFile;
        ifstream gShaderFile;
        
		// Leggo il contenuto dei file, gestisco errori con eccezioni
        vShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
        gShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
        
		try {
            // Apro i file
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            stringstream vShaderStream, fShaderStream;

            // Leggo i file mediante gli stream
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // Chiudo i file
            vShaderFile.close();
            fShaderFile.close();

            // Converto gli stream in stringa
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();			

            // Se e' presente il GS, passo alla lettura del file
            if(geometryPath != nullptr) {
                gShaderFile.open(geometryPath);
                stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (ifstream::failure & e){
            cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
        }
		
        // converto le stringhe in puntatori a char
		const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();

        // Passo 2: compilo gli shader
        GLuint vertex, fragment;
                
		// Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        
		// Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
		// Se e' presente il GS, lo compilo
        GLuint geometry;
        if(geometryPath != nullptr){
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
		
        // Passo 3: Creo Program Shader
        this->ID = glCreateProgram();
        
		glAttachShader(this->ID, vertex);
        glAttachShader(this->ID, fragment);
        if(geometryPath != nullptr)
            glAttachShader(this->ID, geometry);
        glLinkProgram(this->ID);
        checkCompileErrors(this->ID, "PROGRAM");
        
		// gli shader sono linkati allo Shader Program, li posso cancellare
        glDetachShader(this->ID, vertex);
        glDetachShader(this->ID, fragment);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if(geometryPath != nullptr)
        	glDetachShader(this->ID, geometry);
            glDeleteShader(geometry);
    }
	
    /*
     * Metodo che attiva il program shader come parte del processo di rendering attuale
     */
    void Use() { 
        glUseProgram(this->ID); 
    }

	/*
	 * Metodo che cancella il program shader in fase di chiusura dell'applicazione
	 */
    void Delete() {    
		glDeleteProgram(this->ID); 
	}
	
    /*
     * Metodo di utility. Utilizzato per settare un bool nello shader attivo.
     * Prendo in input i seguenti valori:
     * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
     * - value: bool, valore da dare alla variabile uniform individuata dal precedente parametro
     */
    void setBool(const string &name, bool value) const {
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un int nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: int, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setInt(const string &name, int value) const {
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un float nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: float, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setFloat(const string &name, float value) const {
        glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un vec2 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: vec2, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec2(const string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    /*
	 * Metodo di utility. Utilizzato per settare un vec2 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - x, y: float, valori da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec2(const string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(this->ID, name.c_str()), x, y); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un vec3 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: vec3, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec3(const string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    /*
	 * Metodo di utility. Utilizzato per settare un vec3 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - x, y, z: float, valori da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec3(const string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(this->ID, name.c_str()), x, y, z); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un vec4 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: vec4, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec4(const string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    /*
	 * Metodo di utility. Utilizzato per settare un vec4 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - x, y, z, w: float, valori da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setVec4(const string &name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(this->ID, name.c_str()), x, y, z, w); 
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un mat2 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: mat2, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setMat2(const string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un mat3 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: mat3, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setMat3(const string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    /*
	 * Metodo di utility. Utilizzato per settare un mat4 nello shader attivo.
	 * Prendo in input i seguenti valori:
	 * - name: string, nome con cui è salvata nello shader la variabile di tipo uniform associata
	 * - value: mat4, valore da dare alla variabile uniform individuata dal precedente parametro
	 */
    void setMat4(const string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    /*
     * Metodo utilizzato per controllare eventuali errori nella compilazione di VS e FS e nel link per la creazione del PS.
     * In caso affermativo, viene segnalato un messaggio con informazioni riguardo l'errore.
     * Prende in input i seguenti valori:
     * - shader: GLuint, contiene il valore ID con cui viene salvato lo shader in memoria
     * - type: string, variabile contenente il tipo di shader da analizzare per la ricerca di errori.
     */
    void checkCompileErrors(GLuint shader, string type) {
        GLint success;
        GLchar infoLog[1024];
        
		if(type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            
			if(!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << " in SHADER: " << shaderName << ".\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            
			if(!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << " in SHADER: " << shaderName << ".\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
            }
        }
    }
};
#endif
