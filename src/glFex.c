

#ifdef WIN64_BUILD
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "fex.h"


/* local function prototypes */
static int glut_init(int, char **);
static int redraw_spectrogram();
/* events */
static void button_press(int, int, int, int);
static void expose();
static void key_press(int, int, int);
static void motion_notify_active(int, int);
static void motion_notify_passive(int, int);

/* local data */
static bool running = true, overlay = true, hud = true, resizing = false;
static fex_t fex;
static int topWin, win;
static double ftime, freq, calc;
static GLint SpectroTexture;
static int w, h;
static unsigned char *data = NULL;
static bool lock_to_top = true;


int main(int argc, char **argv) {
	if ( (fex=fex_create(argv[1])) < 0 ) return fex;
	glut_init(argc, argv);
	redraw_spectrogram();
	while (running)
		glutMainLoopEvent();
	double pex, tex;
	calc = fex_measure(fex, &pex, &tex);
	fex_destroy(fex);
	printf("%lf / %lf = %lf\n", pex, tex, calc);
   return 0;
}

/* Glut Init and Top Window events */

void drawTop() { glClear(GL_COLOR_BUFFER_BIT); glutSwapBuffers(); }
void reshapeTop() {
	if (!lock_to_top) return;
	glutSetWindow(topWin);
	int ww = glutGet(GLUT_WINDOW_WIDTH);
	int wh = glutGet(GLUT_WINDOW_HEIGHT);
	glutSetWindow(win);
	glutPositionWindow(0, 0);
	glutReshapeWindow(ww, wh);
}
int glut_init(int argc, char **argv) {
   glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	/* Create topWin */
   topWin = glutCreateWindow("FEX: ...");
	glClearColor(1.0,1.0,1.0,1.0);
	glClearColor(0.5,0.55,0.6,1.0);
   glutInitWindowSize(640, 480);
   glutInitWindowPosition(50, 50);
   glutDisplayFunc(drawTop);
   glutReshapeFunc(reshapeTop);
	/* Create child window for spectrogram */
	win = glutCreateSubWindow(topWin,0,0,640,480);
   glutDisplayFunc(expose);
	glutMouseFunc(button_press);
	glutMotionFunc(motion_notify_active);
	glutPassiveMotionFunc(motion_notify_passive);
	glutSpecialFunc(key_press);
	return 0;
}

/* Keyboard Handling */

void key_press(int key, int x, int y) {
	static bool fullscreen = false;
	static int wx, wy, ww, wh;
	switch (key) {
		case GLUT_KEY_UP:    fex_threshold(fex, 0.5);  redraw_spectrogram(); break;
		case GLUT_KEY_DOWN:  fex_threshold(fex, -0.5); redraw_spectrogram(); break;
		case GLUT_KEY_LEFT:  fex_floor(fex, -0.5);     redraw_spectrogram(); break;
		case GLUT_KEY_RIGHT: fex_floor(fex, 0.5);      redraw_spectrogram(); break;
		case GLUT_KEY_F1:    running = false;                                break;
		case GLUT_KEY_F2:    overlay = !overlay;       redraw_spectrogram(); break;
		case GLUT_KEY_F11:   glutFullScreenToggle();                         break;
		default: return;
	}
	glutPostRedisplay();
}

/* Mouse Handling */

static int mN = -1, mX, mY, mW, mH;
void button_press(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		if (button == GLUT_RIGHT_BUTTON) resizing = true;
		mN = button; mX = x; mY = y;
		mW = glutGet(GLUT_WINDOW_WIDTH);
		mH = glutGet(GLUT_WINDOW_HEIGHT);
		if (button == GLUT_MIDDLE_BUTTON) {
			lock_to_top = true;
			reshapeTop();
			mN = -1;
		}
	}
	if (state == GLUT_UP) {
		resizing = false;
		glutPostRedisplay();
		mN = -1;
	}
}

void motion_notify_active(int x, int y) {
	int wx = glutGet(GLUT_WINDOW_X);
	int wy = glutGet(GLUT_WINDOW_Y);
	if (mN == -1) return;
	else if (mN == GLUT_LEFT_BUTTON)
		glutPositionWindow(wx + x - mX, wy + y - mY);
	else if (mN == GLUT_RIGHT_BUTTON)
		glutReshapeWindow(mW + x - mX, mH + y - mY);
	lock_to_top = false;
}

void motion_notify_passive(int x, int y) {
	double ww = glutGet(GLUT_WINDOW_WIDTH);
	double wh = glutGet(GLUT_WINDOW_HEIGHT);
	fex_coordinates(fex, x / ww, 1.0 - y / wh, &ftime, &freq);
	glutPostRedisplay();
}

/* Drawing Functions */

int redraw_spectrogram() {
	data = fex_get_spectrogram(fex, &w, &h);
	calc = fex_measure(fex,NULL,NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &SpectroTexture);
	glBindTexture(GL_TEXTURE_2D, SpectroTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	return 0;
}

void expose() {
if (resizing) {
//return;
}

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
if (fex_get_conf(fex, FexShowOverlay) && (resizing ? fex_get_conf(fex,FexShowOverlayResizing) : true)) {
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	int *path = fex_get_path(fex);
	glColor4f(0.8,0.2,0.0, 0.4);
	glLineWidth(4.0);
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

if (fex_get_conf(fex, FexShowHud) && (resizing ? fex_get_conf(fex,FexShowHudResizing) : true)) {
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
snprintf(str,32,"%0.3lfs %0.1lfkHz    FE: %0.2lf",ftime,freq,calc);
glutBitmapString(GLUT_BITMAP_HELVETICA_18,str);
}
	glDisable(GL_BLEND);
	glutSwapBuffers();
}

