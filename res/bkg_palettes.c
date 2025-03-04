#pragma bank 4

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include "bkg.h"

BANKREF(bkg_palettes)
const palette_color_t bkg_palettes[16] = {
	RGB8(240,224,176), RGB8( 40,248, 40), RGB8( 24,100,  0), RGB8(  0,  0,  0)
	,
	RGB8(240,224,176), RGB8(104,176,255), RGB8(  0, 96,208), RGB8(  0,  0,  0)
	,
	RGB8(240,224,176), RGB8(144,104,  0), RGB8( 96, 80,  8), RGB8(  0,  0,  0)
	,
	RGB8(240,224,176), RGB8(160,160,160), RGB8( 88, 88, 88), RGB8(  0,  0,  0)
};
