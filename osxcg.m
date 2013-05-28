
#include "fex.h"
#include <Cocoa/Cocoa.h>

typedef unsigned short int Bool;
static const unsigned short int True = 1;
static const unsigned short int False = 0;

// function prototypes

static int sw, sh, sx, sy, fftw, ffth;
static int review, restart, ty, bw, wx, wy;
static id appName, win, winview;
static Bool running, zoom, eraser;
static float scx, scy, brushw, brushh;
static unsigned char *alphas, *a;
static FFT *fft;
static long double ex, tex;
static CGContextRef ctx = NULL;
static CGImageRef img;

void osxcg_help() {
}

void draw() {
	// copy prebuffer to buffer
	// show appropriate cursor for eraser

	// make dislpay string

	// copy buffer to win
}

@interface Keyer : NSObject
@end
@implementation Keyer
- (void) quit:(id)sender {
	restart = 0; review = 0;
	[NSApp stop: nil];
}
- (void) restart:(id)sender {
	restart = 1; review = 0;
	[NSApp stop: nil];
}
- (void) test:(id)sender {
	printf("shift r\n");
}
@end

@interface myView : NSView
@end
@implementation myView
- (void) drawRect:(NSRect)rect {
	CGContextRef c = [[NSGraphicsContext currentContext] graphicsPort];
	//CGContextDrawImage(c,NSRectToCGRect(rect),img);
	CGContextDrawImage(c,CGRectMake(0,0,fftw,ffth),img);
}
@end

int preview_create(int w, int h, FFT *fftp) {
	fft = fftp; fftw = w; ffth = h;
	brushw = w/14; brushh = h/14;
	restart = 0; zoom = eraser = 0; //False
	[NSAutoreleasePool new];
	[NSApplication sharedApplication];
	id appName = [[NSProcessInfo processInfo] processName];
	/* MENUS & KEY BINDINGS */
	id menubar = [[NSMenu new] autorelease];
	id appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
	id appMenu = [[NSMenu new] autorelease];
	/* quit */
	id keyer = [[[Keyer alloc] init] autorelease];
	id mTitle = [@"Quit" stringByAppendingString:appName];
	id mItem = [[[NSMenuItem alloc] initWithTitle:mTitle
			action:@selector(quit:) keyEquivalent:@"q"] autorelease];
	[mItem setTarget:keyer];
	[appMenu addItem:mItem];
	/* restart */
	mTitle = [@"Restart" stringByAppendingString:appName];
	mItem = [[[NSMenuItem alloc] initWithTitle:mTitle
			action:@selector(restart:) keyEquivalent:@"r"] autorelease];
	[mItem setTarget:keyer];
	[appMenu addItem:mItem];
	/* other */
	//mTitle = [@"Test" stringByAppendingString:appName];
	//mItem = [[[NSMenuItem alloc] initWithTitle:mTitle
	//		action:@selector(quit:) keyEquivalent:@"."] autorelease];
	//[mItem setKeyEquivalentModifierMask:NSShiftKeyMask];
	//[mItem setTarget:keyer];
	//[appMenu addItem:mItem];
	[appMenuItem setSubmenu:appMenu];

	/* WINDOW */
	NSRect scr = [[NSScreen mainScreen] frame];
	sw = (int) scr.size.width; sh = (int) scr.size.height;
	sx = sw/2; sy = sh/2;
	win = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,sw,sh)
			styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO]
			autorelease];
	winview = [[[myView alloc] initWithFrame:NSMakeRect(0,0,w,h)] autorelease];
	[winview scaleUnitSquareToSize:NSMakeSize(sw/(float)w,sh/(float)h)];
	[win setContentView:winview];
	[[win contentView] enterFullScreenMode:[NSScreen mainScreen]
			withOptions:nil];
	[win setTitle:appName];
	[win makeKeyAndOrderFront:nil];
	[NSApp activateIgnoringOtherApps:YES];

	alphas = malloc(w*h);
	int i,j;
	for (j = 0; j < h; j++) {
		a = alphas + w*j;
		for (i = 0; i < w; i++, a++)
			*a = (unsigned char) (fft->amp[i][h-j] * 255/min);
	}

	int byte_per_row = w*4;
	int bytes = h*byte_per_row;
	CGColorSpaceRef cspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	void *bits = malloc(bytes);
	ctx = CGBitmapContextCreate(bits, w, h, 8, byte_per_row, cspace,
			kCGImageAlphaPremultipliedLast);
	CGColorSpaceRelease(cspace);

	NSData *dat = [NSData dataWithBytes:alphas length:w*h];
	CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef) dat);
	CGImageRef mask = CGImageMaskCreate(w,h,8,8,w,provider,NULL,NO);
	CGContextSetRGBFillColor(ctx,1.0,1.0,1.0,1.0);
	CGContextFillRect(ctx,CGRectMake(0,0,w,h));
	CGContextClipToMask(ctx,CGRectMake(0,0,w,h),mask);
	CGContextSetRGBFillColor(ctx,0.0,0.0,0.0,1.0);
	CGContextFillRect(ctx,CGRectMake(0,0,w,h));
	CGImageRelease(mask);
	CGDataProviderRelease(provider);

	return 0;
}

int preview_peak(int x, int y) {
	return 0;
}

int preview_test(long double ee, long double te) {
	img = CGBitmapContextCreateImage(ctx);
	
	//[winview display];
	[winview setNeedsDisplay:YES];

	ex = ee; tex = te;
	// fill dots
	// draw

	[NSApp run];
	return review;
}

int preview_destroy() {
	CGImageRelease(img);
	free(alphas);
	return restart;
}

