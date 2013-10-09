/**
* @author	Ayan Ray
* @date		February 17th, 2008
*/
class Object3D
{
	public:
		Vertex p1, p2, p3; // Experimental: using currently for camera - p1 center, p2 look at, p3 normal
		float rotationX, rotationY, rotationZ;
		bool visible;

	public:
		bool hitTestObject( Object3D obj ); // functionality coming soon

		// Getters and Setters for center location. P1 will be private... 
		void x ( float val );
		void y ( float val );
		void z ( float val );
		float x ();
		float y ();
		float z ();

		void pitch( float val );
		void yaw( float val );
		void roll( float val );

		void forward( float val );
		void backward( float val );
		void left( float val );
		void right( float val );

	//	virtual void render();
};

//////////////////////////////////////////////////////////////////////// TRANSFORMATIONS

void Object3D::left( float val )
{
	p1.x -= float(cos((rotationY / 180 * PI))) * val;
	p1.z -= float(sin((rotationY / 180 * PI))) * val;
}

void Object3D::right( float val )
{
	p1.x += float(cos(rotationY / 180 * PI)) * val;
	p1.z += float(sin(rotationY / 180 * PI)) * val;
}

void Object3D::forward( float val )
{
	p1.x += float( sin(rotationY / 180 * PI) ) * val;
	p1.y -= float( sin(rotationX / 180 * PI) ) * val;
	p1.z -= float( cos(rotationY / 180 * PI) ) * val;
}

void Object3D::backward( float val )
{
	p1.x -= float( sin(rotationY / 180 * PI) ) * val;
	p1.y += float( sin(rotationZ / 180 * PI) ) * val;
	p1.z += float( cos(rotationY / 180 * PI) ) * val;
}

void Object3D::pitch( float val )
{
	rotationX += val;
	if (rotationX > 360) rotationX -= 360;
	else if (rotationX < -360) rotationX += 360;
}

void Object3D::yaw( float val )
{
	rotationY += val;
	if (rotationY > 360) rotationY -= 360;
	else if (rotationY < -360) rotationY += 360;
}

void Object3D::roll( float val )
{
	rotationZ += val;
	if (rotationZ > 360) rotationZ -= 360;
	else if (rotationZ < -360) rotationZ += 360;
}
