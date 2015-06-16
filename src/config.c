/**********************************************************************\
* CONFIG.C
* Part of FEX, see FEX.C for complete license information
* Author: Jesse McClure, copyright 2013-2015
* License; GPL3
\**********************************************************************/

#include "fex.h"
#include "fex_struct.h"

// TODO: this module is just a placeholder for now

fex_t configure(FEX *fex) {
	if (!fex) return FexNullAccess;
	fex->conf.winlen = 256;
	fex->conf.hop = 64;
	fex->conf.lopass = 1.25;
	fex->conf.hipass = 10.0;
	fex->conf.floor = -28.0;
	fex->conf.threshold = -14.0;
	return FexSuccess;
}
