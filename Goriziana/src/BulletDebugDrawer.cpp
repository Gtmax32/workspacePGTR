/*
 * Codice di partenza: https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11517
 */

#include "BulletDebugDrawer.h"

GLuint VBO, VAO;

BulletDebugDrawer::BulletDebugDrawer(){
	this->m_debugMode = 1;
}

BulletDebugDrawer::~BulletDebugDrawer(){}

void BulletDebugDrawer::SetMatrices(Shader *shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::mat4 modelMatrix){
	//shader->Use();

	shader->setMat4("projectionMatrix", projectionMatrix);
	shader->setMat4("viewMatrix", viewMatrix);
	shader->setMat4("modelMatrix", modelMatrix);
}

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
	// Vertex data
	GLfloat points[12];

	points[0] = from.x();
	points[1] = from.y();
	points[2] = from.z();
	points[3] = 0.0f;
	points[4] = 0.0f;
	points[5] = 0.0f;

	points[6] = to.x();
	points[7] = to.y();
	points[8] = to.z();
	points[9] = 0.0f;
	points[10] = 0.0f;
	points[11] = 0.0f;

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

void BulletDebugDrawer::drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
void BulletDebugDrawer::reportErrorWarning(const char *warningString) {
	std::cout << warningString << std::endl;
}
void BulletDebugDrawer::draw3dText(const btVector3 &, const char *) {}
void BulletDebugDrawer::setDebugMode(int p) {
	m_debugMode = p;
}

int BulletDebugDrawer::getDebugMode() const { return m_debugMode; }
