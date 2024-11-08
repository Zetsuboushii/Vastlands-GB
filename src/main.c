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
#define NUM_SURROUNDING_TILES 16

uint8_t joypad_current = 0, joypad_previous = 0;
uint8_t player_x = 9 * TILE_SIZE;
uint8_t player_y = 9 * TILE_SIZE;

const uint8_t valid_tiles[NUM_VALID_TILES] = {0x2F, 0x33, 0x10, 0x0D, 0x31};

const int8_t surrounding_offets[NUM_SURROUNDING_TILES][2] = {
    {-1, -1}, {0, -1}, {1, -1}, {2, -1}, // top row
    {-1, 0}, {0, 0}, {1, 0}, {2, 0}, // top middle row
    {-1, 1}, {0, 1}, {1, 1}, {2, 1}, // bottom middle row
    {-1, 2}, {0, 2}, {1, 2}, {2, 2} // bottom row
};
uint8_t surrounding_tiles[NUM_SURROUNDING_TILES];

uint8_t get_tile_index(const uint8_t x, const uint8_t y) {
    const uint8_t tile_x = x / TILE_SIZE;
    const uint8_t tile_y = y / TILE_SIZE;

    uint8_t tile_index;
    get_bkg_tiles(tile_x, tile_y, 1, 1, &tile_index);

    return tile_index;
}

void update_surrounding_tiles(const uint8_t player_x, const uint8_t player_y) {
    for (uint8_t i = 0; i < NUM_SURROUNDING_TILES; i++) {
        const int8_t offset_x = surrounding_offets[i][0] * TILE_SIZE;
        const int8_t offset_y = surrounding_offets[i][1] * TILE_SIZE;
        const uint8_t tile_x = player_x + offset_x;
        const uint8_t tile_y = player_y + offset_y;

        if (tile_x < MAP_WIDTH * TILE_SIZE && tile_y < MAP_HEIGHT * TILE_SIZE) {
            surrounding_tiles[i] = get_tile_index(tile_x, tile_y);
        } else {
            surrounding_tiles[i] = 0xFF; // invalid value
        }
    }
}

uint8_t get_tile_under_metasprite(const uint8_t metasprite_x, const uint8_t metasprite_y) {
    const uint8_t tile_x = metasprite_x / TILE_SIZE;
    const uint8_t tile_y = (metasprite_y + TILE_SIZE) / TILE_SIZE;

    uint8_t tile_index;
    get_bkg_tiles(tile_x, tile_y, 1, 1, &tile_index);

    return tile_index;
}

bool is_valid_tile(const uint8_t x, const uint8_t y) {
    uint8_t count_valid_tiles = 0;

    const uint8_t tile_left = get_tile_under_metasprite(x, y + TILE_SIZE);
    const uint8_t tile_right = get_tile_under_metasprite(x + TILE_SIZE, y + TILE_SIZE);

    for (uint8_t i = 0; i < NUM_VALID_TILES; i++) {
        if (valid_tiles[i] == tile_left || valid_tiles[i] == tile_right) {
            count_valid_tiles++;
        }
    }

    return count_valid_tiles >= 2;
}

// TODO: multiple possibilities (up AND down)
uint8_t can_climb() {
    bool can_climb_up_left = false;
    bool can_climb_up_right = false;
    bool can_climb_down_left = false;
    bool can_climb_down_right = false;

    if (surrounding_tiles[4] == 0x0D && surrounding_tiles[5] == 0x30) {
        can_climb_up_left = true;
    }
    if (surrounding_tiles[6] == 0x0F && surrounding_tiles[7] == 0x10) {
        can_climb_up_right = true;
    }
    if (surrounding_tiles[12] == 0x0F && surrounding_tiles[13] == 0x10) {
        can_climb_down_left = true;
    }
    if (surrounding_tiles[14] == 0x0D && surrounding_tiles[15] == 0x30) {
        can_climb_down_right = true;
    }

    // 0: no climb
    // 1: up left climb
    // 2: up right climb
    // 3: up both climb
    // 4: down left climb
    // 5: down right climb
    // 6: down both climb
    if (can_climb_up_left && can_climb_up_right) {
        return 3;
    }
    if (can_climb_up_left) {
        return 1;
    }
    if (can_climb_up_right) {
        return 2;
    }
    if (can_climb_down_left && can_climb_down_right) {
        return 6;
    }
    if (can_climb_down_left) {
        return 4;
    }
    if (can_climb_down_right) {
        return 5;
    }

    return 0;
}

void main() {
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_bkg_data(0, 58, tileTiles);
    set_bkg_tiles(0, 0, MAP_WIDTH, MAP_HEIGHT, tileMap);

    set_sprite_data(0, 4, player_tiles);

    move_metasprite_ex(player_metasprites[0], 0, 0, 0, player_x + 16, player_y + 28);

    update_surrounding_tiles(player_x, player_y);

    // ReSharper disable once CppDFAEndlessLoop
    while (1) {
        uint8_t new_x = player_x;
        uint8_t new_y = player_y;

        joypad_previous = joypad_current;
        joypad_current = joypad();

        const uint8_t climb_status = can_climb();

        if (joypad_current & J_LEFT && !(joypad_previous & J_LEFT)) {
            if (climb_status == 1 || climb_status == 3) new_y -= TILE_SIZE;
            new_x -= TILE_SIZE * 2;
            new_y -= TILE_SIZE;
        } else if (joypad_current & J_RIGHT && !(joypad_previous & J_RIGHT)) {
            if (climb_status == 5 || climb_status == 6) new_y += TILE_SIZE;
            new_x += TILE_SIZE * 2;
            new_y += TILE_SIZE;
        } else if (joypad_current & J_DOWN && !(joypad_previous & J_DOWN)) {
            if (climb_status == 4 || climb_status == 6) new_y += TILE_SIZE;
            new_x -= TILE_SIZE * 2;
            new_y += TILE_SIZE;
        } else if (joypad_current & J_UP && !(joypad_previous & J_UP)) {
            if (climb_status == 2 || climb_status == 3) new_y -= TILE_SIZE;
            new_x += TILE_SIZE * 2;
            new_y -= TILE_SIZE;
        }

        if (is_valid_tile(new_x, new_y)) {
            player_x = new_x;
            player_y = new_y;
            move_metasprite_ex(player_metasprites[0], 0, 0, 0, player_x + 16, player_y + 28);

            update_surrounding_tiles(player_x, player_y);
        }

        wait_vbl_done();
    }
}
