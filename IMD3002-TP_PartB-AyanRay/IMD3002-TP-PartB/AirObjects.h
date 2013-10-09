//#include "xEnginePrimitives.h"
#include "ObjLoader.h"

class AirObject3D: public Object3D
{
public:
	float speed;
	float maxSpeed;
	float minSpeed;
	float damage;

	public:
		AirObject3D(); 
		//void render( void );
};

class Bullet3D: public AirObject3D
{
	public:
		Bullet3D(); 
		void render( void );
};

void Bullet3D::render(void )
{

}
class Airplane3D: public AirObject3D
{
public:
	float ammo;
	float missles;
	ModelType obj;

	public:
		Airplane3D();
		void load(); 
		void load(ModelType objx); 
		void render( void );
};

AirObject3D::AirObject3D()
{

}

Airplane3D::Airplane3D()
{
	maxSpeed = 0.6;
	minSpeed = 0.0;
	speed = 0.0;
	ammo = 1000;
	missles = 4;
}
void Airplane3D::load()
{
	obj.LoadObj("Plane.obj", 0.2f);
}
void Airplane3D::load(ModelType objx)
{
	obj = objx;
}

void Airplane3D::render()
{
	obj.Draw();
}