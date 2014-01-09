/**********************************************************************\
* FEX - The Frequency Excursion Calculator
*
* Author: Jesse McClure, copyright 2013-2014
* License: GPL3
*
*    This program is free software: you can redistribute it and/or
*    modify it under the terms of the GNU General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see
*    <http://www.gnu.org/licenses/>.
*
\**********************************************************************/

#include "fex.h"

int die(const char *msg, ...) {
	va_list arg;
	fprintf(stderr,"Fatal Error: ");
	va_start(arg, msg);
	vfprintf(stderr,msg,arg);
	va_end(arg);
	exit(1);
}



int main(int argc, const char **argv) {
	const char *fname = configure(argc,argv);
	Wave *wav = create_wave(fname);
	FFT *fft = create_fft(wav);
	create_spectro(fft, fname);
	free_wave(&wav);

	xlib_event_loop();
	fprintf(stdout,"%.3lf\n", spect->fex);

	free_spectro();
	free_fft(&fft);
	return 0;
}
