

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

BANKREF(player)

const palette_color_t player_palettes[4] = {
	RGB8(  0,  0,  0), RGB8(255,255,255), RGB8( 29,115,  0), RGB8(  0,  0,  0)
	
};

const uint8_t player_tiles[64] = {
	0x03,0x03,0x07,0x04,
	0x0f,0x08,0x1f,0x10,
	0x1f,0x10,0x1f,0x16,
	0x1f,0x16,0x1f,0x10,
	0x3f,0x20,0x3f,0x20,
	0x3f,0x20,0x1f,0x11,
	0x0e,0x0e,0x00,0x07,
	0x00,0x0f,0x00,0x07,
	
	0x03,0x03,0x07,0x04,
	0x0f,0x08,0x1f,0x10,
	0x1f,0x10,0x1f,0x10,
	0x1f,0x10,0x1f,0x10,
	0x3f,0x20,0x3f,0x20,
	0x3f,0x20,0x1f,0x11,
	0x0e,0x0e,0x00,0x07,
	0x00,0x0f,0x00,0x07
	
};

const metasprite_t player_metasprite0[] = {
	METASPR_ITEM(-8, -8, 0, S_PAL(0)),
	METASPR_ITEM(0, 8, 0, S_PAL(0) | S_FLIPX),
	METASPR_TERM
};

const metasprite_t player_metasprite1[] = {
	METASPR_ITEM(-8, -8, 2, S_PAL(0)),
	METASPR_ITEM(0, 8, 2, S_PAL(0) | S_FLIPX),
	METASPR_TERM
};

const metasprite_t player_metasprite2[] = {
	METASPR_ITEM(-8, -8, 0, S_PAL(0)),
	METASPR_ITEM(0, 8, 2, S_PAL(0) | S_FLIPX),
	METASPR_TERM
};

const metasprite_t player_metasprite3[] = {
	METASPR_ITEM(-8, -8, 2, S_PAL(0)),
	METASPR_ITEM(0, 8, 0, S_PAL(0) | S_FLIPX),
	METASPR_TERM
};

const metasprite_t* const player_metasprites[4] = {
	player_metasprite0, player_metasprite1, player_metasprite2, player_metasprite3
};
