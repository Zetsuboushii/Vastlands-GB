#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gbc_hicolor.h>
#include <gb/gb.h>
#include <gb/cgb.h>

#include "titlescreen.h"
#include <player.h>
#include <dungeon_map.h>
#include <dungeon_tiles.h>
#include <gb/metasprites.h>

#define LAST_TILE_INDEX 255u
const uint8_t empty_tile_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

// Tastenzustände
uint8_t key_state, key_state_prev;
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

// Struktur, die eine banked Adresse kapselt
typedef struct {
    uint8_t bank;
    const void* data;
} banked_ptr_t;

// Da nur der Titlescreen genutzt wird, besteht das Array aus einem einzigen Eintrag
const banked_ptr_t image = {
    BANK(titlescreen), &HICOLOR_VAR(titlescreen)
};

void main(void) {
    uint8_t scroll_limit = 0;
    const hicolor_data* img_data;
    uint8_t img_bank;

    uint8_t player_x = 0;
    uint8_t player_y = 0;

    SHOW_BKG;
    SHOW_SPRITES;

    if (_cpu == CGB_TYPE) {
        // Nur auf Game Boy Color fortfahren
        cpu_fast();

        vsync();
        DISPLAY_OFF;

        img_bank = image.bank;
        img_data = (const hicolor_data*)image.data;

        uint8_t saved_bank = _current_bank;
        if (img_bank) {
            SWITCH_ROM(img_bank);
        }

        SWITCH_ROM(saved_bank);

        // Lade und zeige den Titlescreen im Hi‑Color Modus
        hicolor_start(img_data, img_bank);

        DISPLAY_ON;

        while (true) {
            vsync();
            UPDATE_KEYS();

            if (KEY_IS_PRESSED(J_START)) {
                hicolor_stop();

                set_bkg_data(0, 79u, dungeon_tiles);
                set_bkg_tiles(0, 0, 32u, 32u, dungeon_mapPLN0);
                set_bkg_palette(0, 1, )
                set_sprite_data(0, 4, player_tiles);
                move_metasprite_ex(player_metasprites[0], 0, 0, 0, player_x, player_y);
            }
        }
    }
}
