/**
* @author	Victor Rucareanu
* @date		February 29th, 2008
* Originally adapted from www.videotutorialsrock.com
*/
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <vector>

#include "vec3f.h"

using namespace std;

//Constants
float _timeUntilUpdate = 0; //The amount of time until performUpdate should be called
const int MAX_OCTREE_DEPTH = 6; //Octree Depth
const int MIN_OBJECTS_PER_OCTREE = 3; //Minimum objects before octree creates new leaf node
const int MAX_OBJECTS_PER_OCTREE = 100; //Maximum objects before octree creatues new leaf node 
const float BOX_SIZE = 0.5f; //The length of one side of the environment
const float TIME_BETWEEN_UPDATES = 0.01f; //Time between redraw
const int TIMER_MS = 25; //The number of milliseconds to which the timer is set

//Stores information regarding a object
struct Object 
{
	Vec3f v; //Velocity
	Vec3f pos; //Position
	float r; //Radius
	float damage;
	Vec3f color;
	int type; //1 = Plane 2 = Bullet 3 = Missle
};

//Stores a pair of objects
struct ObjectPair {
	Object* object1;
	Object* object2;
};
class Octree 
{
	private:
		Vec3f corner1; //(minX, minY, minZ)
		Vec3f corner2; //(maxX, maxY, maxZ)
		Vec3f center;//((minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2)
		/* The children of this, if this has any.  children[0][*][*] are the
		 * children with x coordinates ranging from minX to centerX.
		 * children[1][*][*] are the children with x coordinates ranging from
		 * centerX to maxX.  Similarly for the other two dimensions of the
		 * children array.
		 */
		Octree *children[2][2][2];
		//Whether this has children
		bool hasChildren;
		//The objects in this, if this doesn't have any children
		set<Object*> objects;
		//The depth of this in the tree
		int depth;
		//The number of objects in this, including those stored in its children
		int numObjects;

		void fileObject(Object* object, Vec3f pos, bool addObject);
		void haveChildren();
		void collectObjects(set<Object*> &bs);
		void destroyChildren();	
		void remove(Object* object, Vec3f pos);
		
	public:
		void potentialObjectObjectCollisions(vector<ObjectPair> &collisions);
		void objectMoved(Object* object, Vec3f oldPos);
		void remove(Object* object);
		void add(Object* object);
		Octree(Vec3f c1, Vec3f c2, int d);
		~Octree();
};

//Adds a object to or removes one from the children of this
void Octree::fileObject(Object* object, Vec3f pos, bool addObject) 
{
	//Figure out in which child(ren) the object belongs
	for(int x = 0; x < 2; x++) 
	{
		if (x == 0) {
			if (pos[0] - object->r > center[0]) {
				continue;
			}
		}
		else if (pos[0] + object->r < center[0]) {
			continue;
		}
		
		for(int y = 0; y < 2; y++) {
			if (y == 0) {
				if (pos[1] - object->r > center[1]) {
					continue;
				}
			}
			else if (pos[1] + object->r < center[1]) {
				continue;
			}
			
			for(int z = 0; z < 2; z++) {
				if (z == 0) {
					if (pos[2] - object->r > center[2]) {
						continue;
					}
				}
				else if (pos[2] + object->r < center[2]) {
					continue;
				}
				
				//Add or remove the object
				if (addObject) {
					children[x][y][z]->add(object);
				}
				else {
					children[x][y][z]->remove(object, pos);
				}
			}
		}
	}
}

//Creates children of this, and moves the objects in this to the children
void Octree::haveChildren() 
{
	for(int x = 0; x < 2; x++) {
		float minX;
		float maxX;
		if (x == 0) {
			minX = corner1[0];
			maxX = center[0];
		}
		else {
			minX = center[0];
			maxX = corner2[0];
		}
		
		for(int y = 0; y < 2; y++) {
			float minY;
			float maxY;
			if (y == 0) {
				minY = corner1[1];
				maxY = center[1];
			}
			else {
				minY = center[1];
				maxY = corner2[1];
			}
			
			for(int z = 0; z < 2; z++) {
				float minZ;
				float maxZ;
				if (z == 0) {
					minZ = corner1[2];
					maxZ = center[2];
				}
				else {
					minZ = center[2];
					maxZ = corner2[2];
				}
				
				children[x][y][z] = new Octree(Vec3f(minX, minY, minZ),
											   Vec3f(maxX, maxY, maxZ),
											   depth + 1);
			}
		}
	}
			
	//Remove all objects from "objects" and add them to the new children
	for(set<Object*>::iterator it = objects.begin(); it != objects.end();
			it++) {
		Object* object = *it;
		fileObject(object, object->pos, true);
	}
	objects.clear();
	
	hasChildren = true;
}

//Adds all objects in this or one of its descendants to the specified set
void Octree::collectObjects(set<Object*> &bs) 
{
	if (hasChildren) {
		for(int x = 0; x < 2; x++) {
			for(int y = 0; y < 2; y++) {
				for(int z = 0; z < 2; z++) {
					children[x][y][z]->collectObjects(bs);
				}
			}
		}
	}
	else {
		for(set<Object*>::iterator it = objects.begin(); it != objects.end();
				it++) {
			Object* object = *it;
			bs.insert(object);
		}
	}
}
		
//Destroys the children of this, and moves all objects in its descendants
//to the "objects" set
void Octree::destroyChildren() {
	//Move all objects in descendants of this to the "objects" set
	collectObjects(objects);
	
	for(int x = 0; x < 2; x++) {
		for(int y = 0; y < 2; y++) {
			for(int z = 0; z < 2; z++) {
				delete children[x][y][z];
			}
		}
	}
	
	hasChildren = false;
}
		
//Removes the specified object at the indicated position
void Octree::remove(Object* object, Vec3f pos) {
	numObjects--;
	
	if (hasChildren && numObjects < MIN_OBJECTS_PER_OCTREE) {
		destroyChildren();
	}
	
	if (hasChildren) {
		fileObject(object, pos, false);
	}
	else {
		objects.erase(object);
	}
}

//Constructs a new Octree.  c1 is (minX, minY, minZ), c2 is (maxX, maxY,
//maxZ), and d is the depth, which starts at 1.
Octree::Octree(Vec3f c1, Vec3f c2, int d) {
	corner1 = c1;
	corner2 = c2;
	center = (c1 + c2) / 2;
	depth = d;
	numObjects = 0;
	hasChildren = false;
}
		
//Destructor
Octree::~Octree() {
	if (hasChildren) {
		destroyChildren();
	}
}
		
//Adds a object to this
void Octree::add(Object* object) {
	numObjects++;
	if (!hasChildren && depth < MAX_OCTREE_DEPTH &&
		numObjects > MAX_OBJECTS_PER_OCTREE) {
		haveChildren();
	}
	
	if (hasChildren) {
		fileObject(object, object->pos, true);
	}
	else {
		objects.insert(object);
	}
}
		
//Removes a object from this
void Octree::remove(Object* object) 
{
	remove(object, object->pos);
}
		
//Changes the position of a object in this from oldPos to object->pos
void Octree::objectMoved(Object* object, Vec3f oldPos) 
{
	remove(object, oldPos);
	add(object);
}
		
//Adds potential object-object collisions to the specified set
void Octree::potentialObjectObjectCollisions(vector<ObjectPair> &collisions) {
	if (hasChildren) {
		for(int x = 0; x < 2; x++) {
			for(int y = 0; y < 2; y++) {
				for(int z = 0; z < 2; z++) {
					children[x][y][z]->
						potentialObjectObjectCollisions(collisions);
				}
			}
		}
	}
	else {
		//Add all pairs (object1, object2) from objects
		for(set<Object*>::iterator it = objects.begin(); it != objects.end();
				it++) {
			Object* object1 = *it;
			for(set<Object*>::iterator it2 = objects.begin();
					it2 != objects.end(); it2++) {
				Object* object2 = *it2;
				//This test makes sure that we only add each pair once
				if (object1 < object2) {
					ObjectPair bp;
					bp.object1 = object1;
					bp.object2 = object2;
					collisions.push_back(bp);
				}
			}
		}
	}
}

//Puts potential object-object collisions in potentialCollisions.  It must return
//all actual collisions, but it need not return only actual collisions.
void potentialObjectObjectCollisions(vector<ObjectPair> &potentialCollisions,
								 vector<Object*> &objects, Octree* octree) {

	octree->potentialObjectObjectCollisions(potentialCollisions);
}

//Moves all of the objects by their velocity times dt
void moveObjects(vector<Object*> &objects, Octree* octree, float dt) {
	for(unsigned int i = 0; i < objects.size(); i++) {
		Object* object = objects[i];
		Vec3f oldPos = object->pos;
		object->pos += object->v * dt;
		octree->objectMoved(object, oldPos);
		//printf("%f\n",object->pos[2]);
		if (object->pos[2] >= 100.0){
			octree->remove(object);
		}
	}
}

//Returns whether two objects are colliding
bool testObjectObjectCollision(Object* b1, Object* b2) {
	//Check whether the objects are close enough
	float r = b1->r + b2->r;
	if ((b1->pos - b2->pos).magnitudeSquared() < r * r) {
		//Check whether the objects are moving toward each other
		Vec3f netVelocity = b1->v - b2->v;
		Vec3f displacement = b1->pos - b2->pos;
		return netVelocity.dot(displacement) < 0;
	}
	else
		return false;
}

//Handles all object-object collisions
void handleObjectObjectCollisions(vector<Object*> &objects, Octree* octree) {
	vector<ObjectPair> bps;
	potentialObjectObjectCollisions(bps, objects, octree);
	for(unsigned int i = 0; i < bps.size(); i++) {
		ObjectPair bp = bps[i];
		Object* b1 = bp.object1;
		Object* b2 = bp.object2;
		if (testObjectObjectCollision(b1, b2)) {
			//Make the objects reflect off of each other
			Vec3f displacement = (b1->pos - b2->pos).normalize();
			if (b1->type == 1 && b2->type == 2){
				b1->damage += 0.1;
				b1->color = Vec3f(b1->damage,1.0-b1->damage,0.0);
				if (b1->damage > 1.0){
					b1->v = Vec3f(1.0,-16.0,0.0);
				}else{
					b2->v -= 20 * displacement * b1->v.dot(displacement);
				}
				b2->r = 0.0;
				b2->v -= 2 * displacement * b2->v.dot(displacement);
			}
			else if(b1->type == 2 && b2->type == 1){
				b2->damage += 0.1;
				b2->color = Vec3f(b1->damage,1.0-b1->damage,0.0);
				if (b2->damage > 1.0){
					b2->v = Vec3f(1.0,-16.0,0.0);
				}else{
					b1->v -= 20 * displacement * b1->v.dot(displacement);
				}
				b1->r = 0.0;
				b1->v -= 2 * displacement * b2->v.dot(displacement);
			}
		}
	}
}

//Applies gravity and handles all collisions.  Should be called every
//TIME_BETWEEN_UPDATES seconds.
void performUpdate(vector<Object*> &objects, Octree* octree) {
	handleObjectObjectCollisions(objects, octree);
}

//Advances the state of the objects by t.  timeUntilUpdate is the amount of time
//until the next call to performUpdate.
void advance(vector<Object*> &objects,
			 Octree* octree,
			 float t,
			 float &timeUntilUpdate) {
	while (t > 0) {
		if (timeUntilUpdate <= t) {
			moveObjects(objects, octree, timeUntilUpdate);
			performUpdate(objects, octree);
			t -= timeUntilUpdate;
			timeUntilUpdate = TIME_BETWEEN_UPDATES;
		}
		else {
			moveObjects(objects, octree, t);
			timeUntilUpdate -= t;
			t = 0;
		}
	}
}