
#include "fex.h"
#include <Cocoa/Cocoa.h>

static int sw, sh, fftw, ffth;
static int review, restart, ty, bw, wx, wy;
static id appName, keyer, win, winview, tracker;
static unsigned short int zoom = 0, eraser = 0;
static float scx, scy, brushw, brushh;
static unsigned char *alphas, *a;
static FFT *fft;
static long double ex, tex;
static CGContextRef ctx = NULL;
static CGImageRef img;
static char bar[3][256];

@interface Keyer : NSObject
@end
@implementation Keyer
- (void) quit:(id)sender { restart = 0; review = 0; [NSApp stop: nil]; }
- (void) restart:(id)sender { restart = 1; review = 0; [NSApp stop: nil]; }
- (void) help:(id)sender { /* TODO */ printf("Help menu\n"); }
- (void) zoom:(id)sender { zoom = !zoom; /* TODO trigger redraw? */ }
@end

@interface myView : NSView
@end
@implementation myView
- (BOOL)acceptsFirstMouse { return YES; }
- (BOOL) acceptsFirstResponder { return YES; }
- (BOOL) isOpaque { return YES; }
- (void) drawRect:(NSRect)rect {
	/* draw rendered spectrogram with points */
	CGContextRef c = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextDrawImage(c,NSRectToCGRect(rect),img);
	/* calculate text widths */
	int w1,w2;
	GContextSelectFont(c,"Helvetica-Bold",10,kCGEncodingMacRoman);
	CGPoint p1,p2;
	p1.x = 10; p1.y = ffth-10;
	CGContextSetTextPosition(c,p1.x,p1,y);
	CGContextShowText(c,bar[2],strlen(bar[2]));
	p2 = CGContextGetTextPosition(c);
	w2 = p2.x-p1.x;
	CGContextSetTextPosition(c,p1.x,p1,y);
	CGContextShowText(c,bar[2],strlen(bar[1]));
	p2 = CGContextGetTextPosition(c);
	w1 = p2.x-p1.x;
	/* draw top bar */
	CGContextSetRGBFillColor(ctx,0.0,0.0,0.0,1.0);
	CGContextFillRect(ctx,CGRectMake(0,ffth,fftw,10));
	/* draw text */
	CGContextSetRGBFillColor(ctx,1.0,1.0,1.0,1.0);
	CGContextSetTextPosition(c,p1.x,p1,y);
	CGContextShowText(c,bar[0],strlen(bar[0]));
	CGContextSetTextPosition(c,(fftw-w1)/2,p1.y);
	CGContextShowText(c,bar[1],strlen(bar[1]));
	CGContextSetTextPosition(c,fftw-w2-10,p1.y);
	CGContextShowText(c,bar[2],strlen(bar[2]));
	/* eraser block */
	if (eraser) {
		p1 = [NSEvent mouseLocation];
		p1.x -= brushw/2;
		p1.y -= brushh/2;
		CGContextSetRGBFillColor(ctx,0.0,1.0,1.0,0.4);
		CGContextFillRect(ctx,CGRectMake(p1.x,p1.y,brushh,brushw));
	}
}
- (void) keyDown:(NSEvent *)ev {
	/* SHIFT + ARROW */
	if ( ([ev modifierFlags] & NSAlternateKeyMask) &&
			[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSUpArrowFunctionKey]]) {
		brushw *= 1.2; brushh *= 1.2;
	}
	else if ( ([ev modifierFlags] & NSAlternateKeyMask) &&
			[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSDownArrowFunctionKey]]) {
		brushw *= 1/1.2; brushh *= 1/1.2;
	}
	else if ( ([ev modifierFlags] & NSAlternateKeyMask) &&
			[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSRightArrowFunctionKey]]) {
		brushw *= 1.2; brushh *= 1/1.2;
	}
	else if ( ([ev modifierFlags] & NSAlternateKeyMask) &&
			[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSLeftArrowFunctionKey]]) {
		brushw *= 1/1.2; brushh *= 1.2;
	}
	/* ARROW */
	else if ([[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSUpArrowFunctionKey]]) {
		thresh *= 1.2; [keyer restart:self];
	}
	else if ([[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSDownArrowFunctionKey]]) {
		thresh *= 1/1.2; [keyer restart:self];
	}
	else if ([[ev characters] isEqualToString:@"e"]) {
		if ( (eraser = !eraser) ) {
			/* TODO eraser cursor */
		}
		else {
			/* TODO crosshair cursor */
		}
	}
	else {
		[super keyDown:ev];
	}
}
- (void) mouseDown:(NSEvent *)ev {
	restart = 1; review = 1;
	[NSApp stop: nil];
}
- (void) mouseDragged:(NSEvent *)ev {
	NSPoint xy = [ev locationInWindow];
	xy.x = xy.x * fftw/(float)sw;
	xy.y = xy.y * ffth/(float)sh;
	if (eraser) {
		int i,j;
		for (i = xy.x - brushw/2.0; i < xy.x+brushw/2.0; i++)
			for (j = xy.y - brushh/2.0; j < xy.y+brushh/2.0; j++)
				if ( i >= 0 && i < fft->ts && j >= 0 && j < fft->fs)
					fft->amp[i][j] = min;
		restart = 1; review = 1;
		[NSApp stop: nil];
	}
}
- (void) mouseMoved:(NSEvent *)ev {
	[self setNeedsDisplay:YES];
}
@end

void spectro() {
	NSData *dat = [NSData dataWithBytes:alphas length:fftw*ffth];
	CGDataProviderRef provider =
			CGDataProviderCreateWithCFData((CFDataRef) dat);
	CGImageRef mask = CGImageMaskCreate(fftw,ffth,8,8,fftw,provider,NULL,NO);
	CGContextSetRGBFillColor(ctx,1.0,1.0,1.0,1.0);
	CGContextFillRect(ctx,CGRectMake(0,0,fftw,ffth));
	CGContextSaveGState(ctx);
	CGContextClipToMask(ctx,CGRectMake(0,0,fftw,ffth),mask);
	CGContextSetRGBFillColor(ctx,0.0,0.0,0.0,1.0);
	CGContextFillRect(ctx,CGRectMake(0,0,fftw,ffth));
	CGImageRelease(mask);
	CGDataProviderRelease(provider);
	CGContextRestoreGState(ctx);
	CGContextSetRGBFillColor(ctx,1.0,1.0,0.0,1.0);
	[dat release];
}

int preview_create(int w, int h, FFT *fftp) {
	fft = fftp; fftw = w; ffth = h;
	brushw = w/14; brushh = h/14;
	restart = 0; zoom = eraser = 0;
	[NSAutoreleasePool new];
	[[NSApplication sharedApplication] autorelease];
	id appName = [[[NSProcessInfo processInfo] processName] autorelease];
	/* MENUS & KEY BINDINGS */
	id menubar = [[NSMenu new] autorelease];
	id appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
	id appMenu = [[NSMenu new] autorelease];
	/* quit */
	keyer = [[[Keyer alloc] init] autorelease];
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
	/* zoom */
	mTitle = [@"Zoom" stringByAppendingString:appName];
	mItem = [[[NSMenuItem alloc] initWithTitle:mTitle
			action:@selector(zoom:) keyEquivalent:@"z"] autorelease];
	[mItem setTarget:keyer];
	[appMenu addItem:mItem];
	/* help */
	mTitle = [@"Help" stringByAppendingString:appName];
	mItem = [[[NSMenuItem alloc] initWithTitle:mTitle
			action:@selector(help:) keyEquivalent:@"h"] autorelease];
	[mItem setTarget:keyer];
	[appMenu addItem:mItem];
	[appMenuItem setSubmenu:appMenu];
	/* WINDOW */
	NSRect scr = [[NSScreen mainScreen] frame];
	sw = (int) scr.size.width; sh = (int) scr.size.height;
	win = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,sw,sh)
			styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered
			defer:NO] autorelease];
	winview = [[[myView alloc] initWithFrame:NSMakeRect(0,0,w,h)] autorelease];
	[winview scaleUnitSquareToSize:NSMakeSize(sw/(float)w,sh/(float)h)];
	[win setContentView:winview];
	static int hereagain = 0; if ( (++hereagain) == 1 ) {
		/* this is a ridiculously ugly hack
			can any mac-heads explain this to me?
			without the conditional test, this gives
			a bus error at run time */
		[winview enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
		[win makeKeyAndOrderFront:nil];
	}
	[win setTitle:appName];
	[NSCursor crosshairCursor];
	[NSApp activateIgnoringOtherApps:YES];
	tracker = [[[NSTrackingArea alloc] initWithRect:NSMakeRect(0,0,w,h)
			options:NSTrackingMouseMoved owner:winview userInfo:nil]
			autorelease];
	[winview addTrackingArea:tracker];
	/* MAKE IMAGE */
	alphas = malloc(w*h);
	int i,j;
	for (j = 0; j < h; j++) {
		a = alphas + w*j;
		for (i = 0; i < w; i++, a++)
			*a = (unsigned char) (fft->amp[i][h-j] * 255/min);
	}
	int byte_per_row = w*4;
	int bytes = h*byte_per_row;
	CGColorSpaceRef cspace =
			CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	void *bits = malloc(bytes);
	ctx = CGBitmapContextCreate(bits, w, h, 8, byte_per_row, cspace,
			kCGImageAlphaPremultipliedLast);
	CGColorSpaceRelease(cspace);
	spectro();
	return 0;
}

int preview_peak(int x, int y) {
	CGContextMoveToPoint(ctx,x,y);
	CGContextAddArc(ctx,x,y,0.5,0,2*M_PI,0);
	CGContextClosePath(ctx);
	return 0;
}

int preview_test(long double ee, long double te) {
	CGContextFillPath(ctx);
	img = CGBitmapContextCreateImage(ctx);
	CGContextSetRGBFillColor(ctx,0.0,0.0,0.0,1.0);
	[winview setNeedsDisplay:YES];
	ex = ee; tex = te;
	[NSApp run];
	spectro();
	return review;
}

int preview_destroy() {
	CGImageRelease(img);
	free(alphas);
	return restart;
}

