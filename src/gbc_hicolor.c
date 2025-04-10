#include <gbdk/platform.h>
#include <gbdk/incbin.h>
#include <gb/isr.h>

#include <gbc_hicolor.h>

// Macro for quickly picking the lesser of two values
#define MIN(A,B) ((A)<(B)?(A):(B))

/**
 * Internal variables for the HiColor system
 */
static uint16_t SP_SAVE;              // Used in the ISR for storing stack pointer temporarily
static uint8_t STAT_SAVE;             // Preserves the LCD status register
static uint8_t * p_hicolor_palettes;  // Pointer to the palette data used by the HiColor system
static uint8_t hicolor_palettes_bank; // ROM bank where the palette data resides
static uint8_t hicolor_last_scanline; // The last scanline that needs per-line palette updates

static uint16_t hicolor_palettes_cur_addr; // Internal palette buffer pointer used by the ISR


/**
 * @brief HiColor ISR (Interrupt Service Routine) that updates 4 CGB palettes
 *        each scanline, alternating between sets of palettes (0–3 vs. 4–7).
 *
 * The routine does the following:
 *  - Switches ROM bank if needed to access the palette data.
 *  - Performs a chunk of palette transfers right after VBlank (for the first line).
 *  - Then, for each scanline (during HBlank), loads the next set of palette data
 *    so each row of the screen can have a different set of palettes.
 *  - Continues until reaching hicolor_last_scanline, then restores the
 *    LCD status register.
 */
void hicolor_palette_isr(void) NONBANKED {

__asm

    ; ASM constants used during the palette updates per line
    PALETTES_PER_LINE       = #4 ; Number of palettes updated each scanline
    PAL_BYTES_PER_LINE      = #PALETTES_PER_LINE * #4 * #2 ; 4 colors per palette × 2 bytes each color
    BYTES_TO_NEXT_PAL_LINE  = #PAL_BYTES_PER_LINE * #2 + #1 + #1 - #4
    ; Explanation:
    ;  - #PAL_BYTES_PER_LINE * #2 is for reading each color (ld instructions)
    ;  - plus some overhead for instructions (halt/ret or partial execution),
    ;  - minus 4 because we preload some bytes first.

    BYTES_TO_PAL_AFTER_VBLANK_PAL = #8 * #4 * #2 * #2 + #1
    ; Explanation:
    ;  - 8 palettes, 4 colors each, 2 bytes per color,
    ;  - times 2 if we load them from memory,
    ;  - plus ret instructions or overhead.

    ;---------------------------------------
    ; 1) Switch to the correct ROM bank
    ;---------------------------------------
    ldh a, (__current_bank)
    push af
    ld a, (_hicolor_palettes_bank)
    or a
    jr z, 0$  ; If 0, skip setting the bank
    ldh (__current_bank), a
    ld (_rROMB0), a

0$: ; hicol__skip_set_bank_if_zero

    ;---------------------------------------
    ; 2) VBlank-phase palette load
    ;---------------------------------------
    ; Here we set up the return address after finishing the
    ; initial VBlank load. The code below actually runs from
    ; the palette buffer, so we push addresses onto the stack,
    ; effectively building a jump chain.

    ld hl, #1$            ; => hicol__vblank_load_pal_done_ret
    push hl               ; We'll return here after the vblank load

    ; Next, set HL to the start of the data for the vblank load code
    ld hl, #_p_hicolor_palettes
    ld a,  (hl+)
    ld h,  (hl)
    ld l,  a
    push   hl             ; push this so 'ret' in the load code jumps to it

    ; Save the buffer address in BC for the next step
    ld b, h
    ld c, l

    ; Enable BCPD autoincrement (CGB palette registers)
    ld a, #BCPSF_AUTOINC
    ld hl, #_BCPS_REG
    ld (hl+), a
    ret                   ; This 'ret' will jump into the palette load code

;---------------------------------------
; Return point after VBlank palette load
;---------------------------------------
1$:
    ; We come back here after the initial 8 palettes have been loaded post-VBlank

    ; Calculate the palette buffer's next address for the new line
    ld hl, #1            ; Just in case, ensuring HL is set
    ld a, c              ; c had the old pointer
    add  a, #BYTES_TO_PAL_AFTER_VBLANK_PAL
    ld   l, a
    ld   a, b
    adc  a, #0
    ld   h, a

    ;---------------------------------------
    ; Wait for Mode 3 (the scanline drawing)
    ;---------------------------------------
2$: ; ._hicol__vbl_wait_mode_3
    ldh a, (_STAT_REG)
    and #STATF_BUSY
    jr  z, 2$            ; Wait until the LCD is in Mode 3

    ldh a, (_STAT_REG)
    ld (_STAT_SAVE), a   ; Save the STAT register
    ld a, #STATF_MODE00
    ldh (_STAT_REG), a
    xor a
    ldh (_IF_REG), a

    ;---------------------------------------
    ; Skip first scanline buffer step
    ;---------------------------------------
    jr 3$  ; => hicol__scanline_load_start_from_vblank

;---------------------------------------
; Start of each new scanline load
;---------------------------------------
4$: ; hicol__scanline_load_start
    ; Reload the palette buffer address from hicolor_palettes_cur_addr
    ld a, (_hicolor_palettes_cur_addr + 0)
    add a, #BYTES_TO_NEXT_PAL_LINE
    ld l, a
    ld a, (_hicolor_palettes_cur_addr + 1)
    adc a, #0
    ld h, a

;---------------------------------------
; Start from the vblank computed address
;---------------------------------------
3$:
    ; Next return address after the palette load code finishes
    ld de, #5$     ; => hicol__hblank_load_pal_done_ret
    push de

    ; Preload the first 4 bytes from memory
    ld a, (hl+)
    ld b, a
    ld a, (hl+)
    ld c, a
    ld a, (hl+)
    ld d, a
    ld a, (hl+)
    ld e, a

    ; Then push HL as the next code address
    push hl

    ; Save the new pointer in hicolor_palettes_cur_addr
    ld a, l
    ld (_hicolor_palettes_cur_addr + 0), a
    ld a, h
    ld (_hicolor_palettes_cur_addr + 1), a

    xor a
    ldh (_IF_REG), a

    ; HL points to BCPD, used by the palette loader
    ld hl, #_BCPD_REG
    ret              ; Jump to the palette load code in memory

;---------------------------------------
; The code returns here after the HBlank palette load
;---------------------------------------
5$: ; hicol__hblank_load_pal_done_ret
    ld a, (_hicolor_last_scanline)
    ld c, a
    ldh a, (_LY_REG)
    cp c
    jr c, 4$   ; If we haven't reached the last scanline, load the next line's palette

    ; Otherwise, restore the STAT register and end
    ld a, (_STAT_SAVE)
    ldh (_STAT_REG), a
    xor a
    ldh (_IF_REG), a

    ; Restore the original bank
    pop af
    ldh (__current_bank), a
    ld (_rROMB0), a
    ret

__endasm;
}


/**
 * @brief Initializes the HiColor system:
 *        Loads tile patterns, map, and attributes for the HiColor image,
 *        then installs the custom LCD interrupt which updates the palette
 *        each scanline.
 *
 * @param p_hicolor     Pointer to the struct (generated by the png2hicolorgb tool)
 * @param hicolor_bank  The ROM bank where the HiColor data is stored
 */
void hicolor_start(const hicolor_data * p_hicolor, uint8_t hicolor_bank) NONBANKED {

    // Make sure we don't install the same ISR twice
    CRITICAL {
        remove_LCD(hicolor_palette_isr);
    }

    if (!p_hicolor) return;
    hicolor_palettes_bank = hicolor_bank;

    // Switch to the bank containing the HiColor data
    uint8_t bank_save = _current_bank;
    if (hicolor_bank) SWITCH_ROM(hicolor_bank);

    // Save the pointer to the palette data for the ISR
    p_hicolor_palettes = p_hicolor->p_palette;

    // The last scanline that needs updating
    // If the image is smaller than the full screen,
    // you might need to handle partial or leftover lines
    hicolor_last_scanline = (p_hicolor->height_in_tiles > DEVICE_SCREEN_HEIGHT)
        ? (DEVICE_SCREEN_PX_HEIGHT - 1)
        : ((p_hicolor->height_in_tiles << 3) - 1);

    // Load up to 256 tiles, set background map
    VBK_REG = VBK_BANK_0;
    set_bkg_data(0u,
                 MIN(p_hicolor->tile_count, 256),
                 p_hicolor->p_tiles);
    set_bkg_tiles(0u, 0u,
                  DEVICE_SCREEN_WIDTH,
                  p_hicolor->height_in_tiles,
                  p_hicolor->p_map);

    // If more than 256 tiles, load the rest in bank 1,
    // also set the attribute map
    VBK_REG = VBK_BANK_1;
    if (p_hicolor->tile_count > 256)
        set_bkg_data(0u,
                     (p_hicolor->tile_count - 256),
                     p_hicolor->p_tiles + (256 * 16));
    set_bkg_tiles(0,
                  0,
                  DEVICE_SCREEN_WIDTH,
                  p_hicolor->height_in_tiles,
                  p_hicolor->p_attribute_map);

    VBK_REG = VBK_BANK_0;

    // Restore previous bank
    SWITCH_ROM(bank_save);

    // Install and configure the HiColor ISR
    CRITICAL {
        LYC_REG = 152;       // When LY = 152, the ISR triggers
        STAT_REG = STATF_LYC;   // Enable LYC interrupt
        add_LCD(hicolor_palette_isr);  // Attach our custom HiColor ISR
    }

    // Ensure LCD interrupts are active
    set_interrupts(IE_REG | LCD_IFLAG);
}
