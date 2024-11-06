#include <stdio.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include "../res/tiles.c"
#include "player.h"

void main()
{
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_bkg_data(0, 58, tileTiles);
    set_bkg_tiles(0, 0, 20, 18, tileMap);

    //set_sprite_data(0, player_TILE_COUNT, player_tiles);
    //move_metasprite_ex(player_metasprites, 0, 0, 0, 80, 80);

    while (1)
    {
        wait_vbl_done();
    }
}
