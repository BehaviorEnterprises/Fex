/**********************************************************************\
* FEX - The Frequency Excursion Calculator
*
* Author: Jesse McClure, copyright 2012-2014
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

Wave *create_wave(const char *fname) {
	Wave *w = (Wave *) calloc(1,sizeof(Wave));
	SF_INFO *winfo;
	SNDFILE *wfile = NULL;
	winfo = (SF_INFO *) calloc(1, sizeof(SF_INFO));
	wfile = sf_open(fname, SFM_READ, winfo);
	w->samples = winfo->frames;
	w->rate = winfo->samplerate;
	w->d = (double *) calloc(w->samples, sizeof(double));
	sf_count_t n = sf_readf_double(wfile, w->d, w->samples);
	if (n != w->samples)
		fprintf(stderr,"Error reading \"%s\"\n\t"
				"\t%d samples read of\n\t%d reported size\n",
				fname, n, w->samples);
	sf_close(wfile);
	return w;
}

int free_wave(Wave **wp) {
	Wave *w = *wp;
	free(w->d);
	free(w);
}
