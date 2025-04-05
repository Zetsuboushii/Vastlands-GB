
#ifndef METASPRITE_player_attack_H
#define METASPRITE_player_attack_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define player_attack_TILE_ORIGIN 0
#define player_attack_TILE_W 8
#define player_attack_TILE_H 16
#define player_attack_WIDTH 16
#define player_attack_HEIGHT 16
#define player_attack_TILE_COUNT 20
#define player_attack_PALETTE_COUNT 1
#define player_attack_COLORS_PER_PALETTE 4
#define player_attack_TOTAL_COLORS 4
#define player_attack_PIVOT_X 8
#define player_attack_PIVOT_Y 8
#define player_attack_PIVOT_W 16
#define player_attack_PIVOT_H 16
extern const metasprite_t* const player_attack_metasprites[5];

BANKREF_EXTERN(player_attack)

extern const palette_color_t player_attack_palettes[4];
extern const uint8_t player_attack_tiles[320];

#endif
