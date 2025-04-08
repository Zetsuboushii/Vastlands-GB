//
// Created by zetsu on 06.04.25.
//

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