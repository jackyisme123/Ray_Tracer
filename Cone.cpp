/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray (pos, dir). 
*/
float Cone::intersect(glm::vec3 posn, glm::vec3 dir)
{
    float theta = radius / height;
    float a = pow(dir.x, 2) + pow(dir.z, 2) - pow(tan(theta) * dir.y, 2);
    float b = 2 * (dir.x * (posn.x - center.x) + dir.z * (posn.z - center.z) - pow(tan(theta), 2) * dir.y * (posn.y - center.y - height));
    float c = pow(posn.x - center.x, 2) + pow(posn.z - center.z, 2) - pow(tan(theta) * (posn.y - center.y - height), 2);
    float delta = pow(b, 2) - 4 * a * c;
    if (delta < 0.0 or fabs(delta) < 1e-3) return -1.0;
    float t1 = (-b + sqrt(delta)) / (2 * a);
    float t2 = (-b - sqrt(delta)) / (2 * a);
    if (fabs(t1) < 1e-3) {
        if (t2 > 0){
            return t2;
        }else{
            t1 = -1;
        }
    }
    if (fabs(t2) < 1e-3) {
        t2 = -1;
    }
    float min = t1 < t2 ? t1 : t2;
    float max = t1 > t2 ? t1 : t2;
    float y1 = (posn.y + min * dir.y) - center.y;
	float y2 = (posn.y + max * dir.y) - center.y;
	
	if(not (y1 < 0 || y1 > height) && min != -1){
		return min;
	}
	
	else if(not (y2 < 0 || y2 > height) && max != -1){
		return max;
	}
	else{
		return -1;
	}
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    float alpha = atan((p.x - center.x) / (p.z - center.z));
    float theta = radius / height;
    glm::vec3 n = glm::vec3 (sin(alpha) * cos(theta), sin(theta), cos(alpha) * sin(theta));
    //~ glm::vec3 n = glm::vec3 (p.x- center.x, 0, p.z - center.z);
    return n;
}
