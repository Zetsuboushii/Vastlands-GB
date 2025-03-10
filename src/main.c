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

// Valid tiles for collision detection
#define NUM_VALID_TILES 21
const uint8_t valid_tiles[NUM_VALID_TILES] = {0x04, 0x05, 0x06, 0x0c, 0x0d, 0x2e, 0x33, 0x34, 0x79, 0x2d, 0xa3, 0x38, 0x97, 0x98, 0x6e, 0xa4, 0x6f, 0x60, 0x53, 0x54, 0x61};

// Input variables
uint8_t key_state, key_state_prev;
uint8_t joypad_current = 0;

// Player variables
uint8_t player_direction = 0;
uint16_t player_x, player_y; // Current player positions in pixels
metasprite_t const* playerMetasprite;

// Camera variables
uint16_t camera_x, camera_y;
uint16_t old_camera_x, old_camera_y; // Current and old camera positions in pixels
uint16_t map_pos_x, map_pos_y, old_map_pos_x, old_map_pos_y; // Current and old map positions in tiles
uint8_t redraw_flag = 0; // Redraw flag to indicate if camera position has changed

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
bool is_valid_tile(uint16_t x, uint16_t y) {
    uint16_t tile_x = x / 8;
    uint16_t tile_y = y / 8;

    if (tile_x > TILEMAP_WIDTH || tile_x < 1 || tile_y > TILEMAP_HEIGHT || tile_y < 1) {
        return false;
    }

    uint16_t index = (tile_y - 1) * TILEMAP_WIDTH + (tile_x - 1);
    uint16_t tile_dr;
    SWITCH_ROM_MBC5(2);
    tile_dr = bkg_map[index];
    SWITCH_ROM_MBC5(0);

    EMU_printf("Checking: Tile %x in %u,%u at %x\n", tile_dr, tile_y - 1, tile_x - 1, index);

    for (uint8_t i = 0; i < ARRAY_COUNT(valid_tiles); i++) {
        if (tile_dr == valid_tiles[i]) {
            return true;
        }
    }
    return false;
}


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


/*
 *  PLAYER
 */
void setup_player() {
    set_sprite_data(0, player_TILE_COUNT, player_tiles);
    player_x = 9 * TILE_SIZE;
    player_y = 7 * TILE_SIZE;
    player_direction = J_DOWN;
    playerMetasprite = player_metasprites[0];
}

uint8_t update_player() {
    uint8_t playerLastDirection = player_direction;
    uint8_t playerMoving = false;
    uint16_t next_player_x = player_x;
    uint16_t next_player_y = player_y;

    if (joypad_current & J_UP) {
        next_player_y -= WALKING_SPEED;
        player_direction = J_UP;
        playerMoving = true;
    } else if (joypad_current & J_DOWN) {
        next_player_y += WALKING_SPEED;
        player_direction = J_DOWN;
        playerMoving = true;
    }
    if (joypad_current & J_LEFT) {
        next_player_x -= WALKING_SPEED;
        player_direction = J_LEFT;
        playerMoving = true;
    } else if (joypad_current & J_RIGHT) {
        next_player_x += WALKING_SPEED;
        player_direction = J_RIGHT;
        playerMoving = true;
    }


    if (playerMoving) {
        if (player_direction != playerLastDirection) {
            set_sprite_data(0, player_TILE_COUNT, player_tiles);

            switch (player_direction) {
            case J_DOWN: playerMetasprite = player_metasprites[0];
                break;
            case J_UP: playerMetasprite = player_metasprites[1];
                break;
            case J_LEFT: playerMetasprite = player_metasprites[2];
                break;
            case J_RIGHT: playerMetasprite = player_metasprites[3];
                break;
            default: break;
            }
        }

        uint16_t min_player_x = 0;
        uint16_t max_player_x = CAMERA_MAX_X + SCREEN_WIDTH + 8;
        uint16_t min_player_y = 0;
        uint16_t max_player_y = CAMERA_MAX_Y + SCREEN_HEIGHT + 8;

        if (next_player_x < min_player_x) {
            next_player_x = min_player_x;
        } else if (next_player_x > max_player_x) {
            next_player_x = max_player_x;
        }
        if (next_player_y < min_player_y) {
            next_player_y = min_player_y;
        } else if (next_player_y > max_player_y) {
            next_player_y = max_player_y;
        }


        if (is_valid_tile(next_player_x, next_player_y)) {
            player_x = next_player_x;
            player_y = next_player_y;

            uint16_t desired_camera_x =
                (player_x > (160 >> 1)) ? player_x - (160 >> 1) : 0;
            uint16_t desired_camera_y =
                (player_y > (144 >> 1)) ? player_y - (144 >> 1) : 0;

            if (desired_camera_x > CAMERA_MAX_X) desired_camera_x = CAMERA_MAX_X;
            if (desired_camera_y > CAMERA_MAX_Y) desired_camera_y = CAMERA_MAX_Y;

            if ((desired_camera_x != camera_x) || (desired_camera_y != camera_y)) {
                camera_x = desired_camera_x;
                camera_y = desired_camera_y;
                vsync();
                set_camera();
            } else {
                vsync();
            }
        }
    }

    uint16_t screen_x = player_x - camera_x;
    uint16_t screen_y = player_y - camera_y + 8;

    return move_metasprite(playerMetasprite, 0, 0, screen_x, screen_y);
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

        uint16_t init_cam_x = player_x - (160 >> 1);
        uint16_t init_cam_y = player_y - (144 >> 1);
        init_camera(init_cam_x, init_cam_y);

        while (true) {
            joypad_current = joypad();
            uint8_t last_sprite = 0;
            last_sprite += update_player();
            hide_sprites_range(last_sprite, 40);

            set_sprite_palette(0, palettes_PALETTE_COUNT, palettes_palettes);
            wait_vbl_done();
        }
    }
}
