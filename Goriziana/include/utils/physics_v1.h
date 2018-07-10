/*
Classe Physics_v1: classe per l'inizializzazione base della simulazione fisica con la libreria Bullet

La classe inizializza il gestore delle collisioni, e il risolutore dei constraints, 
utilizzando i metodi base. Per metodi avanzati e multithread, consultare la documentazione di Bullet

Il metodo createRigidBody permette di creare una Collision Shape di tipo Box o Sphere.
Per altre forme, estendere il metodo.
Si fa presente che i parametri di raggio e dimensioni vanno eventualmente calcolati a parte, a partire dalla mesh utilizzata.

autore: Davide Gadia

Programmazione Grafica per il Tempo Reale - a.a. 2016/2017
C.d.L. Magistrale in Informatica
Universita' degli Studi di Milano
*/

#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>

/////////////////// classe Physics ///////////////////////
class Physics
{
public:

    btDiscreteDynamicsWorld* dynamicsWorld;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    

    //////////////////////////////////////////   
    // Costruttore
    // Vengono impostati i parametri di base per la simulazione fisica (modalità di gestione delle collisioni, 
    //del risolutore delle equazioni differenziali per le forze e i constraint).
    Physics(){
        // Metodo per la Collision detection tra corpi rigidi (identificati come possibili collisioni dalla fase di BroadPhase Collision Detection)
        ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
        this->collisionConfiguration = new btDefaultCollisionConfiguration();

        ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
        this->dispatcher = new btCollisionDispatcher(collisionConfiguration);

        ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
        this->overlappingPairCache = new btDbvtBroadphase(); // @suppress("Abstract class cannot be instantiated")

        // Imposto il metodo di integrazione numerica, che considera forze, constraint, collisioni ecc per calcolare posizioni e rotazioni dei corpi rigidi
        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        this->solver = new btSequentialImpulseConstraintSolver;

        //  DynamicsWorld è la classe principale della simulazione fisica
        this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration); // @suppress("Abstract class cannot be instantiated")

        // imposto la forza di gravità
        this->dynamicsWorld->setGravity(btVector3(0,-9.82,0));
    }

    //////////////////////////////////////////   
    // Metodo per creare un corpo rigido, dotato di una Collision Shape di tipo Box o Sphere
   //// l'utilizzo di una libreria fisica comporta la necessità di definire un solido di riferimento su cui effettuare i calcoli di simulazione fisica, che "approssimi" come forma il modello della mia scena. Una volta calcolata la simulazione, i valori di traslazione e rotazione del solido vengono applicati al modello.
   btRigidBody* createRigidBody(int type, glm::vec3 pos, glm::vec3 size, glm::vec3 rot, float m, float friction , float restitution){

        btCollisionShape* cShape;

        // converto il vettore di glm in un vettore di Bullet
        btVector3 position = btVector3(pos.x,pos.y,pos.z);
        
        // imposto un quaternione dati gli angolo di Eulero passati come parametro
        btQuaternion rotation;
        rotation.setEuler(rot.x,rot.y,rot.z);

        // Box
        if (type == 0) {
            // converto il vettore di glm in un vettore di Bullet
            btVector3 dim = btVector3(size.x,size.y,size.z);
            // creo una BoxShape
            cShape = new btBoxShape(dim);
        }
        // Sphere (in questo caso prendo solo la prima componente di size, che contiene il raggio)
        else if (type == 1) {
            	cShape = new btSphereShape(size.x);
        	} // Cylinder
        	else if (type == 2){
        			btVector3 dim = btVector3(size.x, size.y, size.z);

        			btTransform localTransform;
        			localTransform.setIdentity();
        			localTransform.setOrigin(btVector3(0.0, 0.1, 0.0));

        			btCylinderShape* cylinder = new btCylinderShape(dim);

        			btCompoundShape* shape = new btCompoundShape();
        			shape->addChildShape(localTransform, cylinder);
        			cShape = shape;

//        			cShape = new btCylinderShape(dim);
        		}

        // aggiungo la Collision Shape alla lista
        this->collisionShapes.push_back(cShape);

        // Imposto le trasformazioni iniziali
        btTransform objTransform;
        objTransform.setIdentity();
        objTransform.setRotation(rotation);
        // imposto la posizione iniziale (deve coincidere con la posizione della mesh corrispondente)
        objTransform.setOrigin(position);

        // se l'oggetto ha massa 0, allora è statico (non si muove e non è soggetto a forze)
        btScalar mass = m;
        bool isDynamic = (mass != 0.0f);

        // se è dinamico (massa > 0) allora calcolo l'inerzia locale
        btVector3 localInertia(0.0,0.0,0.0);
        if (isDynamic)
            cShape->calculateLocalInertia(mass,localInertia);

        // inizializzo il Motion State dell'oggetto sulla base della trasformazione create.
        // Tramite il Motion State, la simulazione fisica partirà considerando i parametri di traslazione e rotazione iniziali dell'oggetto definiti dall'utente.        
        btDefaultMotionState* motionState = new btDefaultMotionState(objTransform);

        // imposto la struttura dati per creare il corpo rigido
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,cShape,localInertia);
        // imposto valori di frizione e restituzione
        rbInfo.m_friction = friction;
        rbInfo.m_restitution = restitution;

        // se è una sfera
        if (type == 1){
            // nella simulazione fisica, la sfera tocca il piano solo in un punto, e questo impedisce la corretta applicazione della frizione tra piano e sfera.
            // La rolling friction aggira il problema, e la unisco quindi a un fattore di damping angolare (che quindi applica una forza di resistenza durante la rotazione), in modo da far fermare la sfera dopo un po' di tempo.
            rbInfo.m_angularDamping = 0.4;
            rbInfo.m_rollingFriction = 0.4;
            rbInfo.m_linearDamping = 0.3;
        } else if (type == 2){
        		rbInfo.m_angularDamping = 0.05;
        	    rbInfo.m_rollingFriction = 0.04;
        }

        // creo il corpo rigido
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        this->dynamicsWorld->addRigidBody(body);

        // la funzione ritorna un puntatore al corpo rigido creato.
        // nel loop di simulazione standard, non è essenziale fare riferimento ai singoli corpo rigidi,
        // ma in alcuni casi (come nel caso dell'applicazione di un impulso), è necessario avere questo puntatore
        return body;
    }

    //////////////////////////////////////////       
    // Cancella dati della simulazione fisica in uscita dal programma
    void Clear()
    {
        //remove the rigidbodies from the dynamics world and delete them
        for (int i=this->dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
        {
            // cancella tutti i MotionState e i CollisionObjects
            btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            this->dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }

        //delete collision shapes
        for (int j=0;j<this->collisionShapes.size();j++)
        {
            btCollisionShape* shape = this->collisionShapes[j];
            this->collisionShapes[j] = 0;
            delete shape;
        }

        //delete dynamics world
        delete this->dynamicsWorld;

        //delete solver
        delete this->solver;

        //delete broadphase
        delete this->overlappingPairCache;

        //delete dispatcher
        delete this->dispatcher;

        delete this->collisionConfiguration;

        this->collisionShapes.clear();
    }
};
