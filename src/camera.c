#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <gbdk/emu_debug.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <player_down.h>
#include <player_up.h>
#include <player_left.h>
#include <player_attack.h>
#include <axe.h>
#include <bkg.h>
#include <palettes.h>
#include <stdio.h>
#include <gb/hardware.h>

// -----------------------------------------------------------------------------
// MACROS, CONSTANTS, AND DECLARATIONS
// -----------------------------------------------------------------------------

#define LAST_TILE_INDEX 255u
#define TILEMAP_WIDTH (bkg_WIDTH >> 3)    // The tilemap width in tile units
#define TILEMAP_HEIGHT (bkg_HEIGHT >> 3)  // The tilemap height in tile units

#define bkg_MAP_WIDTH  (bkg_WIDTH  / bkg_TILE_W)
#define bkg_MAP_HEIGHT (bkg_HEIGHT / bkg_TILE_H)

// The maximum camera X and Y, so the camera won't scroll beyond the map
#define CAMERA_MAX_Y ((bkg_MAP_HEIGHT - DEVICE_SCREEN_HEIGHT) * 8)
#define CAMERA_MAX_X ((bkg_MAP_WIDTH  - DEVICE_SCREEN_WIDTH ) * 8)

#define TILE_SIZE 16
#define WALKING_SPEED 3
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

// -----------------------------------------------------------------------------
// CAMERA VARIABLES
// -----------------------------------------------------------------------------
uint16_t camera_x, camera_y; // Current camera position in pixels
uint16_t old_camera_x, old_camera_y; // Previous camera position in pixels

uint16_t map_pos_x, map_pos_y; // Current top-left of the map shown on screen (in tiles)
uint16_t old_map_pos_x, old_map_pos_y; // Old map positions to detect changes
uint8_t redraw_flag = 0; // If set, we need to redraw parts of the map

// -----------------------------------------------------------------------------
// INLINE FUNCTIONS FOR CAMERA UPDATES
// -----------------------------------------------------------------------------

/**
 * @brief For some devices, we update columns differently. This
 *        function returns the x coordinate to update if moving left.
 *
 * @param x The base x coordinate
 * @return The new x coordinate based on device constraints
 */
inline uint8_t update_column_left(uint8_t x) {
#if (DEVICE_SCREEN_BUFFER_WIDTH == DEVICE_SCREEN_WIDTH)
    return x + 1;
#else
    return x;
#endif
}

/**
 * @brief Similar function to handle the right column refresh if needed.
 */
inline uint8_t update_column_right(uint8_t x) {
    return x + DEVICE_SCREEN_WIDTH;
}

/**
 * @brief Updates row index if scrolling upward.
 */
inline uint8_t update_row_top(uint8_t y) {
#if (DEVICE_SCREEN_BUFFER_HEIGHT == DEVICE_SCREEN_HEIGHT)
    return y + 1;
#else
    return y;
#endif
}

/**
 * @brief Updates row index if scrolling downward.
 */
inline uint8_t update_row_bottom(uint8_t y) {
    return y + DEVICE_SCREEN_HEIGHT;
}

// -----------------------------------------------------------------------------
// FUNCTION: set_camera
// -----------------------------------------------------------------------------
/**
 * @brief Adjusts background (BKG) submap tiles and attributes depending on
 *        camera_x/camera_y changes. This effectively scrolls the background
 *        by loading new columns/rows at the edges as the camera moves.
 */
void set_camera(void) {
    // Convert camera_y from pixels to tiles
    map_pos_y = (uint16_t)(camera_y >> 3);
    if (map_pos_y != old_map_pos_y) {
        // If camera moved up (decreased y)
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
            // If camera moved down
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

    // Convert camera_x from pixels to tiles
    map_pos_x = (uint16_t)(camera_x >> 3);
    if (map_pos_x != old_map_pos_x) {
        // If camera moved left
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
            // If camera moved right
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

    // Save old camera coordinates
    old_camera_x = camera_x;
    old_camera_y = camera_y;

    // Move the BKG hardware scroll to the new camera position
    move_bkg(camera_x, camera_y);
}

// -----------------------------------------------------------------------------
// FUNCTION: init_camera
// -----------------------------------------------------------------------------
/**
 * @brief Sets up the camera position, loads initial tile data, and draws the
 *        first portion of the map onto the screen. Also ensures the camera
 *        doesn't exceed map boundaries.
 *
 * @param x Initial camera x in pixels
 * @param y Initial camera y in pixels
 */
void init_camera(uint8_t x, uint8_t y) {
    // 1) Load tile data from bank 1
    SWITCH_ROM_MBC5(1);
    set_native_tile_data(0, bkg_TILE_COUNT, bkg_tiles);
    SWITCH_ROM_MBC5(0);

    // 2) Load palette data from bank 4 if on a GBC
    SWITCH_ROM_MBC5(4);
    if (_cpu == CGB_TYPE) set_bkg_palette(BKGF_CGB_PAL0, bkg_PALETTE_COUNT, bkg_palettes);
    SWITCH_ROM_MBC5(0);

    // 3) Set initial camera position
    camera_x = x;
    camera_y = y;

    // Clamp camera position to valid map bounds
    if (camera_x > CAMERA_MAX_X) camera_x = CAMERA_MAX_X;
    if (camera_y > CAMERA_MAX_Y) camera_y = CAMERA_MAX_Y;

    // Store old camera positions
    old_camera_x = camera_x;
    old_camera_y = camera_y;

    // Convert pixels to tile coords for map_x, map_y
    map_pos_x = camera_x >> 3;
    map_pos_y = camera_y >> 3;

    // Force old_map_pos_y to an out-of-range value so the next set_camera call
    // definitely updates the map
    old_camera_x = 0;
    old_map_pos_y = 255;

    // Move the BKG hardware scroll
    move_bkg(camera_x, camera_y);

    // 4) Draw the initial submap to fill the screen
    SWITCH_ROM_MBC5(2);
    set_bkg_submap(
        map_pos_x,
        map_pos_y,
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_WIDTH - map_pos_x),
        MIN(DEVICE_SCREEN_HEIGHT + 1u, bkg_MAP_HEIGHT - map_pos_y),
        bkg_map,
        bkg_MAP_WIDTH
    );
    SWITCH_ROM_MBC5(3);
    set_bkg_submap_attributes(
        map_pos_x,
        map_pos_y,
        MIN(DEVICE_SCREEN_WIDTH + 1u, bkg_MAP_WIDTH - map_pos_x),
        MIN(DEVICE_SCREEN_HEIGHT + 1u, bkg_MAP_HEIGHT - map_pos_y),
        bkg_map_attributes,
        bkg_MAP_WIDTH
    );
    SWITCH_ROM_MBC5(0);

    redraw_flag = false;

    // Make sure the hardware scroll is correct
    move_bkg(camera_x, camera_y);

#if DEVICE_SCREEN_WIDTH == DEVICE_SCREEN_WIDTH
    HIDE_LEFT_COLUMN;
#endif
}
