/*
Classe Shader v2
- implementazione classe per caricamento codice shader e creazione Program Shader

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

/********** classe SHADER **********/
class Shader{

private:
	std::string shaderName;
	
public:
    GLuint ID;
    // Costruttore della classe Shader
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr){
		// Passo 0: salvo il nome dello shader, in modo da avere informazioni più dettagliate negli errori
		char* tmp = (char*) calloc(50, sizeof(char));
		
		strcpy(tmp, vertexPath);		
		//shaderName = strtok(tmp, "/");
		shaderName = strtok(tmp, ".");
		
		/*strcpy(tmp, fragmentPath);
		fragmentShaderName = strtok(tmp, "/");
		fragmentShaderName = strtok(nullptr, ".");
		
		std::cout << "VS: " << vertexShaderName << "\nFS: " << fragmentShaderName << std::endl;
		
		if(geometryPath != nullptr) {
			strcpy(tmp, geometryPath);
			geometryShaderName = strtok(tmp, "/");
			geometryShaderName = strtok(nullptr, "/");
			
			//std::cout << "GS: " << geometryShaderName << std::endl;
		}*/
		
		// Passo 1: carico i sorgenti degli shader dal path passato come parametro
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        
		// Leggo il contenuto dei file, gestisco errori con eccezioni
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        
		try {
            // Apro i file
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // Leggo i file mediante gli stream
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();		
            // Chiudo i file
            vShaderFile.close();
            fShaderFile.close();
            // Converto gli stream in stringa
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();			
            // Se è presente il GS, passo alla lettura del file
            if(geometryPath != nullptr) {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure & e){
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
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
        
		// Se è presente il GS, lo compilo
        GLuint geometry;
        if(geometryPath != nullptr){
            const char * gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
		
        // Creo Shader Program
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
	
    // "Installa" il program shader come parte del processo di rendering attuale
    void Use() { 
        glUseProgram(this->ID); 
    }
	// Cancella il program shader in fase di chiusura dell'applicazione
    void Delete() {    
		glDeleteProgram(this->ID); 
	}
	
    // Funzioni Uniform
    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value); 
    }
    
    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value); 
    }
    
    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value); 
    }
    
    void setVec2(const std::string &name, const glm::vec2 &value) const { 
        glUniform2fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    void setVec2(const std::string &name, float x, float y) const { 
        glUniform2f(glGetUniformLocation(this->ID, name.c_str()), x, y); 
    }
    
    void setVec3(const std::string &name, const glm::vec3 &value) const { 
        glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    void setVec3(const std::string &name, float x, float y, float z) const { 
        glUniform3f(glGetUniformLocation(this->ID, name.c_str()), x, y, z); 
    }
    
    void setVec4(const std::string &name, const glm::vec4 &value) const { 
        glUniform4fv(glGetUniformLocation(this->ID, name.c_str()), 1, &value[0]); 
    }
	
    void setVec4(const std::string &name, float x, float y, float z, float w) { 
        glUniform4f(glGetUniformLocation(this->ID, name.c_str()), x, y, z, w); 
    }
    
    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // Funzione utilizzata per controllare eventuali errori nella compilazione di VS e FS
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        
		if(type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            
			if(!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << " in SHADER: " << shaderName << ".\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            
			if(!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << " in SHADER: " << shaderName << ".\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif
