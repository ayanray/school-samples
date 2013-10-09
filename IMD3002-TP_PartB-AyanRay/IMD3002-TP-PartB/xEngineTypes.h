/**
* @author	Ayan Ray
* @date		February 17th, 2008
* @description This file defines structured data types for 3D computation.
*/
struct Vertex
{
	float x, y, z;
};

struct Color
{
	float r, g, b;
};

struct Vector
{
	Vertex p1, p2;
};

struct Transformation
{
	float	x1, y1, z1, w1,
			x2, y2, z2, w2,
			x3, y3, z3, w3,
			x4, y4, z4, w4;
};