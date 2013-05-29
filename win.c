
/* WORK IN PROGRESS */


static HWND win;

LRESULT CALLBACK handler(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
	return DefWindowProc(w,msg,wp,lp);
}

int preview_create(int w, int h, FFT *fftp) {
	MSG Msg;
	WNDCLASSEX cls;
	cls.cbSize = sizeof(WNDCLASSEX); 	
	cls.hInstance = 0;
	cls.lpszClassName = "Fex";
	cls.lpfnWndProc = handler; 
	cls.style = 0; 	
	cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cls.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	cls.lpszMenuName = NULL; 
	cls.cbClsExtra = 0; 
	cls.cbWndExtra = 0;
	cls.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); 
	if( !RegisterClassEx(&cls)) return 0;

	win = CreateWindow("Fex", name,WS_MAXIMIZE,0,0,w,h,NULL,NULL,NULL,None);

	ShowWindow(win,SW_MAXIMIZE);
	UpdateWindow(win); 
}

int preview_test() {
	MSG msg;
	while (GetMessage(&msg,win,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
