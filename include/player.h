#ifndef PLAYER_H
#define PLAYER_H

#include <gb/gb.h>

typedef struct {
    int16_t x, y;   // Position in isometrischen Koordinaten
    uint8_t sprite_id;
} Player;

// Funktionen zur Spielersteuerung
void init_player(Player *player, uint8_t sprite_id);
void move_player(Player *player, int8_t dx, int8_t dy);
void update_player_sprite(Player *player);

#endif
