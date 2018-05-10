/*
 * camera_v4.h
 *
 *  Created on: 09 mag 2018
 *      Author: Gtmax
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <iostream>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Euler angle for rotation around y axis
const GLfloat YAW = 0.0f;
// Euler angle for rotation around x axis
const GLfloat PITCH = 0.0f;

// Camera parameters for user interaction
const GLfloat SPEED = 1.0f;
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

//	GLfloat Yaw;
//	GLfloat Pitch;

//	GLfloat movementSpeed;
//	GLfloat mouseSensitivity;
//	GLfloat Zoom;

// Constructor
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat viewX, GLfloat viewY, GLfloat viewZ) {
		this->Position = vec3(posX, posY, posZ);
		this->View = vec3(viewX, viewY, viewZ);
		this->localUp = vec3(0.0f, 1.0f, 0.0f);
	}

	void positionCamera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat viewX, GLfloat viewY, GLfloat viewZ, GLfloat upX, GLfloat upY, GLfloat upZ) {
		this->Position = vec3(posX, posY, posZ);
		this->View = vec3(viewX, viewY, viewZ);
		this->localUp = vec3(upX, upY, upZ);
	}

	void rotateView(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
		vec3 newView = vec3(0.0f);

		// Get the direction the camera facing
		vec3 view = this->View - this->Position;

		// Calculate the sine and cosine of the angle once
		GLfloat cosTheta = (GLfloat) cos(angle);
		GLfloat sinTheta = (GLfloat) sin(angle);

		// Find the new x position for the new rotated point
		newView.x = (cosTheta + (1 - cosTheta) * x * x) * view.x;
		newView.x += ((1 - cosTheta) * x * y - z * sinTheta) * view.y;
		newView.x += ((1 - cosTheta) * x * z + y * sinTheta) * view.z;

		// Find the new y position for the new rotated point
		newView.y = ((1 - cosTheta) * x * y + z * sinTheta) * view.x;
		newView.y += (cosTheta + (1 - cosTheta) * y * y) * view.y;
		newView.y += ((1 - cosTheta) * y * z - x * sinTheta) * view.z;

		// Find the new z position for the new rotated point
		newView.z = ((1 - cosTheta) * x * z - y * sinTheta) * view.x;
		newView.z += ((1 - cosTheta) * y * z + x * sinTheta) * view.y;
		newView.z += (cosTheta + (1 - cosTheta) * z * z) * view.z;

		//Add the newly rotated vector to camera position to set the new rotated view of the camera
		this->View = this->Position + newView;
	}

	void rotateAroundObject(vec3 objPos, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
		vec3 newPosition = vec3(1.0f);

		//Get the direction vector from camera to obj
		vec3 direction = normalize(this->Position - objPos);
		std::cout << "Direction: " << to_string(direction) << std::endl;

//		// Calculate the sine and cosine of the angle once
//		GLfloat cosTheta = (GLfloat) cos(angle);
//		GLfloat sinTheta = (GLfloat) sin(angle);
//
//		// Find the new x position for the new rotated point
//		newPosition.x = (cosTheta + (1 - cosTheta) * x * x) * direction.x;
//		newPosition.x += ((1 - cosTheta) * x * y - z * sinTheta) * direction.y;
//		newPosition.x += ((1 - cosTheta) * x * z + y * sinTheta) * direction.z;
//
//		// Find the new y position for the new rotated point
//		newPosition.y = ((1 - cosTheta) * x * y + z * sinTheta) * direction.x;
//		newPosition.y += (cosTheta + (1 - cosTheta) * y * y) * direction.y;
//		newPosition.y += ((1 - cosTheta) * y * z - x * sinTheta) * direction.z;
//
//		// Find the new z position for the new rotated point
//		newPosition.z = ((1 - cosTheta) * x * z - y * sinTheta) * direction.x;
//		newPosition.z += ((1 - cosTheta) * y * z + x * sinTheta) * direction.y;
//		newPosition.z += (cosTheta + (1 - cosTheta) * z * z) * direction.z;
//
//		// Now we just add the newly rotated vector to our position to set
//		// our new rotated position of our camera.
//		this->Position = objPos + newPosition;

		mat4 translation = translate(mat4(1.0f), this->Position);
		std::cout << "Translation: " << to_string(translation) << std::endl;
		mat4 rotation = rotate(translation, angle, vec3(x,y,z));
		std::cout << "Rotation: " << to_string(rotation) << std::endl;
		mat4 completeTransformation = translate(rotation, -this->Position);
		std::cout << "Back Translation: " << to_string(completeTransformation) << std::endl;

		this->Position = completeTransformation * vec4(this->Position, 1.0f);

	}

	void moveCamera(GLfloat speed) {
		vec3 direction = this->View - this->Position;

		this->Position.x += direction.x * speed;
		this->Position.z += direction.z * speed;

		this->View.x += direction.x * speed;
		this->View.z += direction.z * speed;
	}

	void setPerspective(GLfloat fov, GLfloat aspectRatio, GLfloat nearPlane, GLfloat farPlane) {
		this->projectionMatrix = perspective(fov, aspectRatio, nearPlane, farPlane);
	}

	mat4 getProjectionMatrix() {
		return this->projectionMatrix;
	}

	mat4 getViewMatrix() {
		return lookAt(this->Position, this->View, this->localUp);
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
