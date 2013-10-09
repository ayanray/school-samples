/**
* @author	Ayan Ray
* @date		February 17th, 2008
*/
class Camera3D: public Object3D
{
	public:
		float focus, zoom; // will be used later

	public:
		Camera3D();
};

Camera3D::Camera3D ()
{
	focus = zoom = 1;
	p2.x = p2.y = p2.z = 0;
	p3.x = 0;
	p3.y = 1.0;
	p3.z = 0;

	rotationX = rotationY = rotationZ = 0;
}