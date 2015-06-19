

#ifdef WIN64_BUILD
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "fex.h"

// http://freeglut.sourceforge.net/docs/api.php#Freeglut.h_Header

typedef unsigned short int bool;
enum { false = 0, true };

static bool running = true, overlay = true, hud = true;
static fex_t fex;
static int topWin, win;
static double time, freq, calc;
static GLint SpectroTexture;
static int w, h;
static unsigned char *data = NULL;

int redraw_spectrogram() {
	data = fex_get_spectrogram(fex, &w, &h);
	calc = fex_measure(fex,NULL,NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &SpectroTexture);
	glBindTexture(GL_TEXTURE_2D, SpectroTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, SpectroTexture);
	return 0;
}







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


/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	//glEnable(GL_MULTISAMPLE_ARB);
	/* Use spectrogram background: */
	glColor4f(1.0,1.0,1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SpectroTexture);
   glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
      glVertex2f(-1.0f, -1.0f);
      glTexCoord2f(1.0, 0.0);
      glVertex2f(1.0f, -1.0f);
      glTexCoord2f(1.0, 1.0);
      glVertex2f( 1.0f,  1.0f);
      glTexCoord2f(0.0, 1.0);
      glVertex2f(-1.0f,  1.0f);
   glEnd();
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
if (overlay) {
	int *path = fex_get_path(fex);
	glColor4f(0.8,0.2,0.0, 0.4);
	glLineWidth(4.0);
	glEnable(GL_LINE_SMOOTH);
	int x;
	double dx, dy;

	glBegin(GL_LINE_STRIP);
	for (x = 0; x < w; ++x) {
		if (path[x] < 0) continue;
		dx = 2.0 * x / w - 1.0;
		dy = 2.0 * path[x] / h - 1.0;
		glVertex2f(dx,dy);
	}
	glEnd();

	glColor4f(0.0,0.0,0.4, 0.5);
	glBegin(GL_QUADS);
	for (x = 0; x < w; ++x) {
		if (path[x] < 0) continue;
		dx = 2.0 * x / w - 1.0;
		dy = 2.0 * path[x] / h - 1.0;
		glVertex2f(dx-0.004,dy-0.008);
		glVertex2f(dx-0.004,dy+0.008);
		glVertex2f(dx+0.004,dy+0.008);
		glVertex2f(dx+0.004,dy-0.008);
	}
	glEnd();
}

if (hud) {
float x_units = 2.0 / (float) glutGet(GLUT_WINDOW_WIDTH);
float y_units = 2.0 / (float) glutGet(GLUT_WINDOW_HEIGHT);
glColor4f(0.0,0.0,0.5,0.4);
	glBegin(GL_QUADS);
      glVertex2f(-1.0f, 1.0f - 26 * y_units);
      glVertex2f(1.0f, 1.0f - 26 * y_units);
      glVertex2f( 1.0f,  1.0f);
      glVertex2f(-1.0f,  1.0f);
	glEnd();

glColor4f(1.0,0.9,0.6,0.8);

glRasterPos2f(-1.0 + 8 * x_units, 1.0 - 20 * y_units);
char str[32];
snprintf(str,32,"%0.3lfs %0.1lfkHz    FE: %0.2lf",time,freq,calc);
glutBitmapString(GLUT_BITMAP_HELVETICA_18,str);
}
	glDisable(GL_BLEND);
	glutSwapBuffers();
}


int show_menu(int x, int y) {
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	glutSwapBuffers();
}





void specialKeys(int key, int x, int y) {
	static bool fullscreen = false;
	static int wx, wy, ww, wh;
	if (key == GLUT_KEY_F1)
		running = false;
	else if (key == GLUT_KEY_F2) {
		overlay = !overlay;
		redraw_spectrogram();
		glutPostRedisplay();
	}
	else if (key == GLUT_KEY_F11)
		glutFullScreenToggle();
	else if (key == GLUT_KEY_UP) {
		fex_threshold(fex, 0.5);
		redraw_spectrogram();
		glutPostRedisplay();
	}
	else if (key == GLUT_KEY_DOWN) {
		fex_threshold(fex, -0.5);
		redraw_spectrogram();
		glutPostRedisplay();
	}
}


static int mN = -1, mX, mY, mW, mH;
void mouse_button(int button, int state, int x, int y) {
	static bool old_hud;
	if (state == GLUT_DOWN) {
		old_hud = hud;
		hud = false;
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
		hud = old_hud;
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
	fex_coordinates(fex, x / ww, 1.0 - y / wh, &time, &freq);
	glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char **argv) {
	fex = fex_create(argv[1]);
	if (fex < 0) return fex;

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
	redraw_spectrogram();
	while (running)
		glutMainLoopEvent();

	double pex, tex;
	calc = fex_measure(fex,&pex,&tex);
	fex_destroy(fex);
	printf("%lf / %lf = %lf\n",pex,tex,calc);
   return 0;
}
