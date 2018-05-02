/*
Classe Shader v1
- implementazione classe per caricamento codice shader e creazione Program Shader

NB) adattamento di https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/shader.h

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano
*/

#pragma once

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// GL Includes
#include <glad/glad.h> // Contains all the necessery OpenGL includes

/////////////////// classe SHADER ///////////////////////
class Shader
{
public:
    GLuint Program;
    
    //////////////////////////////////////////
    
    //Costruttore
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        // Passo 1: carico i sorgenti degli shader dal path passato come parametro        
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        
        // Leggo il contenuto dei file, gestisco errori con eccezioni
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();		
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();			
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        
        // converto le stringhe in puntatori a char
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar * fShaderCode = fragmentCode.c_str();
        
        // Passo 2: compilo gli shader
        GLuint vertex, fragment;
        
        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // controllo errori in fase di compilazione
        checkCompileErrors(vertex, "VERTEX");
        
        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
		// controllo errori in fase di compilazione
        checkCompileErrors(fragment, "FRAGMENT");
        
        // Creo Shader Program
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertex);
        glAttachShader(this->Program, fragment);
        glLinkProgram(this->Program);
        // controllo errori in fase di linking
        checkCompileErrors(this->Program, "PROGRAM");
        
        // gli shader sono linkati allo Shader Program, li posso cancellare
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    //////////////////////////////////////////

    // "Installa" il program shader come parte del processo di rendering attuale
    void Use() { glUseProgram(this->Program); }

    // Cancella il program shader in fase di chiusura dell'applicazione
    void Delete() {    glDeleteProgram(this->Program); }

private:
    //////////////////////////////////////////

    // Check errori di compilazione e linking degli shader
    void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if(type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if(!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if(!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
			}
		}
	}
};