//
// Created by zetsu on 06.04.25.
//


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
#include <player_down.h>
#include <player_up.h>
#include <player_left.h>
#include <player_attack.h>
#include <axe.h>
#include <bkg.h>
#include <palettes.h>
#include <stdio.h>
#include <gb/hardware.h>

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
#define WALKING_SPEED 3
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))



// Camera variables
uint16_t camera_x, camera_y;
uint16_t old_camera_x, old_camera_y; // Current and old camera positions in pixels
uint16_t map_pos_x, map_pos_y, old_map_pos_x, old_map_pos_y; // Current and old map positions in tiles
uint8_t redraw_flag = 0; // Redraw flag to indicate if camera position has changed

/*
 *  CAMERA
 */
// Camera update inline functions to directly replace function calls and check at compile time
inline uint8_t update_column_left(uint8_t x) {
#if (DEVICE_SCREEN_BUFFER_WIDTH == DEVICE_SCREEN_WIDTH)
    return x + 1;
#else
    return x;
#endif
}

inline uint8_t update_column_right(uint8_t x) {
    return x + DEVICE_SCREEN_WIDTH;
}

inline uint8_t update_row_top(uint8_t y) {
#if (DEVICE_SCREEN_BUFFER_HEIGHT == DEVICE_SCREEN_HEIGHT)
    return y + 1;
#else
    return y;
#endif
}

inline uint8_t update_row_bottom(uint8_t y) {
    return y + DEVICE_SCREEN_HEIGHT;
}

void set_camera(void) {
    // Up or down
    map_pos_y = (uint16_t)(camera_y >> 3);
    if (map_pos_y != old_map_pos_y) {
        if (camera_y < old_camera_y) {
            SWITCH_ROM_MBC5(2);
            set_bkg_submap(
                map_pos_x, map_pos_y,
                MIN(21u, bkg_MAP_WIDTH - map_pos_x),
                1,
                bkg_map,
                bkg_MAP_WIDTH
            );
            SWITCH_ROM_MBC5(3);
            set_bkg_submap_attributes(
                map_pos_x, map_pos_y,
                MIN(21u, bkg_MAP_WIDTH - map_pos_x),
                1,
                bkg_map_attributes,
                bkg_MAP_WIDTH
            );
            SWITCH_ROM_MBC5(0);
        } else {
            if ((bkg_MAP_HEIGHT - 18) > map_pos_y) {
                SWITCH_ROM_MBC5(2);
                set_bkg_submap(
                    map_pos_x,
                    map_pos_y + 18u,
                    MIN(21u, bkg_MAP_WIDTH - map_pos_x),
                    1,
                    bkg_map,
                    bkg_MAP_WIDTH
                );
                SWITCH_ROM_MBC5(3);
                set_bkg_submap_attributes(
                    map_pos_x,
                    map_pos_y + 18u,
                    MIN(21u, bkg_MAP_WIDTH - map_pos_x),
                    1,
                    bkg_map_attributes,
                    bkg_MAP_WIDTH
                );
                SWITCH_ROM_MBC5(0);
            }
        }

        old_map_pos_y = map_pos_y;
    }

    // Left or right
    map_pos_x = (uint16_t)(camera_x >> 3);
    if (map_pos_x != old_map_pos_x) {
        if (camera_x < old_camera_x) {
            SWITCH_ROM_MBC5(2);
            set_bkg_submap(
                map_pos_x,
                map_pos_y,
                1,
                MIN(19u, bkg_MAP_HEIGHT - map_pos_y),
                bkg_map,
                bkg_MAP_WIDTH
            );
            SWITCH_ROM_MBC5(3);
            set_bkg_submap_attributes(
                map_pos_x,
                map_pos_y,
                1,
                MIN(19u, bkg_MAP_HEIGHT - map_pos_y),
                bkg_map_attributes,
                bkg_MAP_WIDTH
            );
            SWITCH_ROM_MBC5(0);
        } else {
            if ((bkg_MAP_WIDTH - 20) > map_pos_x) {
                SWITCH_ROM_MBC5(2);
                set_bkg_submap(
                    map_pos_x + 20u,
                    map_pos_y,
                    1,
                    MIN(19u, bkg_MAP_HEIGHT - map_pos_y),
                    bkg_map,
                    bkg_MAP_WIDTH
                );
                SWITCH_ROM_MBC5(3);
                set_bkg_submap_attributes(
                    map_pos_x + 20u,
                    map_pos_y,
                    1,
                    MIN(19u, bkg_MAP_HEIGHT - map_pos_y),
                    bkg_map_attributes,
                    bkg_MAP_WIDTH
                );
                SWITCH_ROM_MBC5(0);
            }
        }

        old_map_pos_x = map_pos_x;
    }

    old_camera_x = camera_x;
    old_camera_y = camera_y;

    move_bkg(camera_x, camera_y); // Update Hardware Scroll Position
}

void init_camera(uint8_t x, uint8_t y) {
    // Setting up tile data
    SWITCH_ROM_MBC5(1);
    set_native_tile_data(0, bkg_TILE_COUNT, bkg_tiles);
    SWITCH_ROM_MBC5(0);

    // Setting up palette data
    SWITCH_ROM_MBC5(4);
    if (_cpu == CGB_TYPE) set_bkg_palette(BKGF_CGB_PAL0, bkg_PALETTE_COUNT, bkg_palettes);
    SWITCH_ROM_MBC5(0);

    // Initial camera position
    camera_x = x;
    camera_y = y;
    // Enforce map bounds
    if (camera_x > CAMERA_MAX_X) camera_x = CAMERA_MAX_X;
    if (camera_y > CAMERA_MAX_Y) camera_y = CAMERA_MAX_Y;
    old_camera_x = camera_x;
    old_camera_y = camera_y;

    map_pos_x = camera_x >> 3;
    map_pos_y = camera_y >> 3;
    old_camera_x = old_map_pos_y = 255;
    move_bkg(camera_x, camera_y);

    // Draw initial map view on screen
    SWITCH_ROM_MBC5(2);
    set_bkg_submap(
        map_pos_x,
        map_pos_y,
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_WIDTH - map_pos_x),
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_HEIGHT - map_pos_y),
        bkg_map,
        bkg_MAP_WIDTH
    );
    SWITCH_ROM_MBC5(3);
    set_bkg_submap_attributes(
        map_pos_x,
        map_pos_y,
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_WIDTH - map_pos_x),
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_HEIGHT - map_pos_y),
        bkg_map_attributes,
        bkg_MAP_WIDTH
    );
    SWITCH_ROM_MBC5(0);

    redraw_flag = false;

    move_bkg(camera_x, camera_y);
#if DEVICE_SCREEN_WIDTH == DEVICE_SCREEN_WIDTH
    HIDE_LEFT_COLUMN;
#endif
}