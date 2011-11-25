#include <stdio.h>
#include <math.h>

typedef double Point3D[3];


#define LINESIZE		250
#define NEAR_ZERO 0.0001	
	/* if diff bigger>NEAR_ZERO slerp, else lin interp,
	in conv q, emap stuff, if <NEAR_ZERO, assume 0 to min precision err. */


#define ROOT 0
#define JOINT 1
#define END_SITE 2

#define PI 3.14159265358979
#define RadianToDegree 180.0 / PI
#define DegreeToRadian PI / 180.0

