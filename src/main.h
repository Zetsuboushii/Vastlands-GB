#include <stdint.h>
#include <stdbool.h>


#ifndef MAIN_H
#define MAIN_H

extern uint8_t two_frame_counter, two_frame_real_value;

bool is_valid_tile(uint16_t x, uint16_t y);

#endif //MAIN_H
