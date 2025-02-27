#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include <gbc_hicolor.h>

#include <titlescreen.h>
#include <player.h>
#include <bkg.h>
#include <palettes.h>
#include <stdio.h>
#include <gbdk/emu_debug.h>

#define LAST_TILE_INDEX 255u
const uint8_t empty_tile_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

uint8_t key_state, key_state_prev;
#define UPDATE_KEYS() (key_state_prev = key_state, key_state = joypad())
#define KEY_NEW_PRESS(mask) ((key_state & ~key_state_prev) & (mask))
#define KEY_IS_PRESSED(mask) (key_state & (mask))

#define TILEMAP_WIDTH (bkg_WIDTH >> 3)
#define TILEMAP_HEIGHT (bkg_HEIGHT >> 3)

#define PLAYER_SPEED 30

typedef struct {
    int x;
    int y;
} Vector8;

const hicolor_data* img_data;
uint8_t img_bank;

typedef struct {
    uint8_t bank;
    const void* data;
} banked_ptr_t;

const banked_ptr_t image = {
    BANK(titlescreen), &HICOLOR_VAR(titlescreen)
};

bool is_title_shown = false;

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


const Vector8 directionsForTwoFrameObjects[7] = {
    {0, 1}, // Down
    {0, 0},
    {0, -1}, // Up,
    {0, 0},
    {-1, 0}, // Left
    {0, 0},
    {1, 0}, // Right
};

uint8_t joypadCurrent = 0;
uint8_t twoFrameCounter = 0;
uint8_t twoFrameRealValue = 0;

void UpdateTwoFrameCounter() {
    twoFrameCounter += 3;
    twoFrameRealValue = twoFrameCounter >> 4;

    if (twoFrameRealValue >= 2) {
        twoFrameRealValue = 0;
        twoFrameCounter = 0;
    }
}


uint8_t playerDirection = 0;
uint16_t playerX, playerY;
uint16_t nextPlayerX, nextPlayerY;

metasprite_t const* playerMetasprite;

bool WorldPositionIsSolid(uint16_t x, uint16_t y) {
    EMU_printf("%u, %u", x, y);

    uint16_t column = x / 16;

    if (column >= TILEMAP_WIDTH) return true;

    uint16_t row = y / 16;

    if (row >= TILEMAP_HEIGHT) return true;

    uint16_t tilemapIndex = column + row * TILEMAP_WIDTH;

    bool tileIsSolid = false;

    uint8_t tilesetTile = bkg_map[tilemapIndex];

    tileIsSolid = (tilesetTile != 0x04);


    return tileIsSolid;
}

void SetupPlayer() {
    set_sprite_data(0, player_TILE_COUNT, player_tiles);

    playerX = 150 << 4;
    playerY = 40 << 4;

    playerDirection = J_DOWN;

    playerMetasprite = player_metasprites[0];
}

uint8_t UpdatePlayer() {
    uint8_t playerLastDirection = playerDirection;
    int8_t playerDirectionX = 0, playerDirectionY = 0;
    uint8_t playerMoving = false;
    nextPlayerX = playerX;
    nextPlayerY = playerY;

    if (joypadCurrent & J_LEFT) {
        nextPlayerX -= 1;
        playerDirection = J_LEFT;
        playerDirectionX = -1;
        playerMoving = true;
    }
    if (joypadCurrent & J_RIGHT) {
        nextPlayerX += 1;
        playerDirection = J_RIGHT;
        playerDirectionX = 1;
        playerMoving = true;
    }
    if (joypadCurrent & J_UP) {
        nextPlayerY -= 1;
        playerDirection = J_UP;
        playerDirectionY = -1;
        playerMoving = true;
    }
    if (joypadCurrent & J_DOWN) {
        nextPlayerY += 1;
        playerDirection = J_DOWN;
        playerDirectionY = 1;
        playerMoving = true;
    }

    if (playerMoving) {
        if (playerDirection != playerLastDirection) {
            switch (playerDirection) {
            case J_DOWN:
                set_sprite_data(0, player_TILE_COUNT, player_tiles);
                playerMetasprite = player_metasprites[0];
                break;
            case J_UP:
                set_sprite_data(0, player_TILE_COUNT, player_tiles);
                playerMetasprite = player_metasprites[1];
                break;
            case J_LEFT:
                set_sprite_data(0, player_TILE_COUNT, player_tiles);
                playerMetasprite = player_metasprites[2];
                break;
            case J_RIGHT:
                set_sprite_data(0, player_TILE_COUNT, player_tiles);
                playerMetasprite = player_metasprites[3];
                break;
            default:
                break;
            }
        }

        if (playerDirectionX != 0) {
            bool solid =
                WorldPositionIsSolid(nextPlayerX + playerDirectionX * 8, playerY - 8) ||
                WorldPositionIsSolid(nextPlayerX + playerDirectionX * 8, playerY) ||
                WorldPositionIsSolid(nextPlayerX + playerDirectionX * 8, playerY + 8);
            if (!solid) {
                playerX += nextPlayerX;
            }
        }
        if (playerDirectionY != 0) {
            bool solid =
                WorldPositionIsSolid(playerX + 8, nextPlayerY + playerDirectionY * 8) ||
                WorldPositionIsSolid(playerX, nextPlayerY + playerDirectionY * 8) ||
                WorldPositionIsSolid(playerX - 8, nextPlayerY + playerDirectionY * 8);
            if (!solid) {
                playerY += nextPlayerY;
            }
        }
    }

    return move_metasprite(playerMetasprite, 0, 0, playerX >> 4, playerY >> 4);
}


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

        SetupPlayer();

        while (true) {
            joypadCurrent = joypad();

            UpdateTwoFrameCounter();

            uint8_t lastSprite = 0;

            lastSprite += UpdatePlayer();

            hide_sprites_range(lastSprite, 40);

            set_bkg_data(0, bkg_TILE_COUNT, bkg_tiles);
            set_bkg_palette(0, bkg_PALETTE_COUNT, bkg_palettes);
            VBK_REG = 1;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map_attributes);
            VBK_REG = 0;
            set_bkg_tiles(0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT, bkg_map);


            set_sprite_palette(0,palettes_PALETTE_COUNT, palettes_palettes);

            wait_vbl_done();
        }
    }
}
