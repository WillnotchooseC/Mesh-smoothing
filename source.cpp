
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h> // (or others, depending on the system in use)
#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <string>
using namespace std;
#define u -0.3
#define r 0.5
int static iteration = 0;

const float ANGFACT = { 1. };
const float MINSCALE = { 0.00f };
const float SCLFACT = { 0.010f };
const float TFACT = { 0.30f };
bool fullscreen = false;
bool mouseDown = false;
int mouseMode = 0;
int	transformation = 0;	
static int menuExit;
int drawType,mousex, mousey = 0;
int win = 0;

void draw();
void menu(int);

// transformation variables
float xrot = 0.0f;
float yrot = 0.0f;

float scale = 1.0f;
float tx = 0.0f;
float ty = 0.0f;

float xdiff = 0.0f;
float ydiff = 0.0f;



enum transformation
{
	ROTATE,
	SCALE,
	TRANSLATE
};

typedef struct {
  float x;
  float y;
  float z;
}FLT3VECT;

typedef struct {
  int a;
  int b;
  int c;
}INT3VECT;

typedef struct {
  int nv;
  int nf;
  int ne;
  FLT3VECT *vertex;
  INT3VECT *face;
}SurFacemesh;
static vector<set<int>> neighborhoods;


void findNeighborhoods(SurFacemesh *mesh, int a, int b, int c) {
	neighborhoods[a].insert(b);
	neighborhoods[a].insert(c);

	neighborhoods[b].insert(a);
	neighborhoods[b].insert(c);

	neighborhoods[c].insert(a);
	neighborhoods[c].insert(b);
}

float dot(FLT3VECT a, FLT3VECT b) {
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}



void normalize(FLT3VECT &a) {
	float length = sqrt(dot(a, a));
	a.x = a.x / length;
	a.y = a.y / length;
	a.z = a.z / length;
}

FLT3VECT cross(FLT3VECT a, FLT3VECT b) {
	FLT3VECT c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;
	return c;
}


void createMenu(void){
	//////////
	// MENU //
	//////////
	
	// Create an entry
	menuExit = glutCreateMenu(menu);
	glutAddMenuEntry("Point", 1);
	glutAddMenuEntry("Fill", 2);
	glutAddMenuEntry("Line", 3);
	glutAddMenuEntry("Both", 4 );
	glutAddMenuEntry("Exit Program", 0);
	
	// Let the menu respond on the right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	
}

void menu(int value){
	if(value == 0){
		glutDestroyWindow(win);
		exit(0);
	}else{
		drawType=value;
	}
	
	// you would want to redraw now
	glutPostRedisplay();
}

SurFacemesh* readPolygon()
{
  int num,n,m;
  int a,b,c,d;
  float x,y,z;
  SurFacemesh *surfmesh;
  char line[256];
  FILE *fin;


  if ((fin=fopen("dog_noise.off", "r"))==NULL){
    printf("read error...\n");
    exit(0);
  };
  
  /* OFF format */
  while (fgets(line,256,fin) != NULL) {
    if (line[0]=='O' && line[1]=='F' && line[2]=='F')
      break;
  }
  fscanf(fin,"%d %d %d\n",&m,&n,&num);
  
  surfmesh = (SurFacemesh*)malloc(sizeof(SurFacemesh));
  surfmesh->nv = m;
  surfmesh->nf = n;
  surfmesh->ne = num;
  surfmesh->vertex = (FLT3VECT *)malloc(sizeof(FLT3VECT)*surfmesh->nv);
  surfmesh->face = (INT3VECT *)malloc(sizeof(INT3VECT)*surfmesh->nf);
  neighborhoods.resize(surfmesh->nv);
 
  for (n = 0; n < surfmesh->nv; n++) {
    fscanf(fin,"%f %f %f\n",&x,&y,&z);
    surfmesh->vertex[n].x = x;
    surfmesh->vertex[n].y = y;
    surfmesh->vertex[n].z = z;
	
  }
  
  for (n = 0; n < surfmesh->nf; n++) {
    fscanf(fin,"%d %d %d %d\n",&a,&b,&c,&d);
    surfmesh->face[n].a = b;
    surfmesh->face[n].b = c;
    surfmesh->face[n].c = d;
    if(a != 3)
      printf("Errors: reading surfmesh .... \n");
	findNeighborhoods(surfmesh,b,c,d);
	

  }
  fclose(fin);
  
  return surfmesh;
}


// Surface mesh obtained from .off file
SurFacemesh* surfmesh = readPolygon();

void smooth(SurFacemesh* mesh) {
	FLT3VECT * smoothV = (FLT3VECT*)malloc(sizeof(FLT3VECT)*mesh->nv);
	
	for (int n = 0; n < mesh->nv; n++) {
		int number = neighborhoods[n].size();
		FLT3VECT sum = {0.0,0.0,0.0};
		for (int i : neighborhoods[n]) {
			sum.x += mesh->vertex[i].x;
			sum.y += mesh->vertex[i].y;
			sum.z += mesh->vertex[i].z;
		}
		smoothV[n].x = (sum.x / number - mesh->vertex[n].x)*u;
		smoothV[n].y = (sum.y / number - mesh->vertex[n].y)*u;
		smoothV[n].z = (sum.z / number - mesh->vertex[n].z)*u;
	}

	for (int n = 0; n < mesh->nv; n++) {
		mesh->vertex[n].x += smoothV[n].x;
		mesh->vertex[n].y += smoothV[n].y;
		mesh->vertex[n].z += smoothV[n].z;
		smoothV[n].x = smoothV[n].x / u*r;
		smoothV[n].y = smoothV[n].y / u*r;
		smoothV[n].z = smoothV[n].z / u*r;
		mesh->vertex[n].x += smoothV[n].x;
		mesh->vertex[n].y += smoothV[n].y;
		mesh->vertex[n].z += smoothV[n].z;
	}
	free(smoothV);
}


void writeMesh(SurFacemesh* mesh) {
	ofstream myfile;
	myfile.open("dog.off");
	myfile << "OFF" << "\n";
	myfile << mesh->nv << " " << mesh->nf << " " << mesh->ne << "\n";
	int i = 0;

	for (int n = 0; n < mesh->nv; n++) {
			myfile << mesh->vertex[n].x << " " << mesh->vertex[n].y << " " << mesh->vertex[n].z << "\n";
	}

	for (int m = 0; m < mesh->nf; m++) {
		myfile << 3 << " " << mesh->face[m].a << " " << mesh->face[m].b << " " << mesh->face[m].c << "\n";
	}


	myfile.close();
}

void draw()
{

	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
	// change mode depending on menu selection
	switch(drawType)
	{
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		break;
	case 3:
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		break;
	}
	
	glColor3f(0.5f, 0.0f, 1.0f);
	for( int i = 0; i < surfmesh->nf ; ++i){
		glBegin(GL_POLYGON);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x , surfmesh->vertex[surfmesh->face[i].a].y, surfmesh->vertex[surfmesh->face[i].a].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x , surfmesh->vertex[surfmesh->face[i].b].y, surfmesh->vertex[surfmesh->face[i].b].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x , surfmesh->vertex[surfmesh->face[i].c].y, surfmesh->vertex[surfmesh->face[i].c].z);
		glEnd();
	} 
	
	// Both FILL and LINE 
	if(drawType == 4){
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		for( int i = 0; i < surfmesh->nf ; ++i){
		glBegin(GL_POLYGON);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x , surfmesh->vertex[surfmesh->face[i].a].y, surfmesh->vertex[surfmesh->face[i].a].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x , surfmesh->vertex[surfmesh->face[i].b].y, surfmesh->vertex[surfmesh->face[i].b].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x , surfmesh->vertex[surfmesh->face[i].c].y, surfmesh->vertex[surfmesh->face[i].c].z);
		glEnd();
		} 
		glDisable(GL_POLYGON_OFFSET_FILL);
	
	}
	

}

bool init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);

	return true;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// Change eye position for translation
	gluLookAt(
	tx, ty, 100.0f,
	tx, ty, 0.0f,
	0.0f, 1.0f, 0.0f);

	// Rotate with rotation vars
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	// Scale with scale variables
	glScalef(scale, scale, scale);

	
	glPushMatrix();
	draw();
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, 1.0f * w / h, 1.0f, -100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
	if (!mouseDown)
	{
		xrot += 0.3f;
		yrot += 0.4f;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'r':
		transformation = ROTATE;
		break;
	case 'R':
		transformation = ROTATE;
		break;
	case 's':
		transformation = SCALE;
		break;
	case 'S':
		transformation = SCALE;
		break;
	case 't':
		transformation = TRANSLATE;
		break;
	case 'T':
		transformation = TRANSLATE;
		break;
	case 27 : 
		exit(1);
		break;
	}
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
		{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();	
		else
		{
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{

if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
{
	mouseDown = true;
	mousex = x;
	mousey = y;
}

else
	mouseDown = false;
}

void mouseMotion(int x, int y)
{

if (mouseDown)
{	
	int dx = x - mousex;
	int dy = y - mousey;

	switch(transformation)
	{
	case ROTATE:
		xrot += ( ANGFACT*dy );
		yrot += ( ANGFACT*dx );
		break;
	case SCALE:
		scale += SCLFACT * (float) ( dx - dy );
		if( scale < MINSCALE )
					scale = MINSCALE;
		break;
	case TRANSLATE:
		tx -= dx * TFACT;
		ty += dy * TFACT;
		break;
	}

	mousex = x;
	mousey = y;
	glutPostRedisplay();
}


}

int main(int argc, char *argv[])
{
	cout << "please enter how many iterations do you need to treat the mesh: " << "\n" << endl;
	cin >> iteration;
	for (int i = 0; i < iteration; i++) {
		smooth(surfmesh);
	}
	writeMesh(surfmesh);
	glutInit(&argc, argv);

	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	win = glutCreateWindow("3D Polygon");
	createMenu();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);

if (!init())
	return 1;

glutMainLoop();

return 0;
}
