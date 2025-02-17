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

#define LAST_TILE_INDEX 255u
const uint8_t empty_tile_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

uint8_t key_state, key_state_prev;
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

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

        while (true) {
            set_bkg_data(0, bkg_TILE_COUNT, bkg_tiles);
            set_bkg_palette(0, bkg_PALETTE_COUNT, bkg_palettes);
            VBK_REG = 1;
            set_bkg_tiles(0, 0, 20, 18, bkg_map_attributes);
            VBK_REG = 0;
            set_bkg_tiles(0, 0, 20, 18, bkg_map);

            SHOW_SPRITES;
            SPRITES_8x16;

            set_sprite_palette(0,palettes_PALETTE_COUNT,palettes_palettes);

            set_sprite_data(0, player_TILE_COUNT, player_tiles);
            move_metasprite(player_metasprites[0], 0, 0, 80, 80);

            wait_vbl_done();
        }
    }
}
