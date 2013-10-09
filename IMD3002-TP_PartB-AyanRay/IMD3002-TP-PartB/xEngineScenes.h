/**
* @author	Ayan Ray
* @date		February 17th, 2008
*/
#include <GL/gl.h>
#include <GL/glut.h>
#include <vector>

class Scene3D
{
private:
	GLuint displayList; // OpenGL Display List
	//Object3D * displayObjects[5];
	//vector<Object3D*> _displayList;
	int numChildren;

public:
	void addChild( Object3D * obj );
	void preRender();
	void renderCamera( Camera3D *  camera); // DISTANCE is added as a hack, this will be fixed in later versions.
};

void Scene3D::addChild( Object3D *  obj )
{
	//displayObjects[0] = obj;
	displayList = glGenLists(1); 
	glNewList(displayList,GL_COMPILE); //compile the new list
	
	//obj->render();

	glEndList(); //end the list
}

/**
* Pre-render so you can decide if its first person or third person game
*/
void Scene3D::preRender()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
/**
* Render Camera's DisplayList
*/
void Scene3D::renderCamera( Camera3D *  camera)
{
	glRotatef(camera->rotationX,1.0,0.0,0.0);
	glRotatef(camera->rotationY,0.0,1.0,0.0);
	glRotatef(camera->rotationZ,0.0,0.0,1.0); 

	glTranslated( -camera->p1.x, -camera->p1.y, -camera->p1.z );

	//gluLookAt(camera->p1.x, camera->p1.y, camera->p1.z, camera->p2.x, camera->p2.y, camera->p2.z, 0.0, 1.0, 0.0);

	// Display List
	//glCallList(displayList);
}