#include "../include/player.h"
#include "../include/isometric.h"

void init_player(Player *player, uint8_t sprite_id) {
    player->x = 0;
    player->y = 0;
    player->sprite_id = sprite_id;
    set_sprite_tile(sprite_id, 0); // Der Sprite wird auf das erste Tile gesetzt
}

void move_player(Player *player, int8_t dx, int8_t dy) {
    player->x += dx;
    player->y += dy;
}

void update_player_sprite(Player *player) {
    int16_t screen_x = player->x;
    int16_t screen_y = player->y;
    iso_to_screen(&screen_x, &screen_y);
    move_sprite(player->sprite_id, screen_x, screen_y);
}
