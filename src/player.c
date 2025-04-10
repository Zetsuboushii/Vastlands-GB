// player.c
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

#include "main.h"
#include "player.h"
#include "camera.h"

#define TILE_SIZE 16
#define WALKING_SPEED 3

// Player variables
uint8_t player_direction = 0;
uint16_t player_x, player_y; // Current player positions in pixels
metasprite_t const* player_metasprite;
bool flip_player = false;
bool player_mid_attack = false;
bool player_attacked = false;
int8_t attack_pause = 0;

metasprite_t const* axe_metasprite = NULL;

void setup_player() {
    set_sprite_data(0, player_down_TILE_COUNT, player_down_tiles);
    player_x = 9 * TILE_SIZE;
    player_y = 7 * TILE_SIZE;
    player_direction = J_DOWN;
    player_metasprite = player_down_metasprites[0];
}

uint8_t update_player() {
    uint16_t joypad_current = joypad();
    uint8_t player_last_direction = player_direction;
    uint8_t player_moving = false;
    uint8_t player_attacking = false;
    uint16_t next_player_x = player_x;
    uint16_t next_player_y = player_y;

    bool up = joypad_current & J_UP;
    bool down = joypad_current & J_DOWN;
    bool left = joypad_current & J_LEFT;
    bool right = joypad_current & J_RIGHT;
    bool attack = joypad_current & J_A;

    bool move_x = left | right;
    bool move_y = up | down;
    bool moving = move_x | move_y;

    uint16_t axe_screen_x = 0;
    uint16_t axe_screen_y = 0;

    if (!attack) {
        player_attacked = false;
    }

    // Attack-Check
    if (!player_mid_attack) {
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

        if (attack && !player_attacked) {
            player_attacking = true;
            player_attacked = true;
            player_moving = false;
        }

        // set standing sprite for after attack
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
                flip_player = true;
                break;
            default: ;
        }

        // Walking animation
        if (player_moving) {
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
                default: ;
            }

            // Limit / Collision
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

                uint16_t desired_camera_x = (player_x > (160 >> 1)) ? player_x - (160 >> 1) : 0;
                uint16_t desired_camera_y = (player_y > (144 >> 1)) ? player_y - (144 >> 1) : 0;

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
        } else if (player_attacking) {
            // Attack start
            set_sprite_data(0, player_attack_TILE_COUNT, player_attack_tiles);
            switch (player_direction) {
                case J_DOWN:
                    player_metasprite = player_attack_metasprites[0];
                    flip_player = false;
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
                    axe_screen_x = player_x - camera_x - 8;
                    axe_screen_y = player_y - camera_y + 1;
                    break;
                case J_RIGHT:
                    player_metasprite = player_attack_metasprites[4];
                    flip_player = true;
                    axe_screen_x = player_x - camera_x + 8;
                    axe_screen_y = player_y - camera_y + 1;
                    break;
                default: ;
            }
            player_mid_attack = true;
        }
    }
    else {
        // Attack second half
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
            default: ;
        }
        player_mid_attack = false;
    }

    // Draw final
    const uint16_t screen_x = player_x - camera_x;
    const uint16_t screen_y = player_y - camera_y + 1;

    if (flip_player) {
        return move_metasprite_vflip(player_metasprite, 0, 0, screen_x, screen_y);
    }
    return move_metasprite(player_metasprite, 0, 0, screen_x, screen_y);
}
