#ifndef BULLETDEBUGDRAWER_H
#define BULLETDEBUGDRAWER_H

#include <iostream>

#include <utils/shader_v2.h>

#include <bullet/LinearMath/btIDebugDraw.h>

class BulletDebugDrawer : public btIDebugDraw {
public:	
	int m_debugMode;
	
	BulletDebugDrawer();
	~BulletDebugDrawer();
	
	void SetMatrices(Shader *shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::mat4 modelMatrix);
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	
	virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&);
	virtual void reportErrorWarning(const char *);
	virtual void draw3dText(const btVector3 &, const char *);
	
	virtual void setDebugMode(int p);
	virtual int getDebugMode() const;
};

#endif // BULLETDEBUGDRAWER_H
