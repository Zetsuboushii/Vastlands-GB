#include <stdio.h>
#include <gb/gb.h>
#include "../include/player.h"
#include "../include/input.h"
#include "../include/sprite_data.h"

Player player;

void main() {
    printf("Knecht");
    // Sprite-Setup
    SPRITES_8x8;
    set_sprite_data(0, 1, spriteTiles); // Definiere ein einfaches Sprite

    // Spieler initialisieren
    init_player(&player, 0);

    SHOW_SPRITES;

    while (1) {
        int8_t dx, dy;

        // Eingabe verarbeiten
        process_input(&dx, &dy);

        // Spieler bewegen
        move_player(&player, dx, dy);

        // Sprite aktualisieren
        update_player_sprite(&player);

        // Kurze Pause zwischen den Frames
        delay(100);
    }
}
