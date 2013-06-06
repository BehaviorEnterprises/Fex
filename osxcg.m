
#include "fex.h"
#include <Cocoa/Cocoa.h>

static int sw, sh, fftw, ffth;
static int review, restart, ty, bw, wx, wy;
static id appName, keyer, win, winview, tracker;
static unsigned short int zoomer = 0, eraser = 0;
static float scx, scy, brushw, brushh;
static unsigned char *alphas, *a;
static FFT *fft;
static long double ex, tex;
static CGContextRef ctx = NULL;
static CGImageRef img;
static char bar[2][256];

@interface Keyer : NSObject
@end
@implementation Keyer
- (void) quit:(id)sender { restart = 0; review = 0; [NSApp stop: nil]; }
- (void) restart:(id)sender { restart = 1; review = 0; [NSApp stop: nil]; }
- (void) review:(id)sender { restart = 1; review = 1; [NSApp stop: nil]; }
- (void) help:(id)sender { /* TODO */ printf("Help menu\n"); }
@end

@interface myView : NSView
@end
@implementation myView
- (BOOL) acceptsFirstMouse:(NSEvent *)ev { return YES; }
- (BOOL) acceptsFirstResponder { return YES; }
- (BOOL) becomeFirstMouse { return YES; }
- (BOOL) isOpaque { return YES; }
- (void) drawRect:(NSRect)rect {
	/* draw rendered spectrogram with points */
	CGContextRef c = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextDrawImage(c,NSRectToCGRect(rect),img);
	/* calculate text widths */
	NSPoint p = [NSEvent mouseLocation];
	p.x *= fftw/(float)sw; p.y *= ffth/(float)sh;
	snprintf(bar[0],255,"time: %.3lfs | freq: %.3lfkhz",
			fft->time[(int)p.x], fft->freq[(int)p.y]);
	snprintf(bar[1],255,"path: %.2Lf | time: %.2Lf | FE: %.2Lf",
			ex,tex,ex/tex);
	int w1,w2;
	CGContextSelectFont(c,"Helvetica-Bold",3,kCGEncodingMacRoman);
	CGPoint p1,p2;
	p1.x = 2; p1.y = ffth-3;
	CGContextSetTextPosition(c,p1.x,p1.y);
	CGContextShowText(c,bar[1],strlen(bar[1]));
	p2 = CGContextGetTextPosition(c);
	w2 = p2.x-p1.x;
	CGContextSetTextPosition(c,p1.x,p1.y);
	CGContextShowText(c,name,strlen(name));
	p2 = CGContextGetTextPosition(c);
	w1 = p2.x-p1.x;
	/* draw top bar */
	CGContextSetRGBFillColor(c,0.0,0.0,0.0,1.0);
	CGContextFillRect(c,CGRectMake(0,ffth-4,fftw,4));
	/* draw text */
	CGContextSetTextDrawingMode (c, kCGTextFill); 
	CGContextSetRGBFillColor(c,1.0,1.0,1.0,1.0);
	CGContextSetTextPosition(c,p1.x,p1.y);
	CGContextShowText(c,bar[0],strlen(bar[0]));
	CGContextSetTextPosition(c,(fftw-w1)/2,p1.y);
	CGContextShowText(c,name,strlen(name));
	CGContextSetTextPosition(c,fftw-w2-2,p1.y);
	CGContextShowText(c,bar[1],strlen(bar[1]));
	if (eraser) { /* eraser block */
		p.x -= brushw/2.0;
		p.y -= brushh/2.0;
		CGContextSetRGBFillColor(c,0.0,1.0,1.0,0.4);
		CGContextFillRect(c,CGRectMake(p.x-1,p.y-1,brushh+2,brushw+2));
	}
	else if (zoomer) { /* zoomer window */
		CGContextSetRGBStrokeColor(c,0.0,0.5,0.1,1.0);
		CGContextSetRGBFillColor(c,0.0,0.5,0.1,0.2);
		CGContextMoveToPoint(c,p.x,0);
		CGContextAddLineToPoint(c,p.x,fft->fs);
		if (range[0]) {
			CGRect r = CGRectMake(range[0],0,p.x-range[0],fft->fs);
			CGContextStrokeRect(c,r)
			CGContextFillRect(c,r)
		}
	}
}
- (void) keyDown:(NSEvent *)ev {
	range[0] = range[1] = zoomer = 0;
	int i, c, n = [[ev characters] length];
	for (i = 0; i < n; i++) {
		c = [[ev characters] characterAtIndex:i];
		if ([ev modifierFlags] & NSAlternateKeyMask ) {
			if (c == NSUpArrowFunctionKey) { brushw *= 1.2; brushh *= 1.2; }
			else if (c==NSDownArrowFunctionKey) { brushw*=1/1.2; brushh*=1/1.2; }
			else if (c==NSLeftArrowFunctionKey) { brushw*=1/1.2; brushh*=1.2; }
			else if (c==NSRightArrowFunctionKey) { brushw*=1.2; brushh*=1/1.2; }
			else [super keyDown:ev];
		}
		else {
			if (c == NSUpArrowFunctionKey) {
				thresh *= 1.2; [keyer review:self];
			}
			else if (c == NSDownArrowFunctionKey) {
				thresh *= 1/1.2; [keyer review:self];
			}
			else if (c == 'e') {
				if ( (eraser=!eraser) ) [NSCursor hide];
				else [NSCursor unhide];
			}
			else if (c == 'z') {
				zoomer = 1;
			}
			else [super keyDown:ev];
		}
	}
	[self setNeedsDisplay:YES];
}
- (void) mouseDown:(NSEvent *)ev {
	if (!zoomer) [super mouseDown:ev];
	NSPoint xy = [ev locationInWindow];
	if (!range[0]) range[0] = xy.x*fftw/(float)ws;
	else range[1] = xy.x*fftw/(float)ws;
	if (range[1]) [keyer restart:self];
}
- (void) mouseDragged:(NSEvent *)ev {
	if (eraser) {
		NSPoint xy = [ev locationInWindow];
		xy.x = xy.x * fftw/(float)sw;
		xy.y = xy.y * ffth/(float)sh;
		int i,j;
		for (i = xy.x - brushw/2.0; i < xy.x+brushw/2.0; i++)
			for (j = xy.y - brushh/2.0; j < xy.y+brushh/2.0; j++)
				if ( i >= 0 && i < fft->ts && j >= 0 && j < fft->fs)
					fft->amp[i][j] = min;
		[keyer review:self];
	}
	else {
		[self setNeedsDisplay:YES];
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
	restart = range[0] = range[1] = zoomer = eraser = 0;
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
	[win makeFirstResponder:winview];
	tracker = [[[NSTrackingArea alloc] initWithRect:NSMakeRect(0,0,w,h)
			options:NSTrackingMouseMoved|NSTrackingActiveAlways
			owner:winview userInfo:nil] autorelease];
	[winview addTrackingArea:tracker];
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

