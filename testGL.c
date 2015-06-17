

#ifdef _WIN32_
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "fex.h"

// http://freeglut.sourceforge.net/docs/api.php#Freeglut.h_Header

typedef unsigned short int bool;
enum { false = 0, true };

static bool running = true;
static int topWin, win;

void drawTop() {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0,1.0,1.0, 1.0);
	//glColor4f(0.5,0.525,0.575, 1.0);
	glBegin(GL_QUADS);
		glVertex2f(-1.0, -1.0);
		glVertex2f(-1.0, 1.0);
		glVertex2f(1.0, 1.0);
		glVertex2f(1.0, -1.0);
	glEnd();
	glutSwapBuffers();
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_MULTISAMPLE_ARB);
	/* Use spectrogram background: */
	glColor4f(0.0,1.0,1.0, 1.0);
   glBegin(GL_QUADS);
      glVertex2f(-0.5f, -0.5f);
      glVertex2f( 0.5f, -0.5f);
      glVertex2f( 0.5f,  0.5f);
      glVertex2f(-0.5f,  0.5f);
   glEnd();
	glutSwapBuffers();
}


void specialKeys(int key, int x, int y) {
	static bool fullscreen = false;
	static int wx, wy, ww, wh;
	if (key == GLUT_KEY_F1)
		running = false;
	else if (key == GLUT_KEY_F11)
		glutFullScreenToggle();
}


static int mN = -1, mX, mY, mW, mH;
void mouse_button(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mN = button; mX = x; mY = y;
		mW = glutGet(GLUT_WINDOW_WIDTH);
		mH = glutGet(GLUT_WINDOW_HEIGHT);
		if (button == GLUT_MIDDLE_BUTTON) {
			glutSetWindow(topWin);
			int ww = glutGet(GLUT_WINDOW_WIDTH);
			int wh = glutGet(GLUT_WINDOW_HEIGHT);
			glutSetWindow(win);
			glutPositionWindow(0, 0);
			glutReshapeWindow(ww, wh);
			mN = -1;
		}
	}
	if (state == GLUT_UP) {
		glutPostRedisplay();
		mN = -1;
	}
}

void move_resize(int x, int y) {
	int wx = glutGet(GLUT_WINDOW_X);
	int wy = glutGet(GLUT_WINDOW_Y);
	if (mN == -1) return;
	else if (mN == GLUT_LEFT_BUTTON)
		glutPositionWindow(wx + x - mX, wy + y - mY);
	else if (mN == GLUT_RIGHT_BUTTON)
		glutReshapeWindow(mW + x - mX, mH + y - mY);
}

void motion(int x, int y) {
	double ww = glutGet(GLUT_WINDOW_WIDTH);
	double wh = glutGet(GLUT_WINDOW_HEIGHT);
	glutPostRedisplay();
}

int main(int argc, char **argv) {
   glutInit(&argc, argv);                 // Initialize GLUT
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
   topWin = glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
   glutInitWindowSize(640, 480);   // Set the window's initial width & height
   glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
   glutDisplayFunc(drawTop); // Register display callback handler for window re-paint

	win = glutCreateSubWindow(topWin,0,0,640,480);
	glClearColor(1.0,1.0,1.0,1.0);

   glutDisplayFunc(display); // Register display callback handler for window re-paint
	glutMouseFunc(mouse_button);
	glutMotionFunc(move_resize);
	glutPassiveMotionFunc(motion);
	glutSpecialFunc(specialKeys);
	while (running)
		glutMainLoopEvent();

   return 0;
}
