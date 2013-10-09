/**
* @author		Ayan Ray
* @date			February 29th, 2008
* @description	Provides a framework for AI controlled airplanes
*/
class AIAirplane
{
private:
	bool checkBoundary(Airplane3D * plane);
public: 
	void autoPilot(Airplane3D * plane);
};

/**
* Currently does only simple return to start location... this isnt real AI
*/
void AIAirplane::autoPilot(Airplane3D *plane)
{
	if(!checkBoundary(plane))
	{
		//plane->yaw(1);
		plane->p1.z = 200;
	}
	plane->forward(0.3);
}

bool AIAirplane::checkBoundary(Airplane3D * plane)
{
	//printf("PLANE: %f %f %f/ rot: %f %f %f\n", plane->p1.x, plane->p1.y, plane->p1.z, plane->rotationX , plane->rotationY, plane->rotationZ);
	if(plane->p1.z < -200 || plane->p1.z > 200)
		return false;
	else if(plane->p1.x < -200 || plane->p1.x > 200)
		return false;

	return true;
}
