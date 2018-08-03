/*========================================================================
* COSC 363  Computer Graphics (2018)
* Ray tracer 
* Writer : Yuan Cui
* User Id: ycu20
* Id No.: 63483319
* 
* Build Command: g++ -Wall -o "%e" "%f" Sphere.cpp SceneObject.cpp Ray.cpp Plane.cpp Cylinder.cpp Cone.cpp TextureBMP.cpp -lGL -lGLU -lglut
* 
*=========================================================================
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/glut.h>
#include "Plane.h"
#include "TextureBMP.h"
#include "Cylinder.h"
#include "Cone.h"
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene
TextureBMP texture;
TextureBMP texture1;
TextureBMP texture2;

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{    
    
    int f_value = 30; // reflect point size
    glm::vec3 specCol;
    glm::vec3 specCol1;
    glm::vec3 sumCol;
	glm::vec3 backgroundCol(0);
	glm::vec3 light(50, 20, -10);
    glm::vec3 light1(-50, 40, -20);
	glm::vec3 ambientCol(0.2);   //Ambient color of light
    glm::vec3 red(1, 0, 0);
    glm::vec3 green(0, 1, 0);
    glm::vec3 blue(0, 0, 1);
    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray
    
    
    if(ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour
    
    glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor(); //else return object's colour
    glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);
    glm::vec3 lightVector = light - ray.xpt;
    glm::vec3 lightVector1 = light1 - ray.xpt;    
    glm::vec3 lightVector_n = glm::normalize(lightVector);
    glm::vec3 lightVector1_n = glm::normalize(lightVector1);
    glm::vec3 reflVector = glm::reflect(-lightVector_n, normalVector);
    glm::vec3 reflVector1 = glm::reflect(-lightVector1_n, normalVector);    
    Ray shadow(ray.xpt, lightVector_n);
    Ray shadow1(ray.xpt, lightVector1_n);
    shadow.closestPt(sceneObjects);
    shadow1.closestPt(sceneObjects);
    
    float lDotn = glm::dot(lightVector_n, normalVector);       
    float lDotn1 = glm::dot(lightVector1_n, normalVector);  
    float rDotv = glm::dot(reflVector, -ray.dir);
    float rDotv1 = glm::dot(reflVector1, -ray.dir);
    // no specular color from floor
    specCol = rDotv >= 0 && ray.xindex !=3 && ray.xindex !=8? glm::vec3(1, 1, 1) * pow(rDotv, f_value) : glm::vec3(0, 0, 0); 
    specCol1 = rDotv1 >= 0  && ray.xindex !=3 && ray.xindex !=8? glm::vec3(1, 1, 1) * pow(rDotv1, f_value) : glm::vec3(0, 0, 0);    
    sumCol = (lDotn <= 0 || shadow.xindex > -1 || shadow.xdist >= glm::length(lightVector)) ? ambientCol * materialCol : ambientCol * materialCol + lDotn * materialCol + specCol;
    sumCol = (lDotn1 <= 0 || shadow1.xindex > -1 || shadow1.xdist >= glm::length(lightVector1)) ? ambientCol * materialCol : ambientCol * materialCol + lDotn1 * sumCol + specCol1;
        
    //reflection
    if(ray.xindex == 0 && step < MAX_STEPS)
    {
        glm::vec3 refl = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, refl);
        glm::vec3 reflCol = trace(reflectedRay, step+1);
        sumCol = sumCol + (0.6f * reflCol);
    }    
    
    //texture on sphere
    if(ray.xindex == 1)
    {
        float a = asin(normalVector.x) / M_PI + 0.5;
        float b = asin(normalVector.y) / M_PI + 0.5;
        sumCol = texture1.getColorAt(a, b)+ specCol + specCol1;
    }
    
    //refraction
    float rgb = 0.2; // refraction color RGB
    if(ray.xindex == 2 && step < MAX_STEPS)
    {
        float eta1 = 1/1.2;
        glm::vec3 n = normalVector;
        glm::vec3 d = ray.dir;
        glm::vec3 g = glm::refract(d, n ,eta1);
        Ray refrRay1(ray.xpt, g);
        refrRay1.closestPt(sceneObjects);
        if(refrRay1.xindex == -1) return backgroundCol;
        glm::vec3 m = sceneObjects[refrRay1.xindex] -> normal(refrRay1.xpt);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta1);
        Ray refrRay2(refrRay1.xpt, h);
        refrRay2.closestPt(sceneObjects);
        if(refrRay2.xindex == -1) return backgroundCol;
        glm::vec3 refrCol = trace(refrRay2, step+1);
        sumCol = sumCol * rgb +  refrCol * (1- rgb);
    }  
      
    // texutre on board
    if(ray.xindex == 6)
    {
        float texcoords = (ray.xpt.x-40)/(-80);
        float texcoordt = (ray.xpt.y+20)/(40);
        sumCol = texture.getColorAt(texcoords, texcoordt);
    }
    //transparent
    if(ray.xindex == 7 && step < MAX_STEPS)
    {
        float eta2 = 1/1.005;
        glm::vec3 n = normalVector;
        glm::vec3 d = ray.dir;
        glm::vec3 g = glm::refract(d, n ,eta2);
        Ray refrRay1(ray.xpt, g);
        refrRay1.closestPt(sceneObjects);
        if(refrRay1.xindex == -1) return backgroundCol;
        glm::vec3 m = sceneObjects[refrRay1.xindex] -> normal(refrRay1.xpt);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta2);
        Ray refrRay2(refrRay1.xpt, h);
        refrRay2.closestPt(sceneObjects);
        if(refrRay2.xindex == -1) return backgroundCol;
        glm::vec3 refrCol = trace(refrRay2, step+1);
        sumCol = sumCol * rgb +  refrCol * (1- rgb);
        
        glm::vec3 refl1 = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay1(ray.xpt, refl1);
        glm::vec3 reflCol1 = trace(reflectedRay1, step+1);
        sumCol = sumCol*0.6f + (0.6f * reflCol1);
    } 
    
    if(ray.xindex == 8)
    {
        if (ray.xpt.y >= -13 and ray.xpt.y < -11) sumCol += red * 0.8f;
        if (ray.xpt.y >= -11 and ray.xpt.y < -9) sumCol += green * 0.8f;
        if (ray.xpt.y >= -9 and ray.xpt.y <= -7) sumCol += blue * 0.8f;
    } 
    
    return sumCol;
    
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
float eye_x,eye_z,lookAngle;
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height

	glm::vec3 eye(eye_x, 0., eye_z);  //The eye position (source of primary rays) is the origin

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a quad.

	for(int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;
            //With Anti-Aliasing
		    glm::vec3 dir1(xp+0.25*cellX, yp+0.25*cellY, -EDIST);	//direction of the primary ray
            glm::vec3 dir2(xp+0.75*cellX, yp+0.75*cellY, -EDIST);
            glm::vec3 dir3(xp+0.25*cellX, yp+0.75*cellY, -EDIST);
            glm::vec3 dir4(xp+0.75*cellX, yp+0.75*cellY, -EDIST);
		    Ray ray1 = Ray(eye, dir1);		//Create a ray originating from the camera in the direction 'dir'
            Ray ray2 = Ray(eye, dir2);
            Ray ray3 = Ray(eye, dir3);
            Ray ray4 = Ray(eye, dir4);
			ray1.normalize();				//Normalize the direction of the ray to a unit vector
            ray2.normalize();
            ray3.normalize();
            ray4.normalize();
		    glm::vec3 col1 = trace (ray1, 1); //Trace the primary ray and get the colour value
            glm::vec3 col2 = trace (ray2, 1);
            glm::vec3 col3 = trace (ray3, 1);
            glm::vec3 col4 = trace (ray4, 1);
            float r = (col1.r + col2.r + col3.r + col4.r) / 4;
            float g = (col1.g + col2.g + col3.g + col4.g) / 4;
            float b = (col1.b + col2.b + col3.b + col4.b) / 4;
			glColor3f(r, g, b);
            //Without Anti-Aliasing
            //~ glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);
            //~ Ray ray = Ray(eye, dir);
            //~ ray.normalize();
            //~ glm::vec3 col = trace (ray, 1); 
            //~ glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}


//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

	//-- Create a pointer to a sphere object
	Sphere *sphere1 = new Sphere(glm::vec3(-5.0, -5.0, -150.0), 15.0, glm::vec3(169 * 1.0 / 256, 169 * 1.0 / 256, 169 * 1.0 / 256));
    Sphere *sphere2 = new Sphere(glm::vec3(-4.0, -5.0, -30), 1.0, glm::vec3(1, 0, 0));
    Sphere *sphere3 = new Sphere(glm::vec3(10.5, -15.0, -120.0), 4.0, glm::vec3(1, 1, 1));
    Sphere *sphere4 = new Sphere(glm::vec3(5, -5, -90.0), 3.0, glm::vec3(0, 0, 1));
    Sphere *sphere5 = new Sphere(glm::vec3(-17, -10, -85.0), 3.0, glm::vec3(1, 1, 1));
    Plane *plane1 = new Plane(glm::vec3(-40, -20, -40), glm::vec3(40, -20, -40), glm::vec3(40, -20, -200), glm::vec3(-40, -20, -200), glm::vec3(249*1.0/255, 135*1.0/255, 197*1.0/255));
    Plane *plane2 = new Plane(glm::vec3(40, -20, -200), glm::vec3(-40, -20, -200), glm::vec3(-40, 40, -200), glm::vec3(40, 40, -200), glm::vec3(0, 0, 0));
    Cylinder *cylinder = new Cylinder(glm::vec3(-15, -15, -70), 2, 5, glm::vec3(1, 0, 0));
    Cone *cone = new Cone(glm::vec3(-5, -20, -90), 2, 5, glm::vec3(1, 0, 0));
    Plane *cube1 = new Plane(glm::vec3(25, -19, -130), glm::vec3(30, -19, -130), glm::vec3(30, -19, -135), glm::vec3(25, -19, -135), glm::vec3(0, 0, 0));
    Plane *cube2 = new Plane(glm::vec3(25, -14, -130), glm::vec3(30, -14, -130), glm::vec3(30, -19, -130), glm::vec3(25, -19, -130), glm::vec3(0, 1, 0));
    Plane *cube3 = new Plane(glm::vec3(30, -14, -130), glm::vec3(30, -14, -135), glm::vec3(30, -19, -135), glm::vec3(30, -19, -130), glm::vec3(1, 1, 1));
    Plane *cube4 = new Plane(glm::vec3(25, -14, -135), glm::vec3(25, -19, -135), glm::vec3(30, -19, -135), glm::vec3(30, -14, -135), glm::vec3(0, 0, 1));
    Plane *cube5 = new Plane(glm::vec3(25, -14, -130), glm::vec3(25, -19, -130), glm::vec3(25, -19, -135), glm::vec3(25, -14, -135), glm::vec3(1, 0, 0));
    Plane *cube6 = new Plane(glm::vec3(25, -14, -130), glm::vec3(30, -14, -130), glm::vec3(30, -14, -135), glm::vec3(25, -14, -135), glm::vec3(0, 0, 1));
    Plane *frustum1 = new Plane(glm::vec3(5, -19, -80), glm::vec3(15, -19, -80), glm::vec3(15, -19, -90), glm::vec3(5, -19, -90), glm::vec3(0, 1, 0));
    Plane *frustum2 = new Plane(glm::vec3(15, -19, -80), glm::vec3(15, -19, -90), glm::vec3(12, -14, -88), glm::vec3(12, -14, -82), glm::vec3(0, 1, 0));
    Plane *frustum3 = new Plane(glm::vec3(15, -19, -90), glm::vec3(5, -19, -90), glm::vec3(8, -14, -88), glm::vec3(12, -14, -88), glm::vec3(0, 1, 0));
    Plane *frustum4 = new Plane(glm::vec3(5, -19, -90), glm::vec3(15, -19, -90), glm::vec3(12, -14, -88), glm::vec3(8, -14, -88), glm::vec3(0, 1, 0));
    Plane *frustum5 = new Plane(glm::vec3(5, -19, -80), glm::vec3(5, -19, -90), glm::vec3(8, -14, -88), glm::vec3(8, -14, -82), glm::vec3(0, 1, 0));
    Plane *frustum6 = new Plane(glm::vec3(8, -14, -82), glm::vec3(12, -14, -82), glm::vec3(12, -14, -88), glm::vec3(8, -14, -88), glm::vec3(0, 1, 0));
    Plane *tetrahedron1 = new Plane(glm::vec3(-3, -19, -99), glm::vec3(6, -19, -99), glm::vec3(0, -19, -93), glm::vec3(0, -19.0001, -93), glm::vec3(0, 0, 1)); 
    Plane *tetrahedron2 = new Plane(glm::vec3(-3, -19, -99), glm::vec3(6, -19, -99), glm::vec3(0, -15, -96), glm::vec3(0, -15, -96.0001), glm::vec3(0, 0, 1)); 
    Plane *tetrahedron3 = new Plane(glm::vec3(-3, -19, -99), glm::vec3(0, -19, -93), glm::vec3(0, -15, -96), glm::vec3(0, -15, -96.0001), glm::vec3(0, 0, 1)); 
    Plane *tetrahedron4 = new Plane(glm::vec3(6, -19, -99), glm::vec3(0, -19.0001, -93), glm::vec3(0, -15, -96), glm::vec3(0, -15, -96.0001), glm::vec3(0, 0, 1));
    Plane *squarepyramid1 = new Plane(glm::vec3(8, -12, -82), glm::vec3(12, -12, -82), glm::vec3(10.0001, -10, -85), glm::vec3(10, -10, -85.0001), glm::vec3(1, 0, 0));
    Plane *squarepyramid2 = new Plane(glm::vec3(12, -12, -82), glm::vec3(12, -12, -88), glm::vec3(10.0001, -10, -85), glm::vec3(10, -10, -85.0001), glm::vec3(1, 0, 0));
    Plane *squarepyramid3 = new Plane(glm::vec3(12, -12, -88), glm::vec3(8, -12, -88), glm::vec3(10.0001, -10, -85), glm::vec3(10, -10, -85.0001), glm::vec3(1, 0, 0));
    Plane *squarepyramid4 = new Plane(glm::vec3(8, -12, -88), glm::vec3(8, -12, -82), glm::vec3(10.0001, -10, -85), glm::vec3(10, -10, -85.0001), glm::vec3(1, 0, 0));
    Plane *squarepyramid5 = new Plane(glm::vec3(8, -12, -82), glm::vec3(12, -12, -82), glm::vec3(12, -12, -88), glm::vec3(8, -12, -88), glm::vec3(1, 0, 0));
	//--Add the above to the list of scene objects.
	sceneObjects.push_back(sphere1); 
    sceneObjects.push_back(sphere2);
    sceneObjects.push_back(sphere3);
    sceneObjects.push_back(plane1);
    sceneObjects.push_back(cylinder);
    sceneObjects.push_back(cone);    
    sceneObjects.push_back(plane2);
    sceneObjects.push_back(sphere4);
    sceneObjects.push_back(sphere5);
    sceneObjects.push_back(cube1); 
    sceneObjects.push_back(cube2);
    sceneObjects.push_back(cube3);
    sceneObjects.push_back(cube4);
    sceneObjects.push_back(cube5);
    sceneObjects.push_back(cube6);   
    sceneObjects.push_back(frustum1);
    sceneObjects.push_back(frustum2);
    sceneObjects.push_back(frustum3);
    sceneObjects.push_back(frustum4);
    sceneObjects.push_back(frustum5);
    sceneObjects.push_back(frustum6);  
    sceneObjects.push_back(tetrahedron1); 
    sceneObjects.push_back(tetrahedron2);
    sceneObjects.push_back(tetrahedron3);
    sceneObjects.push_back(tetrahedron4);
    sceneObjects.push_back(squarepyramid1);
    sceneObjects.push_back(squarepyramid2);
    sceneObjects.push_back(squarepyramid3);
    sceneObjects.push_back(squarepyramid4);
    sceneObjects.push_back(squarepyramid5);
    char* fileName1 = (char *) "Earth.bmp";
    char* fileName2 = (char *) "Sun.bmp";
    char* fileName3 = (char *) "door.bmp";
    texture = TextureBMP(fileName1);
    texture2 = TextureBMP(fileName2);
    texture1 = TextureBMP(fileName3);
    
}


 void special(int key, int x, int y)
 {
	if(key == GLUT_KEY_LEFT) lookAngle -= 0.1;  //Change direction
	else if(key == GLUT_KEY_RIGHT) lookAngle += 0.1;
	else if(key == GLUT_KEY_DOWN)
	{ //Move backward
		eye_x -= 5*sin(lookAngle);
		eye_z += 5*cos(lookAngle);
	}
	else if(key == GLUT_KEY_UP)
	{ //Move forward
		eye_x += 5*sin(lookAngle);
		eye_z -= 5*cos(lookAngle);
	}
    //shift view

    
	glutPostRedisplay();
}
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");
    
    glutDisplayFunc(display);
    glutSpecialFunc(special);
    initialize();

    glutMainLoop();
    return 0;
}
