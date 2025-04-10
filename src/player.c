#define __TARGET_gb

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

// Include references for main, player, camera functionality:
#include "main.h"
#include "player.h"
#include "camera.h"

// -----------------------------------------------------------------------------
// SETTINGS
// -----------------------------------------------------------------------------
#define TILE_SIZE 16      // Each tile in the game is 16x16 pixels (for the player sprite)
#define WALKING_SPEED 3   // Speed at which the player moves in pixels per frame

// -----------------------------------------------------------------------------
// PLAYER VARIABLES
// -----------------------------------------------------------------------------
uint8_t player_direction = 0; // Current facing direction (uses J_LEFT/J_RIGHT/J_UP/J_DOWN)
uint16_t player_x, player_y; // Player position in pixels on the world map
metasprite_t const* player_metasprite;
bool flip_player = false; // If true, the sprite is flipped horizontally
bool player_mid_attack = false; // Indicates if we are in the "middle" (2nd half) of an attack
bool player_attacked = false; // Avoid repeated attacks on a single button press
int8_t attack_pause = 0; // (Unused here, but can be used to add a delay after attacking)

metasprite_t const* axe_metasprite = NULL; // You can store a second metasprite for the axe if needed

// -----------------------------------------------------------------------------
// FUNCTION: setup_player
// -----------------------------------------------------------------------------
/**
 * @brief Initializes the player by loading the "player down" sprite data
 *        into VRAM and setting up default position and direction.
 */
void setup_player() {
    // Load the "player down" tile data into VRAM (starting at tile index 0).
    // This uses the data from player_down.h
    set_sprite_data(0, player_down_TILE_COUNT, player_down_tiles);

    // Initial position is 9 tiles right, 7 tiles down from the top-left
    player_x = 9 * TILE_SIZE;
    player_y = 7 * TILE_SIZE;

    // Start facing downward
    player_direction = J_DOWN;

    // The default metasprite to draw is the first "down" frame
    player_metasprite = player_down_metasprites[0];
}

// -----------------------------------------------------------------------------
// FUNCTION: update_player
// -----------------------------------------------------------------------------
/**
 * @brief Updates the player's position, direction, and attack state based on input.
 *        Also manages collisions with the map and draws the final sprite.
 *
 * @return uint8_t : The next free OAM (sprite) index after drawing the player
 *                   (used by hide_sprites_range(...) later).
 */
uint8_t update_player() {
    // 1) READ CURRENT JOYPAD
    uint16_t joypad_current = joypad();

    // 2) INITIAL FLAGS
    uint8_t player_moving = false; // True if the player is actually moving in x/y
    uint8_t player_attacking = false; // True if the player starts an attack
    uint16_t next_player_x = player_x;
    uint16_t next_player_y = player_y;

    // Decompose direction inputs
    bool up = (joypad_current & J_UP);
    bool down = (joypad_current & J_DOWN);
    bool left = (joypad_current & J_LEFT);
    bool right = (joypad_current & J_RIGHT);
    bool attack = (joypad_current & J_A);

    // For movement checks
    bool move_x = left | right;
    bool move_y = up | down;
    bool moving = move_x | move_y;

    // This ensures that if the A button is no longer pressed, we allow a new attack
    if (!attack) {
        player_attacked = false;
    }

    // 3) ATTACK CHECK: If not already in the middle of an attack
    if (!player_mid_attack) {
        // a) MOVEMENT: If direction keys are pressed, compute next position
        if (left) {
            next_player_x -= WALKING_SPEED;
            player_direction = J_LEFT;
            player_moving = true;
        } else if (right) {
            next_player_x += WALKING_SPEED;
            player_direction = J_RIGHT;
            player_moving = true;
        }
        if (up) {
            next_player_y -= WALKING_SPEED;
            player_direction = J_UP;
            player_moving = true;
        } else if (down) {
            next_player_y += WALKING_SPEED;
            player_direction = J_DOWN;
            player_moving = true;
        }

        // b) ATTACK INPUT: If A is pressed and we haven't attacked yet
        if (attack && !player_attacked) {
            player_attacking = true;
            player_attacked = true; // So we don't spam multiple attacks from a single press
            player_moving = false; // Stop movement if we begin an attack
        }

        // c) Set standing sprites if not attacking or moving
        switch (player_direction) {
        case J_DOWN:
            set_sprite_data(0, player_down_TILE_COUNT, player_down_tiles);
            player_metasprite = player_down_metasprites[0];
            flip_player = false;
            break;
        case J_UP:
            set_sprite_data(0, player_up_TILE_COUNT, player_up_tiles);
            player_metasprite = player_up_metasprites[0];
            flip_player = false;
            break;
        case J_LEFT:
            set_sprite_data(0, player_left_TILE_COUNT, player_left_tiles);
            player_metasprite = player_left_metasprites[0];
            flip_player = false;
            break;
        case J_RIGHT:
            set_sprite_data(0, player_left_TILE_COUNT, player_left_tiles);
            player_metasprite = player_left_metasprites[0];
            flip_player = true; // Right uses the left sprite set, but flipped
            break;
        default:
            // No change
            break;
        }

        // d) ANIMATION & COLLISION
        if (player_moving) {
            // If we are walking, pick the correct tile set and frame
            switch (player_direction) {
            case J_DOWN:
                set_sprite_data(0, player_down_TILE_COUNT, player_down_tiles);
                player_metasprite = player_down_metasprites[two_frame_real_value];
                flip_player = false;
                break;
            case J_UP:
                set_sprite_data(0, player_up_TILE_COUNT, player_up_tiles);
                player_metasprite = player_up_metasprites[two_frame_real_value];
                flip_player = false;
                break;
            case J_LEFT:
                set_sprite_data(0, player_left_TILE_COUNT, player_left_tiles);
                player_metasprite = player_left_metasprites[two_frame_real_value];
                flip_player = false;
                break;
            case J_RIGHT:
                set_sprite_data(0, player_left_TILE_COUNT, player_left_tiles);
                player_metasprite = player_left_metasprites[two_frame_real_value];
                flip_player = true;
                break;
            }

            // Boundaries for the player (prevents going too far off map/camera)
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

            // Check tile collision to ensure we can step onto that tile
            if (is_valid_tile(next_player_x, next_player_y)) {
                // If valid, update the player's position
                player_x = next_player_x;
                player_y = next_player_y;

                // Camera adjustments
                uint16_t desired_camera_x = (player_x > (160 >> 1)) ? (player_x - (160 >> 1)) : 0;
                uint16_t desired_camera_y = (player_y > (144 >> 1)) ? (player_y - (144 >> 1)) : 0;

                if (desired_camera_x > CAMERA_MAX_X) desired_camera_x = CAMERA_MAX_X;
                if (desired_camera_y > CAMERA_MAX_Y) desired_camera_y = CAMERA_MAX_Y;

                // If the camera position changed, update it
                if ((desired_camera_x != camera_x) || (desired_camera_y != camera_y)) {
                    camera_x = desired_camera_x;
                    camera_y = desired_camera_y;
                    vsync();
                    set_camera();
                } else {
                    vsync();
                }
            }
        } else if (player_attacking) {
            // Attack is beginning (the "first half")
            set_sprite_data(0, player_attack_TILE_COUNT, player_attack_tiles);

            // Choose which frame based on direction
            uint16_t axe_screen_x = 0;
            uint16_t axe_screen_y = 0;

            switch (player_direction) {
            case J_DOWN:
                player_metasprite = player_attack_metasprites[0];
                flip_player = false;
            // Example: position the weapon in front or below
                axe_screen_x = player_x - camera_x;
                axe_screen_y = player_y - camera_y + 1 + 8;
                break;
            case J_UP:
                player_metasprite = player_attack_metasprites[2];
                flip_player = false;
                axe_screen_x = player_x - camera_x;
                axe_screen_y = player_y - camera_y + 1 - 8;
                break;
            case J_LEFT:
                player_metasprite = player_attack_metasprites[4];
                flip_player = false;
                axe_screen_x = (player_x - camera_x) - 8;
                axe_screen_y = player_y - camera_y + 1;
                break;
            case J_RIGHT:
                player_metasprite = player_attack_metasprites[4];
                flip_player = true;
                axe_screen_x = (player_x - camera_x) + 8;
                axe_screen_y = player_y - camera_y + 1;
                break;
            default:
                break;
            }
            // Switch to "mid attack" state, next frame call will handle second half
            player_mid_attack = true;
        }
    } else {
        // If we're already in the middle of an attack, show the second half frames
        set_sprite_data(0, player_attack_TILE_COUNT, player_attack_tiles);
        switch (player_direction) {
        case J_DOWN:
            player_metasprite = player_attack_metasprites[1];
            flip_player = false;
            break;
        case J_UP:
            player_metasprite = player_attack_metasprites[3];
            flip_player = false;
            break;
        case J_LEFT:
            player_metasprite = player_attack_metasprites[4];
            flip_player = false;
            break;
        case J_RIGHT:
            player_metasprite = player_attack_metasprites[4];
            flip_player = true;
            break;
        default:
            break;
        }
        // Attack is done once we display the second half
        player_mid_attack = false;
    }

    // -------------------------------------------------------------------------
    // FINAL RENDERING: Draw the player sprite (with or without flip)
    // -------------------------------------------------------------------------
    const uint16_t screen_x = player_x - camera_x;
    const uint16_t screen_y = player_y - camera_y + 1;

    // If flip_player is true, we use move_metasprite_vflip for a mirrored image
    if (flip_player) {
        return move_metasprite_vflip(player_metasprite, 0, 0, screen_x, screen_y);
    }
    // Otherwise, draw normally
    return move_metasprite(player_metasprite, 0, 0, screen_x, screen_y);
}
