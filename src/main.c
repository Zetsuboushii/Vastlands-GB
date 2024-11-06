#include <stdio.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include "../res/tiles.c"
#include "player.h"

uint8_t joypadCurrent = 0, joypadPrevious = 0;
uint8_t playerX = 72, playerY = 92;

void main() {
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_bkg_data(0, 58, tileTiles);
    set_bkg_tiles(0, 0, 20, 18, tileMap);

    set_sprite_data(0, 4, player_tiles);

    while (1) {
        joypadPrevious = joypadCurrent;
        joypadCurrent = joypad();

        if ((joypadCurrent & J_LEFT) && !(joypadPrevious & J_LEFT)) {
            playerX -= 16;
            playerY -= 8;
        } else if ((joypadCurrent & J_RIGHT) && !(joypadPrevious & J_RIGHT)) {
            playerX += 16;
            playerY += 8;
        } else if ((joypadCurrent & J_DOWN) && !(joypadPrevious & J_DOWN)) {
            playerX -= 16;
            playerY += 8;
        } else if ((joypadCurrent & J_UP) && !(joypadPrevious & J_UP)) {
            playerX += 16;
            playerY -= 8;
        }

        move_metasprite_ex(player_metasprites[0], 0, 0, 0, playerX, playerY);

        wait_vbl_done();
    }
}
