
#ifndef METASPRITE_bkg_H
#define METASPRITE_bkg_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define bkg_TILE_ORIGIN 0
#define bkg_TILE_W 8
#define bkg_TILE_H 8
#define bkg_WIDTH 1248
#define bkg_HEIGHT 528
#define bkg_TILE_COUNT 210
#define bkg_PALETTE_COUNT 4
#define bkg_COLORS_PER_PALETTE 4
#define bkg_TOTAL_COLORS 16
#define bkg_MAP_ATTRIBUTES bkg_map_attributes
#define bkg_MAP_ATTRIBUTES_WIDTH 156
#define bkg_MAP_ATTRIBUTES_HEIGHT 66
#define bkg_MAP_ATTRIBUTES_PACKED_WIDTH 156
#define bkg_MAP_ATTRIBUTES_PACKED_HEIGHT 66

BANKREF_EXTERN(bkg_palettes)
extern const palette_color_t bkg_palettes[16];
BANKREF_EXTERN(bkg_tiles)
extern const uint8_t bkg_tiles[3360];
BANKREF_EXTERN(bkg_map)
extern const unsigned char bkg_map[10296];
BANKREF_EXTERN(bkg_map_attributes)
extern const unsigned char bkg_map_attributes[10296];

#endif
