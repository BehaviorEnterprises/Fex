
/*
	WORK IN PROGRESS !
 I don't currently have a windows machine to test on - and I've never
 done win32 programming before.  So this may  not even be on the right
 track at most points.
 	J McClure - 30 May 2013
 */


#include <windows.h>

#define quit()		{review=0; restart=0; PostQuitMessage(0);}
#define review()	{review=1; restart=0; PostQuitMessage(0);}
#define restart()	{review=0; restart=1; PostQuitMessage(0);}

static void buttonpress(MSG *);
static void buttonrelease(MSG *);
//static void expose(MSG *);
static void keypress(MSG *);
static void motionnotify(MSG *);

static HWND win;
static HDC dc;

static void (*handler[WM_USER])(MSG *ev) = {
	[WM_LBUTTONDOWN]	= buttonpress,
	[WM_LBUTTONUP]		= buttonrelease,
	[WM_PAINT]			= expose,
	[WM_KEYDOWN]		= keypress,
	[WM_MOUSEMOVE]		= motionnotify,
};

void buttonpress(MSG *ev) {
}

void buttonrelease(MSG *ev) {
	// stop message;
}

void draw() {
	// TODO
}

void expose(MSG *ev) {
	draw();
}

void keypress(MSG *ev) {
	int key, mod; // TODO
	if (mod == Control) {
		if (key == 'q') quit();
		else if (key == 'r') restart();
	}
	else if (mod == Shift) {
		if (key == VK_UP) { brushw *= 1.2; brushh *= 1.2; };
		else if (key == VK_DOWN) { brushw *= 1/1.2; brushh *= 1/1.2; };
		else if (key == VK_LEFT) { brushw *= 1/1.2; brushh *= 1.2; };
		else if (key == VK_RIGHT) { brushw *= 1.2; brushh *= 1/1.2; };
	}
	else if (key == VK_UP) { thresh *= 1.2; restart(); }
	else if (key == VK_DOWN) { thresh *= 1/1.2; restart(); }
	// zoom?
	else if (key == 'e') eraser = !eraser;
	draw();
}

void motionnotify(MSG *ev) {
	int x = GET_X_LPARAM(ev->lParam), y = GET_Y_LPARAM(ev->lParam);
	// TODO
}

int preview_create(int w, int h, FFT *fftp) {
	MSG Msg;
	WNDCLASSEX cls; cls.cbSize = sizeof(WNDCLASSEX); 	
	cls.hInstance = 0; cls.lpszClassName = "Fex"; cls.lpfnWndProc = NULL; 
	cls.style = 0; cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cls.hIconSm = LoadIcon(NULL, IDI_WINLOGO); cls.cbClsExtra = 0; 
	cls.hCursor = LoadCursor(NULL, IDC_ARROW); cls.lpszMenuName = NULL; 
	cls.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); 
	cls.cbWndExtra = 0;
	if( !RegisterClassEx(&cls)) return 0;
	win = CreateWindow("Fex", name,WS_MAXIMIZE,0,0,w,h,NULL,NULL,NULL,None);
	dc = GetWindowDC(win);

	ShowWindow(win,SW_MAXIMIZE);
}

int preview_test() {
	UpdateWindow(win); 
	MSG msg;
	while (GetMessage(&msg,win,0,0)) {
		//TranslateMessage(&msg);
		if (msg.hwnd != win) continue;
		if (handler[msg.message]) handler[msg.message](&msg)
	}
}
