#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <gbdk/emu_debug.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include <gbdk/console.h>
#include <gbdk/font.h>

#include <gbc_hicolor.h>
#include <titlescreen.h>
#include <player.h>
#include <bkg.h>
#include <palettes.h>
#include <stdio.h>

/*
 *  MACROS, CONSTANTS AND DECLARATIONS
 */
#define LAST_TILE_INDEX 255u
#define TILEMAP_WIDTH (bkg_WIDTH >> 3)
#define TILEMAP_HEIGHT (bkg_HEIGHT >> 3)
#define bkg_MAP_WIDTH (bkg_WIDTH / bkg_TILE_W)
#define bkg_MAP_HEIGHT (bkg_HEIGHT / bkg_TILE_H)
#define CAMERA_MAX_Y ((bkg_MAP_HEIGHT - DEVICE_SCREEN_HEIGHT) * 8)
#define CAMERA_MAX_X ((bkg_MAP_WIDTH - DEVICE_SCREEN_WIDTH) * 8)
#define TILE_SIZE 16
#define NUM_VALID_TILES 18
#define WALKING_SPEED 3
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

const uint8_t empty_tile_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Valid tiles for collision detection
const uint8_t valid_tiles[NUM_VALID_TILES] = {
    0x04, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x2C,
    0x37, 0x38
};

// Input
uint8_t key_state, key_state_prev;
uint8_t joypad_current = 0;

// Player
uint8_t player_direction = 0;
uint8_t player_x, player_y;
metasprite_t const* playerMetasprite;


// Title screen variables
bool is_title_shown = false;
const hicolor_data* img_data;
uint8_t img_bank;

typedef struct {
    uint8_t bank;
    const void* data;
} banked_ptr_t;

const banked_ptr_t image = {
    BANK(titlescreen), &HICOLOR_VAR(titlescreen)
};

/*
 *  TITLE SCREEN
 */
void show_titlescreen() {
    is_title_shown = true;
    img_bank = image.bank;
    img_data = (const hicolor_data*)image.data;

    uint8_t saved_bank = _current_bank;
    if (img_bank) {
        SWITCH_ROM(img_bank);
    }

    SWITCH_ROM(saved_bank);
    hicolor_start(img_data, img_bank);
}

void cancel_titlescreen() {
    hicolor_stop();
    is_title_shown = false;
}

/*
 *  COLLISION DETECTION
 */
bool is_valid_tile(uint8_t x, uint8_t y) {
    uint8_t tile_x = x / 8;
    uint8_t tile_y = y / 8;

    if (tile_x > TILEMAP_WIDTH || tile_x < 1 || tile_y > TILEMAP_HEIGHT || tile_y < 1) {
        return false;
    }

    uint16_t index = (tile_y - 1) * TILEMAP_WIDTH + tile_x - 1;
    uint16_t tile_dr = bkg_map[index];
    uint16_t tile_ur = bkg_map[index + TILEMAP_WIDTH];

    for (uint8_t i = 0; i < ARRAY_COUNT(valid_tiles); i++) {
        for (uint8_t j = 0; j < ARRAY_COUNT(valid_tiles); j++) {
            if (tile_dr == valid_tiles[i] && tile_ur == valid_tiles[j]) {
                return true;
            }
        }
    }
    return false;
}

/*
 *  PLAYER
 */
void setup_player() {
    set_sprite_data(0, player_TILE_COUNT, player_tiles);
    player_x = 9 * TILE_SIZE;
    player_y = 9 * TILE_SIZE;
    player_direction = J_DOWN;
    playerMetasprite = player_metasprites[0];
}

uint8_t update_player() {
    uint8_t playerLastDirection = player_direction;
    uint8_t playerMoving = false;
    uint8_t next_player_x = player_x;
    uint8_t next_player_y = player_y;

    if (joypad_current & J_LEFT) {
        next_player_x -= WALKING_SPEED;
        player_direction = J_LEFT;
        playerMoving = true;
    }
    if (joypad_current & J_RIGHT) {
        next_player_x += WALKING_SPEED;
        player_direction = J_RIGHT;
        playerMoving = true;
    }
    if (joypad_current & J_UP) {
        next_player_y -= WALKING_SPEED;
        player_direction = J_UP;
        playerMoving = true;
    }
    if (joypad_current & J_DOWN) {
        next_player_y += WALKING_SPEED;
        player_direction = J_DOWN;
        playerMoving = true;
    }

    if (playerMoving) {
        if (player_direction != playerLastDirection) {
            set_sprite_data(0, player_TILE_COUNT, player_tiles);

            switch (player_direction) {
                case J_DOWN: playerMetasprite = player_metasprites[0]; break;
                case J_UP: playerMetasprite = player_metasprites[1]; break;
                case J_LEFT: playerMetasprite = player_metasprites[2]; break;
                case J_RIGHT: playerMetasprite = player_metasprites[3]; break;
                default: break;
            }
        }

        if (is_valid_tile(next_player_x, next_player_y)) {
            player_x = next_player_x;
            player_y = next_player_y;
        }
    }

    return move_metasprite(playerMetasprite, 0, 0, player_x, player_y + 10);
}

/*
 *  MAIN GAME LOOP
 */
void main(void) {
    SHOW_BKG;

    if (_cpu == CGB_TYPE) {
        cpu_fast();
        vsync();
        DISPLAY_OFF;
        show_titlescreen();
        DISPLAY_ON;

        while (is_title_shown) {
            UPDATE_KEYS();
            if (KEY_IS_PRESSED(J_START)) {
                cancel_titlescreen();
            }
        }

        SHOW_SPRITES;
        SPRITES_8x16;
        setup_player();

        while (true) {
            joypad_current = joypad();
            uint8_t last_sprite = 0;
            last_sprite += update_player();
            hide_sprites_range(last_sprite, 40);

            set_bkg_data(0, bkg_TILE_COUNT, bkg_tiles);
            set_bkg_palette(0, bkg_PALETTE_COUNT, bkg_palettes);
            VBK_REG = 1;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map_attributes);
            VBK_REG = 0;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map);

            set_sprite_palette(0, palettes_PALETTE_COUNT, palettes_palettes);
            wait_vbl_done();
        }
    }
}