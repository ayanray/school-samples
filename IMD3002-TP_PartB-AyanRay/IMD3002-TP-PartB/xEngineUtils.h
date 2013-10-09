/**
* @author	Ayan Ray
* @date		February 17th, 2008
* @description Provides utility tools for common vector math
*/
#define PI 3.1415926535897932384626433832795

// Algebra
float CalculateDotProduct( Vector * va, Vector * vb);
float CalculateMagnitude ( Vector * v );
float CalculateAngleBetween( Vector * va, Vector * vb );
void CalculateCrossProduct( Vector * va, Vector * vb, Vector * vc );

// Transformations
void DoVertexTransformation( Vertex &p, Transformation * t );

///////////////////////////////////////////////////////////////  Math Functions

/**
* Calculates the Dot product of any two vectors
*/
float CalculateDotProduct( Vector * va, Vector * vb)
{
	return (	(va->p2.x - va->p1.x) * (vb->p2.x - vb->p1.x) +
				(va->p2.y - va->p1.y) * (vb->p2.y - vb->p1.y) +
				(va->p2.z - va->p1.z) * (vb->p2.z - vb->p1.z) );
}
/**
* Calculates the magnitude of a vector (length)
*/
float CalculateMagnitude ( Vector * v )
{
	return float( sqrt( pow( v->p2.x - v->p1.x, 2) + pow( v->p2.y - v->p1.y, 2) + pow( v->p2.z - v->p1.z, 2) ) );
}
/**
* Calculates the Angle between any two vectors
*/
float CalculateAngleBetween( Vector *va, Vector *vb )
{
	return acos( CalculateDotProduct( va, vb ) / ( CalculateMagnitude( va ) * CalculateMagnitude( vb ) ) ) * 180 / PI;
}
/**
* Calculates the perpendicular vector of two vectors (the cross product)
*/
void CalculateCrossProduct( Vector *va, Vector *vb, Vector *vc )
{
	vc->p2.x = (va->p2.y - va->p1.y) * (vb->p2.z - vb->p1.z) - (va->p2.z - va->p1.z) * (vb->p2.y - vb->p1.y);
	vc->p2.y = (va->p2.z - va->p1.z) * (vb->p2.x - vb->p1.x) - (va->p2.x - va->p1.x) * (vb->p2.z - vb->p1.z);
	vc->p2.z = (va->p2.x - va->p1.x) * (vb->p2.y - vb->p1.y) - (va->p2.y - va->p1.y) * (vb->p2.x - vb->p1.x);
}

///////////////////////////////////////////////////////////////  Transformation Functions

/**
* Does a matrix transformation on a vertex
*/
void DoVertexTransformation( Vertex &p, Transformation * t )
{
	// TODO: Might change struct Vertex into a four variable position... maybe.
	p.x = t->x1*p.x + t->y1*p.y + t->z1*p.z + t->w1*1;
	p.y = t->x2*p.x + t->y2*p.y + t->z2*p.z + t->w2*1;
	p.z = t->x3*p.x + t->y3*p.y + t->z3*p.z + t->w3*1;
}

//Returns a random float from 0 to < 1
float randomFloat() {
	return (float)rand() / ((float)RAND_MAX + 1);
}