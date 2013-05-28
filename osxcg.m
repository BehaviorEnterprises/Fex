
#include "fex.h"
#include <Cocoa/Cocoa.h>

static int sw, sh, fftw, ffth;
static int review, restart, ty, bw, wx, wy;
static id appName, win, winview;
static unsigned short int zoom = 0, eraser = 0;
static float scx, scy, brushw, brushh;
static unsigned char *alphas, *a;
static FFT *fft;
static long double ex, tex;
static CGContextRef ctx = NULL;
static CGImageRef img;
static NSPoint xy;

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
- (void) help:(id)sender {
	// TODO
	printf("Help menu\n");
}
- (void) zoom:(id)sender {
	zoom = !zoom;
	// TODO trigger redraw?
}
@end

@interface myView : NSView
@end
@implementation myView
- (BOOL) acceptsFirstResponder {
    return YES;
}
- (void) drawRect:(NSRect)rect {
	CGContextRef c = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextDrawImage(c,NSRectToCGRect(rect),img);
	//CGContextDrawImage(c,CGRectMake(0,0,fftw,ffth),img);
}
- (void) keyDown:(NSEvent *)ev {
	/* SHIFT + ARROW */
	if ( ([theEvent modifierFlags] & NSAlternateKeyMask) &&
    		[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSUpArrowFunctionKey]]) {
		brushw *= 1.2; brushh *= 1.2;
	}
	else if ( ([theEvent modifierFlags] & NSAlternateKeyMask) &&
    		[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSDownArrowFunctionKey]]) {
		brushw *= 1/1.2; brushh *= 1/1.2;
	}
	else if ( ([theEvent modifierFlags] & NSAlternateKeyMask) &&
    		[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSRightArrowFunctionKey]]) {
		brushw *= 1.2; brushh *= 1/1.2;
	}
	else if ( ([theEvent modifierFlags] & NSAlternateKeyMask) &&
    		[[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSLeftArrowFunctionKey]]) {
		brushw *= 1/1.2; brushh *= 1.2;
	}
	/* ARROW */
	else if ([[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSUpArrowFunctionKey]]) {
		thresh *= 1.2; [keyer restart];
	}
	else if ([[ev characters] isEqualToString:
			[NSString stringWithFormat:@"%c",NSDownArrowFunctionKey]]) {
		thresh *= 1/1.2; [keyer restart];
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
        [super keyDown:event];
    }
}
- (void) mouseDown:(NSEvent *)ev {
	xy = [ev locationInWindow];
}
- (void) mouseDragged:(NSEvent *)ev {
	if (eraser) {
		int i,j;
		for (i = xy.x - brushw/2.0; i < xy.x+brushw/2.0; i++)
			for (j = xy.y - brushw/2.0; j < xy.y+brushw/2.0; j++)
				if ( i >= 0 ** i < fft->ts && j >= 0 && j < fft->fs)
					fft->amp[i][j] = min;
		// TODO trigger redraw;
	}
	xy = [ev locationInWindow];
}
@end

int preview_create(int w, int h, FFT *fftp) {
	fft = fftp; fftw = w; ffth = h;
	brushw = w/14; brushh = h/14;
	restart = 0; zoom = eraser = 0;
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
	xy = NSMakePoint(sw/2,sh/2);
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
	CGContextSetRGBFillColor(ctx,1.0,0.9,0.0,1.0);
	[NSCursor crosshairCursor];
	return 0;
}

int preview_peak(int x, int y) {
	//CGContextMoveToPoint(ctx,x,y);
	CGContextAddArc(ctx,x,y,0.35,0,2*M_PI,0);
	CGContextClosePath(ctx);
	return 0;
}

int preview_test(long double ee, long double te) {
	CGContextFillPath(ctx);
	img = CGBitmapContextCreateImage(ctx);
	[winview setNeedsDisplay:YES];
	ex = ee; tex = te;
	[NSApp run];
	return review;
}

int preview_destroy() {
	CGImageRelease(img);
	free(alphas);
	return restart;
}

