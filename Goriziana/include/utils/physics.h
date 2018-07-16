/*
Classe Physics
- Definisce tutte le variabili necessarie alla creazione del dynamicsWorld
- Crea un rigidBody, completo di tutto, in base ai parametri passati dall'utente
*/

#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>

/********** classe PHYSICS **********/
class Physics{
public:
	// Attributo che rappresenta la classe fondamentale per la simulazione fisica
    btDiscreteDynamicsWorld* dynamicsWorld;
    // Attributo che conserva tutte le Collision Shape di tutti gli elementi della scena
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    // Attributo che rappresenta la configurazione per il collision manager
    btDefaultCollisionConfiguration* collisionConfiguration;
    // Attributo che rappresenta il collision manager
    btCollisionDispatcher* dispatcher;
    // Attributo che rappresenta la tipologia di collision detection
    btBroadphaseInterface* overlappingPairCache;
    // Atributo che gestisce i constraint della scena
    btSequentialImpulseConstraintSolver* solver;
    
    /*
     * Costruttore
     * Vengono impostati i parametri di base per la creazione del dynamicsWorld
     */
    Physics(){

        this->collisionConfiguration = new btDefaultCollisionConfiguration();

        this->dispatcher = new btCollisionDispatcher(collisionConfiguration);

        this->overlappingPairCache = new btDbvtBroadphase(); // @suppress("Abstract class cannot be instantiated")

        this->solver = new btSequentialImpulseConstraintSolver;

        this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration); // @suppress("Abstract class cannot be instantiated")

        this->dynamicsWorld->setGravity(btVector3(0,-9.82,0));
    }

    /*
     * Metodo per la creazione di un rigidBody, dotato di Collision Shape e parametri fisici.
     * Prende in input i seguenti valori:
     * - type: int, accetta i valori 0-Box Shape 1-Sphere Shape 2-Cylinder Shape
     * - pos: glm::vec3, indica la posizione iniziale del rigidBody
     * - size: glm::vec3, indica la dimensione del rigidBody
     * - rot: glm::vec3, indica la rotazione iniziale del rigidBody
     * - m: float, massa del rigidBody. Inserire 0 per corpo statico
     * - friction: float, rappresenta il coefficiente di attrito del corpo
     * - restitution: float, rappresenta il coefficiente di restituzione del corpo
     * Restituisce in output il puntatore all'oggetto btRigidBody, composto da tutte le caratteristiche fisiche indicate sopra.
     */
    btRigidBody* createRigidBody(int type, glm::vec3 pos, glm::vec3 size, glm::vec3 rot, float m, float friction , float restitution){

        btCollisionShape* cShape;

        btVector3 position = btVector3(pos.x,pos.y,pos.z);
        
        btQuaternion rotation;
        rotation.setEuler(rot.x,rot.y,rot.z);

        // Box
        if (type == 0) {
            btVector3 dim = btVector3(size.x,size.y,size.z);
            cShape = new btBoxShape(dim);
        } // Sphere
        else if (type == 1) {
            	cShape = new btSphereShape(size.x);
        	} // Cylinder
        	else if (type == 2){
        			btVector3 dim = btVector3(size.x, size.y, size.z);

        			btTransform localTransform;
        			localTransform.setIdentity();
        			localTransform.setOrigin(btVector3(0.0, 0.1, 0.0));

        			btCylinderShape* innerShape = new btCylinderShape(dim);

        			btCompoundShape* shape = new btCompoundShape();
        			shape->addChildShape(localTransform, innerShape);
        			cShape = shape;
        		}

        this->collisionShapes.push_back(cShape);

        btTransform objTransform;
        objTransform.setIdentity();
        objTransform.setRotation(rotation);
        objTransform.setOrigin(position);

        btScalar mass = m;
        bool isDynamic = (mass != 0.0f);

        btVector3 localInertia(0.0,0.0,0.0);
        if (isDynamic)
            cShape->calculateLocalInertia(mass,localInertia);

        btDefaultMotionState* motionState = new btDefaultMotionState(objTransform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,cShape,localInertia);

        rbInfo.m_friction = friction;
        rbInfo.m_restitution = restitution;

        if (type == 1){
            // Nella simulazione fisica, la sfera tocca il piano solo in un punto, e questo impedisce la corretta applicazione della frizione tra piano e sfera.
            // La rolling friction aggira il problema, e la unisco quindi a un fattore di damping angolare (che quindi applica una forza di resistenza durante la rotazione), in modo da far fermare la sfera dopo un po' di tempo.
            rbInfo.m_angularDamping = 0.4;
            rbInfo.m_rollingFriction = 0.4;
            rbInfo.m_linearDamping = 0.3;
        } else if (type == 2){
        		rbInfo.m_angularDamping = 0.05;
        	    rbInfo.m_rollingFriction = 0.04;
        }

        btRigidBody* body = new btRigidBody(rbInfo);

        this->dynamicsWorld->addRigidBody(body);

        return body;
    }

    /*
     * Metodo utilizzato per pulire la memoria dagli oggetti della simulazione fisica, una volta che il ciclo di rendering è terminato
     */
    void Clear(){
        for (int i=this->dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--){

            btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);

            if (body && body->getMotionState()){
                delete body->getMotionState();
            }

            this->dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }

        for (int j=0;j<this->collisionShapes.size();j++){
            btCollisionShape* shape = this->collisionShapes[j];
            this->collisionShapes[j] = 0;
            delete shape;
        }

        delete this->dynamicsWorld;

        delete this->solver;

        delete this->overlappingPairCache;

        delete this->dispatcher;

        delete this->collisionConfiguration;

        this->collisionShapes.clear();
    }
};
