#include <stdio.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include <stdbool.h>
#include "../res/tiles.c"
#include "../include/player.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 18
#define TILE_SIZE 8
#define NUM_VALID_TILES 5

//TODO Fix sprite location stuff

uint8_t joypad_current = 0, joypad_previous = 0;
uint8_t player_x = 9 * TILE_SIZE;
uint8_t player_y = 9 * TILE_SIZE;

const uint8_t valid_tiles[NUM_VALID_TILES] = {0x2F, 0x33, 0x10, 0x0D, 0x31};

uint8_t get_tile_under_metasprite(const uint8_t metasprite_x, const uint8_t metasprite_y) {
    const uint8_t tile_x = metasprite_x / TILE_SIZE;
    const uint8_t tile_y = (metasprite_y + TILE_SIZE) / TILE_SIZE;

    uint8_t tile_index;
    get_bkg_tiles(tile_x, tile_y, 1, 1, &tile_index);

    return tile_index;
}

bool is_valid_tile(const uint8_t new_x, const uint8_t new_y) {
    uint8_t count_valid_tiles = 0;

    const uint8_t tile_left = get_tile_under_metasprite(new_x, new_y + TILE_SIZE);
    const uint8_t tile_right = get_tile_under_metasprite(new_x + TILE_SIZE, new_y + TILE_SIZE);

    for (uint8_t i = 0; i < NUM_VALID_TILES; i++) {
        if (valid_tiles[i] == tile_left || valid_tiles[i] == tile_right) {
            count_valid_tiles++;
        }
    }

    return count_valid_tiles >= 2;
}

void main() {
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_bkg_data(0, 58, tileTiles);
    set_bkg_tiles(0, 0, MAP_WIDTH, MAP_HEIGHT, tileMap);

    set_sprite_data(0, 4, player_tiles);

    move_metasprite_ex(player_metasprites[0], 0, 0, 0, player_x + 16, player_y + 28);

    // ReSharper disable once CppDFAEndlessLoop
    while (1) {
        uint8_t new_x = player_x;
        uint8_t new_y = player_y;

        joypad_previous = joypad_current;
        joypad_current = joypad();

        if ((joypad_current & J_LEFT) && !(joypad_previous & J_LEFT)) {
            new_x -= TILE_SIZE * 2;
            new_y -= TILE_SIZE;
        } else if ((joypad_current & J_RIGHT) && !(joypad_previous & J_RIGHT)) {
            new_x += TILE_SIZE * 2;
            new_y += TILE_SIZE;
        } else if ((joypad_current & J_DOWN) && !(joypad_previous & J_DOWN)) {
            new_x -= TILE_SIZE * 2;
            new_y += TILE_SIZE;
        } else if ((joypad_current & J_UP) && !(joypad_previous & J_UP)) {
            new_x += TILE_SIZE * 2;
            new_y -= TILE_SIZE;
        }

        if (is_valid_tile(new_x, new_y)) {
            player_x = new_x;
            player_y = new_y;
            move_metasprite_ex(player_metasprites[0], 0, 0, 0, player_x + 16, player_y + 28);
        }

        wait_vbl_done();
    }
}
