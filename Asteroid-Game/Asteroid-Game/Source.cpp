#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <cmath>


int WIDTH = 1000;
int HEIGHT = 700;

#define DEG2RAD(a) (a * 0.0174532925)

//Camera controller variables
float cameraRadius = 15;
float cameraAngle = 90.0;
bool camera360 = false;
float camy = 0.03;

// Plane Controller variables
float planeX = 0.0;
float planeY = 0.0;
float planeAngX = 0.0;
float planeAngY = 0.0;
float planeAngZ = 0.0;

// All asteroids controller variables
float asteroidScale = 0.03;
float seconds = 0;

// First asteroid controller variables
bool asteroid1 = false;
bool a1switch = false;
float a1t = 0.0;
float a1x = 0.0;
float a1y = 0.0;
float a1z = -20.0;
float a10[2];
float a11[2];
float a12[2];
float a13[2];

GLuint tex;
GLuint tex2;
GLUquadricObj * qobj;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

//// Model Variables
Model_3DS model_plane2;
Model_3DS shield;
Model_3DS model_asteroid;

// Textures
GLTexture tex_ground;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f &v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f &v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 0.0f, float eyeY = 0.0f, float eyeZ = 15.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void sideView() {
		eye.x = 15.0;
		eye.y = 0.0;
		eye.z = 0.0;
	}

	void frontView() {
		eye.x = 0.0;
		eye.y = 0.0;
		eye.z = 15.0;
	}

	void topView() {
		eye.x = 0.0;
		eye.y = 15.0;
		eye.z = 0.8;
	}

	void look() {
		if (camera360) {
			eye.x = cameraRadius* cos(DEG2RAD(cameraAngle));
			eye.y = camy;
			eye.z = cameraRadius* sin(DEG2RAD(cameraAngle));
		}
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;

float* bezier(float t, float* p0, float* p1, float* p2, float* p3)
{
	float res[2];
	res[0] = pow((1 - t), 3)*p0[0] + 3 * t*pow((1 - t), 2)*p1[0] + 3 * pow(t, 2)*(1 - t)*p2[0] + pow(t, 3)*p3[0];
	res[1] = pow((1 - t), 3)*p0[1] + 3 * t*pow((1 - t), 2)*p1[1] + 3 * pow(t, 2)*(1 - t)*p2[1] + pow(t, 3)*p3[1];
	return res;
}

void drawCircle(float x, float y, float z, float inr, float outr) {
	glPushMatrix();
	glTranslatef(x, y, z);
	GLUquadric *quadObj = gluNewQuadric();
	gluDisk(quadObj, inr, outr, 50, 50);
	glPopMatrix();
}

void drawLogo(float x, float y, float z, float r, float g, float b) {
	glColor3f(r, g, b);
	drawCircle(x, y, 0, 13, 15);
	glColor3f(r, g, b);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x - 7, y + 17, z);
	glVertex3f(x + 3, y + 19, z);
	glVertex3f(x, y + 5, z);
	glVertex3f(x + 8, y + 8, z);
	glVertex3f(x - 5, y - 20, z);
	glVertex3f(x - 2, y, z);
	glVertex3f(x - 10, y - 3, z);
	glEnd();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{

	//Loading Model files
	model_plane2.Load("Models/plane2/plane2.3ds");
	shield.Load("Models/shield/shield2/CaptainAmericasShield.3ds");
	model_asteroid.Load("Models/asteroid-3DS.3DS");

	//Loading texture files
	loadBMP(&tex, "textures/space3.bmp", true);
	loadBMP(&tex2, "textures/logo.bmp", true);
}

//=======================================================================
// Keyboard Function
//=======================================================================
void Keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case 'f':
		camera.frontView();
		break;
	case 's':
		camera.sideView();
		break;
	case 't':
		camera.topView();
		break;
	case 'r':
		camera360 = !camera360;
		break;
	}
	glutPostRedisplay();
}

void Special(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_LEFT:
		planeX -= 0.1;
		planeAngY += 0.1;
		planeAngZ += 0.5;
		break;
	case GLUT_KEY_RIGHT:
		planeX += 0.1;
		planeAngY -= 0.1;
		planeAngZ -= 0.5;
		break;
	case GLUT_KEY_UP:
		planeY += 0.1;
		planeAngX += 0.5;
		break;
	case GLUT_KEY_DOWN:
		planeY -= 0.1;
		planeAngX -= 0.5;
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Camera setup function
//=======================================================================
void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(100, 1000 / 700, 0.001, 100);
	//	glOrtho(-500, 500, -350, 350, -800, 800);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void Anim() {
	// object
	// x = 99.14 -> 15.86 3ard = 3
	// y = 68 -> 10.88 ertfa3 = 0.6
	// z = 152.2 -> 24.4 tool = 3.5
	// asteroid
	// x = 12.06 -> 0.36 3ard = 1.2
	// y = 7.63 -> 0.228 ertfa3 = 0.7
	// z = 7.47 -> 0.22  tool = 0.68
	if (
		((-0.6 + a1x >= planeX - 1.5  && -0.6 + a1x <= planeX + 1.5) || (0.6 + a1x >= planeX - 1.5 && 0.6 + a1x <= planeX + 1.5))
		&&
		((-0.35 + a1y >= planeY + -3.3 && -0.35 + a1y <= planeY - 2.7) || (0.35 + a1y >= planeY + -3.3 && 0.35 + a1y <= planeY - 2.7))
		&&
		((-0.34 + a1z >= 8.25 && -0.34 + a1z <= 11.75) || (0.34 + a1z >= 8.25 && 0.34 + a1z <= 11.75))
		)
	{
		asteroid1 = false;
	}

}

void Timer(int value) {
	seconds += 10;
	/************************CAMERA ANIMATION***************************/
	if (camera360) {
		cameraAngle++;
	}

	/***********************FIRST ASTEROID ANIMATION**********************/
	if (seconds >= 100) {
		asteroid1 = true;
	}
	if (asteroid1 == true) {
		printf("%f\n", a1z);
		if (a1z >= 20) {

			printf("%s\n", "d5l");
			a1z = -20;
		}
		else
		{
			a1z += 0.05;
			if (a1t >= 0.995) {
				a1t = 0.0;
			}
			else {
				a1t += 0.005;

				float* b = bezier(a1t, a10, a11, a12, a13);
				a1x = b[0];
				a1y = b[1];
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(10, Timer, 0);
}

void drawNitrous() {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glRotated(90, 0, 1, 0);
	qobj = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, tex2);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluCylinder(qobj, 0.5, 0.5, 2, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslated(0, 0.5, 0);
	glRotated(90, 1, 0, 0);
	qobj = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, tex2);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluCylinder(qobj, 0.2, 0.2, 0.5, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	setupCamera();
	InitLightSource();
	//InitMaterial();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//space box
	glPushMatrix();
	GLUquadricObj * qobj;
	qobj = gluNewQuadric();
	//glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 80, 100, 100);
	gluDeleteQuadric(qobj);

	glPopMatrix();

	/*GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);*/

	// large plane model
	glPushMatrix();
	glTranslated(planeX, planeY, 0);
	glRotated(planeAngX, 1, 0, 0);
	glRotated(planeAngY, 0, 1, 0);
	glRotated(planeAngZ, 0, 0, 1);
	glTranslatef(0, -3, 10);
	glScalef(0.016, 0.016, 0.016);
	model_plane2.Draw();
	glPopMatrix();

	// life plane model
	/*glPushMatrix();
	glTranslatef(0, 0, 10);
	glScalef(0.006, 0.006, 0.006);
	model_plane2.Draw();
	glPopMatrix();*/

	//drawNitrous();


	// sheild
	/*glPushMatrix();
	glColor3f(1, 1, 1);
	glScalef(2, 2, 2);
	glRotatef(90, 1, 0, 0);
	shield.Draw();
	glPopMatrix();*/

	//Asteroid
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(a1x, a1y - 2.2, a1z);
	glScalef(asteroidScale, asteroidScale, asteroidScale);
	model_asteroid.Draw();
	glPopMatrix();

	glColor3f(1, 1, 1);

	glutSwapBuffers();
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(200, 100);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(Keyboard);

	glutSpecialFunc(Special);

	glutIdleFunc(Anim);

	glutTimerFunc(0, Timer, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	// first asteroid bezier
	a10[0] = 0.0;
	a10[1] = 0.0;

	a11[0] = -5.0;
	a11[1] = 3.0;

	a12[0] = 5.0;
	a12[1] = 3.0;

	a13[0] = 0.0;
	a13[1] = 0.0;

	glutMainLoop();
}