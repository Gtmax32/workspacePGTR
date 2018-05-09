/*
 * camera_v4.h
 *
 *  Created on: 09 mag 2018
 *      Author: Gtmax
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Euler angle for rotation around y axis
const GLfloat YAW = 0.0f;
// Euler angle for rotation around x axis
const GLfloat PITCH = 0.0f;

// Camera parameters for user interaction
const GLfloat SPEED = 5.0f;
const GLfloat SENSITIVITY = 0.1f;
const GLfloat ZOOM = 45.0f;

using namespace glm;

/********** CAMERA class **********/
class Camera {
public:
	// Camera position attribute
	vec3 Position;
	// Camera view vector attribute
	vec3 View;
	// Camera right vector attribute
	vec3 Right;
	// Camera up vector attribute
	vec3 localUp;

	// Up vector in world coordinates
	vec3 worldUp;

	GLfloat Yaw;
	GLfloat Pitch;

	//	GLfloat movementSpeed;
	//	GLfloat mouseSensitivity;
	//	GLfloat Zoom;

	// Constructor

	void rotateView(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
		vec3 newView = vec3(0.0f);

		// Get the direction the camera facing
		vec3 view = this->View - this->Position;

		GLfloat cosTheta = (GLfloat) cos(angle);
		GLfloat sinTheta = (GLfloat) sin(angle);

		newView.x = (cosTheta + (1 - cosTheta) * x * x) * view.x;
		newView.x += ((1 - cosTheta) * x * y - z * sinTheta) * view.y;
		newView.x += ((1 - cosTheta) * x * z + y * sinTheta) * view.z;

		newView.y = ((1 - cosTheta) * x * y + z * sinTheta) * view.x;
		newView.y += (cosTheta + (1 - cosTheta) * y * y) * view.y;
		newView.y += ((1 - cosTheta) * y * z - x * sinTheta) * view.z;

		newView.z = ((1 - cosTheta) * x * z - y * sinTheta) * view.x;
		newView.z += ((1 - cosTheta) * y * z + x * sinTheta) * view.y;
		newView.z += (cosTheta + (1 - cosTheta) * z * z) * view.z;

		this->View = this->Position + newView;
	}

	void positionCamera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat viewX,
			GLfloat viewY, GLfloat viewZ, GLfloat upX, GLfloat upY,
			GLfloat upZ) {
		this->Position = vec3(posX, posY, posZ);
		this->View = vec3(viewX, viewY, viewZ);
		this->localUp = vec3(upX, upY, upZ);
	}

	void setPerspective(GLfloat fov, GLfloat aspectRatio, GLfloat nearPlane,
			GLfloat farPlane) {
		this->projectionMatrix = perspective(fov, aspectRatio, nearPlane,
				farPlane);
	}

	mat4 getProjectionMatrix() {
		return this->projectionMatrix;
	}

	mat4 getViewMatrix() {
		return lookAt(this->selectedBallPos, this->selectedBallPos + this->View,
				this->localUp);
	}

	void setObjectPos(vec3 objPos) {
		this->selectedBallPos = objPos;
	}

	vec3 getObjectPos() {
		return this->selectedBallPos;
	}

protected:
	// Object around which the camera rotate
	vec3 selectedBallPos;
	mat4 projectionMatrix;

};

#endif /* INCLUDE_UTILS_CAMERA_V4_H_ */
