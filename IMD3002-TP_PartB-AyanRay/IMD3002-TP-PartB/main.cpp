/**
* @author Ayan Ray and Victor Rucareanu
* @version 1.00
* @description Flight Simulator game
*/
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <windows.h>

// Source: http://www.codeguru.com/forum/archive/index.php/t-329626.html
#define GLUT_DISABLE_ATEXIT_HACK
#include <gl\glut.h>
#include <GL/gl.h>

#include "tga.h"
#include "terrain.h"
#include "xEngine.h"
#include "AirObjects.h" // custom air objects to use with xEngine
#include "AIAirplane.h"

///////////////////////////////////////////////////////////////  Functions

// Camera / Render
void RenderScene(void);

// Resize
void ChangeSize( int width, int height );

// User Input
void KeyPress( unsigned char key, int x, int y );
void OnMouseMove( int x, int y);

void InitScene();
void Cleanup(void);
void UpdateScene( int value );

///////////////////////////////////////////////////////////////  Variables

float SCREEN_WIDTH = 600;
float SCREEN_HEIGHT = 600;

int savedOffsetX, savedOffsetY;
int view = 3;

// Terrain
int terrainDL,iterations = 0,totalIterations = 0;

// 3D Engine Variables 
Camera3D camera;
Scene3D scene;

// Default Airplanes
Airplane3D airplane; 
Airplane3D airplane2;
Airplane3D airplane3;

// Airplane AI Controller
AIAirplane AI1;

// Octree Collision Detection
vector<Object*> _objects; //All of the objects in play
Octree* _octree; //An octree with all af the objects

///////////////////////////////////////////////////////////////  USER INPUT

void KeyPress( unsigned char key, int x, int y ) 
{	
	switch( key )
	{
		case 'w':
			if (view == 1 || view == 3)
			{
				airplane.speed += 0.4;
			}
			else
			{
				camera.p1.y += 0.1;
			}
			break;

		case 's':
			if (view == 1 || view == 3)
			{
				airplane.speed = ((airplane.speed -0.3) < airplane.minSpeed) ? airplane.minSpeed : airplane.speed - 0.3;
			}
			else
			{
				camera.p1.y -= 0.1;
			}
			break;

		case 'd':
			if (view == 1 || view == 3)
			{
				camera.roll(1);
			}
			else
			{
				camera.p1.x += 0.1;
			}
			break;

		case 'a':
			if (view == 1 || view == 3)
			{
				camera.roll(-1);
			}
			else
			{
				camera.p1.x -= 0.1;
			}
			break;

		case '1':
			view = 1;
			break;

		case '3':
			view = 3;
			break;

		case '4':
			camera.p1.x = 0;
			camera.p1.y = 0;
			camera.p1.z = 0;
			camera.rotationX = 0;
			camera.rotationY = 0;
			camera.rotationZ = 0;
			airplane.speed = 0.0;
			view = 4;
			break;

		case 27:
			Cleanup();
			exit(0);
			break;

		case 'e':
			if (view == 4) // Collision Detection - create planes
			{
				for(int i = 0; i < 1; i++) {
					Object* object = new Object();
					object->pos = Vec3f(15*randomFloat()-7,10*randomFloat()-4,(5*randomFloat()-2)-50);
					object->v = Vec3f(randomFloat()/2,randomFloat()/2,randomFloat()/2);
					object->r = 3.0f;
					object->color = Vec3f(0.0,1.0,0.0);
					object->damage = 0.0f;
					object->type = 1;
					_objects.push_back(object);
					_octree->add(object);
				}
			}
			break;

		case ' ':
			if (view == 4)
			{
				Object* object = new Object();
				object->pos = Vec3f(camera.p1.x,camera.p1.y,camera.p1.z);
				object->v = Vec3f(0.0,0.0,-25.0);
				object->r = 0.3f;
				object->color = Vec3f(0.7,0.7,0.7);
				object->damage = 0.0f;
				object->type = 2;
				_objects.push_back(object);
				_octree->add(object);
			}
			break;
	}

	//printf("[KeyPress] %d \n", key);
}

void OnMouseMove( int x, int y)
{
	savedOffsetX = x - SCREEN_WIDTH/2;
	savedOffsetY = y - SCREEN_HEIGHT/2;
}

///////////////////////////////////////////////////////////////  

void InitScene() 
{
	//Seed the random number generator
	srand((unsigned int)time(0));

	// Setup Terrain
	terrainLoadFromImage("terrain.tga",1);
	terrainScale(0,100);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);

	// Create Terrain
	terrainDL = terrainCreateDL(0,0,0);
	
	// Lighting
	GLfloat Ambient[] = {0.3,0.3,0.3,1.0};
	GLfloat Diffuse[] = {1.0,1.0,1.0,1.0};
	GLfloat Specular[] = {1.0,1.0,1.0,1.0};

	// Lights
	glLightfv(GL_LIGHT0,GL_AMBIENT,Ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,Diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);

	// Enable Lighting and Smooth Shading
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel (GL_SMOOTH); 

	airplane.load();
	airplane2.obj = airplane.obj;

	airplane3.rotationY = 90;
	airplane3.p1.z = -100;
	airplane3.p1.y = 10;

	// Octree for Macro Collision Detection
	_octree = new Octree(Vec3f(-BOX_SIZE / 2, -BOX_SIZE / 2, -BOX_SIZE / 2),
						 Vec3f(BOX_SIZE / 2, BOX_SIZE / 2, BOX_SIZE / 2), 1);
}

void ChangeSize ( int width, int height) 
{
	if(height == 0) height = 1;
	float ratio = width/height;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 50.0, ratio, 1.0, 300.0 );
	glViewport( 0, 0, width, height );

	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void UpdateScene( int value )
{
	advance(_objects, _octree, (float)TIMER_MS / 1000.0f, _timeUntilUpdate);
	glutPostRedisplay();
	glutTimerFunc(TIMER_MS, UpdateScene, 0);

	// Movement
	if(airplane.speed > airplane.maxSpeed) airplane.speed = airplane.maxSpeed;
	camera.forward(airplane.speed);

	// Manage Accelleration
	//airplane.speed -= 0.2;
	if(airplane.speed < airplane.minSpeed) airplane.speed = airplane.minSpeed;

	if (view == 1 || view == 3){
		// Edit Camera Rotations
		camera.yaw( savedOffsetX/(SCREEN_WIDTH/2) );
		camera.pitch( savedOffsetY/(SCREEN_HEIGHT/2) );
		if(camera.rotationX <= -90) camera.rotationX = -90; // prevent upside down
		if(camera.rotationX >= 90) camera.rotationX = 90; // prevent upside down

		// Temporary Enemy Pilots
		airplane3.forward(0.3);
		if(airplane3.p1.x > 200) airplane3.p1.x = -200;
	}
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.7f, 0.7f, 1.0f, 1.0f); //background color

	// Pre Render
	scene.preRender();

	// 3rd Person
	if(view == 3)
	{
		airplane.rotationY = -savedOffsetX/SCREEN_WIDTH*15;
		airplane.rotationX = -savedOffsetY/SCREEN_HEIGHT*15;
		glPushMatrix();
			glTranslated( 0.0, 0.0, -20.0f + airplane.speed*2 );
			glRotatef(90.0f,0.0,1.0,0.0);
			glRotatef(airplane.rotationY,0.0,1.0,0.0);
			glRotatef(airplane.rotationX,1.0,0.0,0.0);
			airplane.render();
		glPopMatrix();
	}
	
	// Shooting Mode
	if(view == 4)
	{
		//Crosshair
		glBegin(GL_LINES);
			glColor3f(1.0,0.0,0.0);

			glVertex3f(1.0,0.0,-30);
			glVertex3f(-1.0,0.0,-30);

			glVertex3f(0.0,-1.0,-30);
			glVertex3f(0.0,1.0,-30);
		glEnd();
	}

	// Render Display List
	scene.renderCamera( &camera );

	// Enemy Airplanes
	if(view == 3 || view == 1)
	{
		AI1.autoPilot( &airplane2 );
		glPushMatrix();
			glTranslated( airplane2.p1.x, airplane2.p1.y, airplane2.p1.z );
			glRotatef(90.0f,0.0,1.0,0.0);
			airplane2.render();
		glPopMatrix();

		glPushMatrix();
			glTranslated( airplane3.p1.x, airplane3.p1.y, airplane3.p1.z );
			airplane2.render();
		glPopMatrix();
	}

	//Draw the objects
	for(unsigned int i = 0; i < _objects.size(); i++) 
	{
		Object* object = _objects[i];
		glPushMatrix();
		glTranslatef(object->pos[0], object->pos[1], object->pos[2]);

		if (object->type == 1)
		{
			glColor3f(object->color[0], object->color[1], object->color[2]);
			glPushMatrix();
				glRotatef(90.0,0.0,1.0,0.0);
				airplane.render(); //Draw a plane
			glPopMatrix();
		}
		else if(object->type == 2)
		{
			glColor3f(object->color[0], object->color[1], object->color[2]);
			glutSolidSphere(object->r, 12, 12); //Draw a bullet
		}
		glPopMatrix();
	}

	// Draw Terrain
	glPushMatrix();
		glTranslatef(0.0f, -100.0f, 0.0f);
		glCallList(terrainDL);
	glPopMatrix();
	
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////  Function: Cleanup

void Cleanup(void) 
{
	for(unsigned int i = 0; i < _objects.size(); i++) {
		delete _objects[i];
	}
	delete _octree;
}

///////////////////////////////////////////////////////////////  Function: Main

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutCreateWindow("IMD3002-Term Project Part B - Ayan Ray + Victor Rucareanu");

	glutTimerFunc(TIMER_MS, UpdateScene, 0);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);
	glutReshapeFunc(ChangeSize);

	glutKeyboardFunc( KeyPress );
	
	printf("\nTermProject Part B\n");
	printf("Press Key 1 and 3 for 1st and 3rd person views.\n");
	printf("Press Key 4 for Shooter Mode (Collision Detection test).\n");
	printf("In shooting mode, press the space bar to shoot bullets.\n");
	printf("In shooting mode, press the e key to create enemy airplanes.\n");
	printf("You can use WASD keys for moving\n");
	printf("The mouse is used to determine the tilt and pan\n");
	printf("Have fun.\n");

	// Mouse Functions
	glutMotionFunc(OnMouseMove);
	glutPassiveMotionFunc(OnMouseMove);

	InitScene();
	glutMainLoop();

	return(0);
}