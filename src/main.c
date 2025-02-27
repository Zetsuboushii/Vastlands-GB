#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <gbc_hicolor.h>

#include <titlescreen.h>
#include <player.h>
#include <bkg.h>
#include <palettes.h>
#include <gbdk/emu_debug.h>

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

#define PLAYER_SPEED 10

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

uint8_t get_tile_index(uint8_t x, uint8_t y) {
    uint8_t tile_x = x / TILE_SIZE;
    uint8_t tile_y = y / TILE_SIZE;

    const uint8_t tile_index = get_bkg_tile_xy(tile_x, tile_y);

    return tile_index;
}

bool is_valid_tile(uint8_t x, uint8_t y) {
    for (uint8_t i = 0; i < NUM_VALID_TILES; i++) {
        if (valid_tiles[i] == get_tile_index(x, y)) { return true; }
        EMU_printf("%u", get_tile_index(x, y));
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
