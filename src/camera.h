#include <gbdk/platform.h>
#include <stdint.h>
#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include <gbdk/console.h>
#include <player_down.h>
#include <bkg.h>
#include <gb/hardware.h>


#ifndef CAMERA_H
#define CAMERA_H

#define bkg_MAP_WIDTH (bkg_WIDTH / bkg_TILE_W)
#define bkg_MAP_HEIGHT (bkg_HEIGHT / bkg_TILE_H)

#define CAMERA_MAX_Y ((bkg_MAP_HEIGHT - DEVICE_SCREEN_HEIGHT) * 8)
#define CAMERA_MAX_X ((bkg_MAP_WIDTH - DEVICE_SCREEN_WIDTH) * 8)

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

extern uint16_t camera_x, camera_y;

void init_camera(uint8_t x, uint8_t y);
void set_camera(void);

#endif //CAMERA_H
