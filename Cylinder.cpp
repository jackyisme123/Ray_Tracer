/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray (pos, dir). 
*/
float Cylinder::intersect(glm::vec3 posn, glm::vec3 dir)
{
    float a = pow(dir.x, 2) + pow(dir.z, 2);
    float b = 2 * (dir.x * (posn.x - center.x) + dir.z * (posn.z - center.z));
    float c = pow(posn.x - center.x, 2) + pow(posn.z - center.z, 2) - pow(radius, 2);
    float delta = pow(b, 2) - 4 * a * c;
    if (delta < 0.0 or fabs(delta) < 1e-3) return -1.0;
    float t1 = (-b + sqrt(delta)) / (2 * a);
    float t2 = (-b - sqrt(delta)) / (2 * a);
    float y1 = posn.y + t1 * dir.y;
    float y2 = posn.y + t2 * dir.y;
    if (y1 >= center.y + height) {
        return y2 >= center.y + height ? -1.0 : (center.y + height - posn.y) / dir.y;
    }else if (y2 <= center.y) {
        return y1 <= center.y ? -1.0 : (center.y - posn.y) / dir.y;
    }else {
        return t2;
    }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3 (p.x- center.x, 0, p.z - center.z);
    n = glm::normalize(n);
    return n;
}
