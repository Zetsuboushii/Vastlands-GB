#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <gbdk/emu_debug.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <gbc_hicolor.h>

#include <titlescreen.h>
#include <player.h>
#include <bkg.h>
#include <palettes.h>
#include <stdio.h>
#include <gbdk/console.h>

#include <gbdk/font.h>

font_t min_font;

void init_window() {
    font_init();
    min_font = font_load(font_min);
    SHOW_WIN;
    move_win(7, 120);
}

#define LAST_TILE_INDEX 255u
const uint8_t empty_tile_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

uint8_t key_state, key_state_prev;
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

#define TILEMAP_WIDTH (bkg_WIDTH >> 3)
#define TILEMAP_HEIGHT (bkg_HEIGHT >> 3)
#define TILE_SIZE 16
#define NUM_VALID_TILES  1

const uint8_t valid_tiles[NUM_VALID_TILES] = {0x04};

const hicolor_data* img_data;
uint8_t img_bank;

typedef struct {
    uint8_t bank;
    const void* data;
} banked_ptr_t;

const banked_ptr_t image = {
    BANK(titlescreen), &HICOLOR_VAR(titlescreen)
};

bool is_title_shown = false;

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

uint8_t joypadCurrent = 0;

uint8_t playerDirection = 0;
uint8_t playerX, playerY;
uint8_t nextPlayerX, nextPlayerY;

metasprite_t const* playerMetasprite;

bool is_valid_tile(uint8_t x, uint8_t y) {
    uint8_t tile_x = x / 8;
    uint8_t tile_y = y / 8;

    if (tile_x > TILEMAP_WIDTH || tile_x < 1 || tile_y > TILEMAP_HEIGHT || tile_y < 1) {
        return false;
    }

    uint16_t index = (tile_y - 1) * TILEMAP_WIDTH + tile_x - 1;
    uint16_t tile = bkg_map[index];

    for (uint8_t i = 0; i < ARRAY_COUNT(valid_tiles); i++) {
        if (tile == valid_tiles[i]) {
            return true;
        }
    }
    return false;
}

void setup_player() {
    set_sprite_data(0, player_TILE_COUNT, player_tiles);

    playerX = 9 * TILE_SIZE;
    playerY = 9 * TILE_SIZE;

    playerDirection = J_DOWN;

    playerMetasprite = player_metasprites[0];
}

uint8_t update_player() {
    uint8_t playerLastDirection = playerDirection;


    uint8_t playerMoving = false;
    nextPlayerX = playerX;
    nextPlayerY = playerY;

    if (joypadCurrent & J_LEFT) {
        nextPlayerX -= TILE_SIZE;
        playerDirection = J_LEFT;
        playerMoving = true;
    }
    if (joypadCurrent & J_RIGHT) {
        nextPlayerX += TILE_SIZE;
        playerDirection = J_RIGHT;
        playerMoving = true;
    }
    if (joypadCurrent & J_UP) {
        nextPlayerY -= TILE_SIZE;
        playerDirection = J_UP;
        playerMoving = true;
    }
    if (joypadCurrent & J_DOWN) {
        nextPlayerY += TILE_SIZE;
        playerDirection = J_DOWN;
        playerMoving = true;
    }

    if (playerMoving) {
        if (playerDirection != playerLastDirection) {
            set_sprite_data(0, player_TILE_COUNT, player_tiles);

            switch (playerDirection) {
            case J_DOWN:
                playerMetasprite = player_metasprites[0];
                break;
            case J_UP:
                playerMetasprite = player_metasprites[1];
                break;
            case J_LEFT:
                playerMetasprite = player_metasprites[2];
                break;
            case J_RIGHT:
                playerMetasprite = player_metasprites[3];
                break;
            default:
                break;
            }
        }

        if (is_valid_tile(nextPlayerX, nextPlayerY)) {
            playerX = nextPlayerX;
            playerY = nextPlayerY;
        }
    }

    return move_metasprite(playerMetasprite, 0, 0, playerX, playerY + 8);
}


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
            joypadCurrent = joypad();

            uint8_t lastSprite = 0;

            lastSprite += update_player();

            hide_sprites_range(lastSprite, 40);

            set_bkg_data(0, bkg_TILE_COUNT, bkg_tiles);
            set_bkg_palette(0, bkg_PALETTE_COUNT, bkg_palettes);
            VBK_REG = 1;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map_attributes);
            VBK_REG = 0;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map);


            set_sprite_palette(0,palettes_PALETTE_COUNT, palettes_palettes);

            wait_vbl_done();
        }
    }
}
