#define __TARGET_gb

#include <axe.h>
#include <bkg.h>
#include <gb/cgb.h>
#include <gb/gb.h>
#include <gb/hardware.h>
#include <gb/metasprites.h>
#include <gbc_hicolor.h>
#include <gbdk/console.h>
#include <gbdk/emu_debug.h>
#include <gbdk/font.h>
#include <gbdk/platform.h>
#include <palettes.h>
#include <player_attack.h>
#include <player_down.h>
#include <player_left.h>
#include <player_up.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <titlescreen.h>

/*
 *  MACROS, CONSTANTS AND DECLARATIONS
 */
#define LAST_TILE_INDEX 255u
#define TILEMAP_WIDTH (bkg_WIDTH >> 3)
#define TILEMAP_HEIGHT (bkg_HEIGHT >> 3)

#define TILE_SIZE 16
#define WALKING_SPEED 3
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

// Valid tiles for collision detection
#define NUM_VALID_TILES 21
const uint8_t valid_tiles[NUM_VALID_TILES] = {
    0x04, 0x05, 0x06, 0x0c, 0x0d, 0x2e, 0x33, 0x34, 0x79, 0x2d, 0xa3,
    0x38, 0x97, 0x98, 0x6e, 0xa4, 0x6f, 0x60, 0x53, 0x54, 0x61};

// Input variables
uint8_t key_state, key_state_prev;
uint8_t joypad_current = 0;

// Sprite Two-Frame Counter
uint8_t two_frame_counter, two_frame_real_value = 0;

void update_two_frame_counter() {
  two_frame_counter += 3;
  two_frame_real_value = two_frame_counter >> 4;

  if (two_frame_real_value >= 2) {
    two_frame_real_value = 0;
    two_frame_counter = 0;
  }
}



/*
 *  COLLISION DETECTION
 */
bool is_valid_tile(uint16_t x, uint16_t y) {
  uint16_t tile_x = x / 8;
  uint16_t tile_y = y / 8;

  if (tile_x > TILEMAP_WIDTH || tile_x < 1 || tile_y > TILEMAP_HEIGHT ||
      tile_y < 1) {
    return false;
  }

  uint16_t index = (tile_y - 1) * TILEMAP_WIDTH + (tile_x - 1);
  uint16_t tile_dr;
  SWITCH_ROM_MBC5(2);
  tile_dr = bkg_map[index];
  SWITCH_ROM_MBC5(0);

  EMU_printf("Checking: Tile %x in %u,%u at %x\n", tile_dr, tile_y - 1,
             tile_x - 1, index);

  for (uint8_t i = 0; i < ARRAY_COUNT(valid_tiles); i++) {
    if (tile_dr == valid_tiles[i]) {
      return true;
    }
  }
  return false;
}

#include "camera.h"
#include "player.h"
#include "titlescreen.h"

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

      update_two_frame_counter();
      uint8_t last_sprite = 0;
      last_sprite += update_player();
      hide_sprites_range(last_sprite, 40);
      set_sprite_palette(0, palettes_PALETTE_COUNT, palettes_palettes);

      wait_vbl_done();
    }
  }
}
