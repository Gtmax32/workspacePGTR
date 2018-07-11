/*
Classe BulletDebugDrawer
- Eredita e ridefinisce da btIDebugDraw parte dei metodi ed attributi utili per il debug della simulazione fisica

Codice di partenza: https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11517
*/

#ifndef BULLETDEBUGDRAWER_H
#define BULLETDEBUGDRAWER_H

#include <iostream>

#include <bullet/LinearMath/btIDebugDraw.h>
#include <utils/shader.h>

/********** classe BULLETDEBUGDRAWER **********/
class BulletDebugDrawer : public btIDebugDraw {
public:	
	// Attributo che rappresenta la tipologia di debug da attivare
	int m_debugMode;
	
	// Costruttore della classe
	BulletDebugDrawer();
	// Distruttore della classe
	~BulletDebugDrawer();
	
	/*
	 * Metodo utilizzato per settare le matrici Projection, View e Model dello shader del debugger.
	 * Prende in input i seguenti valori:
	 * - shader: Shader*, puntatore allo shader associato al debugger
	 * - viewMatrix: glm::mat4, matrice View della scena
	 * - projectionMatrix: glm::mat4, matrice Projection della scena
	 * - modelMatrix: glm::mat4, matrice Model della scena
	 */
	void SetMatrices(Shader *shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::mat4 modelMatrix);

	/*
	 * Metodo utilizzato per disegnare effettivamente i contorni degli oggetti fisici
	 */
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	
	virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&);
	virtual void reportErrorWarning(const char *);
	virtual void draw3dText(const btVector3 &, const char *);
	
	/*
	 * Metodo set per m_debugMode;
	 */
	virtual void setDebugMode(int p);

	/*
	 * Metodo get per m_debugMode;
	 */
	virtual int getDebugMode() const;
};

#endif // BULLETDEBUGDRAWER_H
