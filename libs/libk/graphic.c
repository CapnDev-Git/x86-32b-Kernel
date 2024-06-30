#include <graphic.h>
#include <stdlib.h>

/*
 * offscreen buffer (for double buffering).
 */

static unsigned char offbuffer[FB_SIZE];

/*
 * the font is composed of 8*8 characters.
 */

static unsigned char font[2048] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x81, 0xA5, 0x81,
    0xBD, 0x99, 0x81, 0x7E, 0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,
    0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x10, 0x38, 0x7C, 0xFE,
    0x7C, 0x38, 0x10, 0x00, 0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x92, 0x10, 0x7C,
    0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C, 0x00, 0x00, 0x18, 0x3C,
    0x3C, 0x18, 0x00, 0x00, 0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,
    0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00, 0xFF, 0xC3, 0x99, 0xBD,
    0xBD, 0x99, 0xC3, 0xFF, 0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
    0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18, 0x3F, 0x33, 0x3F, 0x30,
    0x30, 0x70, 0xF0, 0xE0, 0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,
    0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99, 0x80, 0xE0, 0xF8, 0xFE,
    0xF8, 0xE0, 0x80, 0x00, 0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x66, 0x66, 0x66, 0x66,
    0x66, 0x00, 0x66, 0x00, 0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00,
    0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0x86, 0xFC, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0x7E, 0x7E, 0x00, 0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18,
    0x7E, 0x3C, 0x18, 0x00, 0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,
    0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
    0xC0, 0xFE, 0x00, 0x00, 0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00,
    0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7E,
    0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00, 0x6C, 0x6C, 0x6C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,
    0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00, 0x00, 0xC6, 0xCC, 0x18,
    0x30, 0x66, 0xC6, 0x00, 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00,
    0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x60, 0x60,
    0x60, 0x30, 0x18, 0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
    0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E,
    0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
    0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x18, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00,
    0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00, 0x30, 0x70, 0x30, 0x30,
    0x30, 0x30, 0xFC, 0x00, 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00,
    0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00, 0x1C, 0x3C, 0x6C, 0xCC,
    0xFE, 0x0C, 0x1E, 0x00, 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00,
    0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00, 0xFC, 0xCC, 0x0C, 0x18,
    0x30, 0x30, 0x30, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,
    0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x7E, 0x00,
    0x7E, 0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
    0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00, 0x7C, 0xC6, 0xDE, 0xDE,
    0xDC, 0xC0, 0x7C, 0x00, 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,
    0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00, 0x3C, 0x66, 0xC0, 0xC0,
    0xC0, 0x66, 0x3C, 0x00, 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,
    0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00, 0xFE, 0x62, 0x68, 0x78,
    0x68, 0x60, 0xF0, 0x00, 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00,
    0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00, 0x78, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x78, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,
    0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00, 0xF0, 0x60, 0x60, 0x60,
    0x62, 0x66, 0xFE, 0x00, 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
    0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00, 0x38, 0x6C, 0xC6, 0xC6,
    0xC6, 0x6C, 0x38, 0x00, 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,
    0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00, 0xFC, 0x66, 0x66, 0x7C,
    0x6C, 0x66, 0xE6, 0x00, 0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00,
    0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0xCC, 0xCC, 0xCC, 0xCC,
    0xCC, 0xCC, 0xFC, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
    0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00, 0xC6, 0xC6, 0x6C, 0x38,
    0x6C, 0xC6, 0xC6, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00,
    0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00, 0x78, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x78, 0x00, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
    0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 0x10, 0x38, 0x6C, 0xC6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0C,
    0x7C, 0xCC, 0x76, 0x00, 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00, 0x1C, 0x0C, 0x0C, 0x7C,
    0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x76, 0xCC,
    0xCC, 0x7C, 0x0C, 0xF8, 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,
    0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0x0C, 0x00, 0x1C, 0x0C,
    0x0C, 0xCC, 0xCC, 0x78, 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
    0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0x00, 0x00, 0xCC, 0xFE,
    0xFE, 0xD6, 0xD6, 0x00, 0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
    0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0xDC, 0x66,
    0x66, 0x7C, 0x60, 0xF0, 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,
    0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x7C, 0xC0,
    0x70, 0x1C, 0xF8, 0x00, 0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,
    0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0xCC, 0xCC,
    0xCC, 0x78, 0x30, 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
    0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0xCC, 0xCC,
    0xCC, 0x7C, 0x0C, 0xF8, 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
    0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00, 0x18, 0x18, 0x18, 0x00,
    0x18, 0x18, 0x18, 0x00, 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00,
    0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6C,
    0xC6, 0xC6, 0xFE, 0x00, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x0C, 0x06, 0x7C,
    0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x1C, 0x00, 0x78, 0xCC,
    0xFC, 0xC0, 0x78, 0x00, 0x7E, 0x81, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00,
    0xCC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 0xE0, 0x00, 0x78, 0x0C,
    0x7C, 0xCC, 0x76, 0x00, 0x30, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0xC0, 0x78, 0x0C, 0x38, 0x7E, 0x81, 0x3C, 0x66,
    0x7E, 0x60, 0x3C, 0x00, 0xCC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00,
    0xE0, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 0xCC, 0x00, 0x70, 0x30,
    0x30, 0x30, 0x78, 0x00, 0x7C, 0x82, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,
    0xE0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 0xC6, 0x10, 0x7C, 0xC6,
    0xFE, 0xC6, 0xC6, 0x00, 0x30, 0x30, 0x00, 0x78, 0xCC, 0xFC, 0xCC, 0x00,
    0x1C, 0x00, 0xFC, 0x60, 0x78, 0x60, 0xFC, 0x00, 0x00, 0x00, 0x7F, 0x0C,
    0x7F, 0xCC, 0x7F, 0x00, 0x3E, 0x6C, 0xCC, 0xFE, 0xCC, 0xCC, 0xCE, 0x00,
    0x78, 0x84, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0xCC, 0x00, 0x78,
    0xCC, 0xCC, 0x78, 0x00, 0x00, 0xE0, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00,
    0x78, 0x84, 0x00, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0xE0, 0x00, 0xCC,
    0xCC, 0xCC, 0x76, 0x00, 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
    0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00, 0xCC, 0x00, 0xCC, 0xCC,
    0xCC, 0xCC, 0x78, 0x00, 0x18, 0x18, 0x7E, 0xC0, 0xC0, 0x7E, 0x18, 0x18,
    0x38, 0x6C, 0x64, 0xF0, 0x60, 0xE6, 0xFC, 0x00, 0xCC, 0xCC, 0x78, 0x30,
    0xFC, 0x30, 0xFC, 0x30, 0xF8, 0xCC, 0xCC, 0xFA, 0xC6, 0xCF, 0xC6, 0xC3,
    0x0E, 0x1B, 0x18, 0x3C, 0x18, 0x18, 0xD8, 0x70, 0x1C, 0x00, 0x78, 0x0C,
    0x7C, 0xCC, 0x76, 0x00, 0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
    0x00, 0x1C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00, 0x00, 0x1C, 0x00, 0xCC,
    0xCC, 0xCC, 0x76, 0x00, 0x00, 0xF8, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0x00,
    0xFC, 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00, 0x3C, 0x6C, 0x6C, 0x3E,
    0x00, 0x7E, 0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C, 0x00, 0x00,
    0x18, 0x00, 0x18, 0x18, 0x30, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x0C, 0x0C, 0x00, 0x00,
    0xC6, 0xCC, 0xD8, 0x36, 0x6B, 0xC2, 0x84, 0x0F, 0xC3, 0xC6, 0xCC, 0xDB,
    0x37, 0x6D, 0xCF, 0x03, 0x18, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x18, 0x00,
    0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00, 0x00, 0xCC, 0x66, 0x33,
    0x66, 0xCC, 0x00, 0x00, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xDB, 0xF6, 0xDB, 0x6F,
    0xDB, 0x7E, 0xD7, 0xED, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x18,
    0xF8, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36, 0xF6, 0x36, 0x36, 0x36,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0x36, 0x36, 0x36, 0x00, 0x00, 0xF8, 0x18,
    0xF8, 0x18, 0x18, 0x18, 0x36, 0x36, 0xF6, 0x06, 0xF6, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0xFE, 0x06,
    0xF6, 0x36, 0x36, 0x36, 0x36, 0x36, 0xF6, 0x06, 0xFE, 0x00, 0x00, 0x00,
    0x36, 0x36, 0x36, 0x36, 0xFE, 0x00, 0x00, 0x00, 0x18, 0x18, 0xF8, 0x18,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36,
    0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x30, 0x3F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3F, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0xF7, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 0x00, 0x00, 0xFF, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x36, 0x36, 0xF7, 0x00, 0xF7, 0x36, 0x36, 0x36,
    0x18, 0x18, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x3F, 0x00, 0x00, 0x00, 0x18, 0x18, 0x1F, 0x18, 0x1F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xFF, 0x36, 0x36, 0x36,
    0x18, 0x18, 0xFF, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x18, 0x18,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xDC, 0xC8, 0xDC, 0x76, 0x00,
    0x00, 0x78, 0xCC, 0xF8, 0xCC, 0xF8, 0xC0, 0xC0, 0x00, 0xFC, 0xCC, 0xC0,
    0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0xFE, 0x6C, 0x6C, 0x6C, 0x6C, 0x00,
    0xFC, 0xCC, 0x60, 0x30, 0x60, 0xCC, 0xFC, 0x00, 0x00, 0x00, 0x7E, 0xD8,
    0xD8, 0xD8, 0x70, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x60, 0xC0,
    0x00, 0x76, 0xDC, 0x18, 0x18, 0x18, 0x18, 0x00, 0xFC, 0x30, 0x78, 0xCC,
    0xCC, 0x78, 0x30, 0xFC, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x6C, 0x38, 0x00,
    0x38, 0x6C, 0xC6, 0xC6, 0x6C, 0x6C, 0xEE, 0x00, 0x1C, 0x30, 0x18, 0x7C,
    0xCC, 0xCC, 0x78, 0x00, 0x00, 0x00, 0x7E, 0xDB, 0xDB, 0x7E, 0x00, 0x00,
    0x06, 0x0C, 0x7E, 0xDB, 0xDB, 0x7E, 0x60, 0xC0, 0x38, 0x60, 0xC0, 0xF8,
    0xC0, 0x60, 0x38, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
    0x00, 0x7E, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18,
    0x18, 0x00, 0x7E, 0x00, 0x60, 0x30, 0x18, 0x30, 0x60, 0x00, 0xFC, 0x00,
    0x18, 0x30, 0x60, 0x30, 0x18, 0x00, 0xFC, 0x00, 0x0E, 0x1B, 0x1B, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xD8, 0xD8, 0x70,
    0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00, 0x00, 0x76, 0xDC, 0x00,
    0x76, 0xDC, 0x00, 0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x0F, 0x0C, 0x0C, 0x0C, 0xEC, 0x6C, 0x3C, 0x1C,
    0x58, 0x6C, 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x70, 0x98, 0x30, 0x60,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void draw_begin(void) { draw_clear(CONS_BLACK); }

void draw_end(void) { swap_frontbuffer(offbuffer); }

void draw_clear(color_t color) { memset(offbuffer, color, FB_SIZE); }

void draw_pixel(unsigned int x, unsigned int y, color_t color) {
  if (x >= GRAPHIC_WIDTH)
    return;
  if (y >= GRAPHIC_HEIGHT)
    return;

  offbuffer[y * GRAPHIC_WIDTH + x] = color;
}

void draw_line(unsigned int x1, unsigned int y1, unsigned int x2,
               unsigned int y2, color_t color) {
  unsigned int i;

  if (x1 >= GRAPHIC_WIDTH)
    return;
  if (y1 >= GRAPHIC_HEIGHT)
    return;
  if (x2 >= GRAPHIC_WIDTH)
    return;
  if (y2 >= GRAPHIC_HEIGHT)
    return;

  if (x1 == x2) {
    for (i = y1; i < y2; i++)
      draw_pixel(x1, i, color);
  } else if (y1 == y2) {
    for (i = x1; i < x2; i++)
      draw_pixel(i, y1, color);
  } else {
    int s;
    int steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
      s = x1;
      x1 = y1;
      y1 = s;
      s = x2;
      x2 = y2;
      y2 = s;
    }
    if (x1 > x2) {
      s = x1;
      x1 = x2;
      x2 = s;
      s = y2;
      y2 = y1;
      y1 = s;
    }
    int deltax = x2 - x1;
    int deltay = abs(y2 - y1);
    int error = -deltax / 2;
    int ystep;
    int y = y1;

    if (y1 < y2)
      ystep = 1;
    else
      ystep = -1;
    unsigned int x;
    for (x = x1; x < x2; x++) {
      if (steep)
        draw_pixel(y, x, color);
      else
        draw_pixel(x, y, color);

      error = error + deltay;

      if (error > 0) {
        y = y + ystep;
        error = error - deltax;
      }
    }
  }
}

void draw_rect(unsigned int x1, unsigned int y1, unsigned int x2,
               unsigned int y2, color_t color) {
  unsigned int x;
  unsigned int y;

  for (x = x1; x < x2 && x < GRAPHIC_WIDTH; x++) {
    draw_pixel(x, y1, color);
    draw_pixel(x, y2, color);
  }
  for (y = y1; y < y2 && y < GRAPHIC_HEIGHT; y++) {
    draw_pixel(x1, y, color);
    draw_pixel(x2, y, color);
  }
}

void draw_fillrect(unsigned int x1, unsigned int y1, unsigned int x2,
                   unsigned int y2, color_t color, color_t interior) {
  unsigned int x;
  unsigned int y;

  for (x = x1; x < x2 && x < GRAPHIC_WIDTH; x++) {
    draw_pixel(x, y1, color);
    draw_pixel(x, y2, color);
  }
  for (y = y1; y <= y2 && y < GRAPHIC_HEIGHT; y++) {
    draw_pixel(x1, y, color);
    draw_pixel(x2, y, color);
  }
  for (x = x1 + 1; x < x2 && x < GRAPHIC_WIDTH; x++)
    for (y = y1 + 1; y < y2 && y < GRAPHIC_HEIGHT; y++)
      draw_pixel(x, y, interior);
}

/*
 * Windows BMP file header.
 */

struct bitmap_header {
  char signature[2];
  unsigned long filesize;
  unsigned long reserved1;
  unsigned long offset;
  unsigned long reserved2;
  unsigned long width;
  unsigned long height;
  unsigned short planes;
  unsigned short bpp;
  unsigned long reserved3;
  unsigned long size;
  char reserved[16];
} __attribute__((packed));

struct image *load_image(const char *path) {
  struct bitmap_header bmp;

  int fd = open(path, 0);
  if (fd < 0)
    return NULL;

  int rc = read(fd, &bmp, sizeof(bmp));
  if (rc < (int)sizeof(bmp)) {
    goto err_img;
  }

  if (!(bmp.signature[0] == 'B' && bmp.signature[1] == 'M')) {
    goto err_img;
  }

  struct image *img = malloc(sizeof(struct image));
  if (!img) {
    goto err_img;
  }

  img->width = bmp.width;
  img->height = bmp.height;

  img->data = calloc(img->height, sizeof(*img->data));
  if (!img->data)
    goto err_buf;

  for (unsigned int i = 0; i < img->height; i++) {
    img->data[i] = calloc(img->width, sizeof(*img->data[i]));
    if (!img->data[i])
      goto err;
  }

  int ppl = (bmp.size - (img->width * img->height)) / img->height;

  if (lseek(fd, bmp.offset, SEEK_SET) == (off_t)-1) {
    goto err;
  }

  for (unsigned int i = 0; i < img->height; i++) {
    rc = read(fd, img->data[i], img->width);
    if (rc < (int)img->width)
      goto err;

    rc = lseek(fd, ppl, SEEK_CUR);

    if (rc == (off_t)-1)
      goto err;
  }

  close(fd);

  return img;

err:
  for (unsigned int i = 0; i < img->height; i++)
    free(img->data[i]);
  free(img->data);
err_buf:
  free(img);
err_img:
  close(fd);
  return NULL;
}

void clear_image(struct image *image) {
  for (unsigned int i = 0; i < image->height; i++)
    free(image->data[i]);
  free(image->data);
  free(image);
}

void draw_image_alpha(struct image *image, unsigned int x, unsigned int y,
                      unsigned int alpha) {
  for (unsigned int i = 0; i < image->height; i++)
    for (unsigned int j = 0; j < image->width; j++) {
      if ((alpha == (unsigned int)-1) || (alpha != image->data[i][j]))
        draw_pixel(x + j, y + image->height - i, image->data[i][j]);
    }
}

void draw_image(struct image *image, unsigned int x, unsigned int y) {
  draw_image_alpha(image, x, y, -1);
}

/*
 * used to decompose a byte
 */

static int bit_on(char c, int n) {
  int mask;

  mask = 1 << (7 - n);
  return c & mask;
}

void draw_text(const char *s, unsigned int x, unsigned int y, color_t fg,
               color_t bg) {
  char c;
  char ch;
  char p;
  unsigned int pos;
  unsigned int strp = 0;

  for (; *s; s++, strp++) {
    c = *s;

    for (unsigned int i = 0; i < 8; ++i) {
      for (unsigned int j = 0; j < 8; ++j) {
        ch = font[c * 8 + i];
        p = bit_on(ch, j) ? fg : bg;

        pos = ((y + i) * GRAPHIC_WIDTH) + (strp * 8 + x) + j;
        if (!((bg == (unsigned int)-1) && (p == -1)))
          offbuffer[pos] = p;
      }
    }
  }
}

struct anim *load_anim(char *paths, int delay) {
  char *p, *bck;
  char *filename;
  struct anim *anim = NULL;
  int i;

  if (!paths || !*paths)
    return NULL;

  if (!(anim = malloc(sizeof(struct anim))))
    return NULL;

  anim->nr_img = 1;
  anim->current_img = 0;
  anim->delay = delay;
  anim->jiffies = 0;

  for (p = paths; *p; p++)
    if (*p == ' ')
      anim->nr_img++;

  if (!(anim->imgs = calloc(anim->nr_img, sizeof(struct image *)))) {
    free(anim);
    return NULL;
  }

  if (!(p = strdup(paths))) {
    free(anim->imgs);
    free(anim);
    return NULL;
  }
  bck = p;

  for (i = 0; i < anim->nr_img; i++) {
    filename = p;

    while (*p && *p != ' ')
      p++;

    *p = '\0';
    p++;

    if (!(anim->imgs[i] = load_image(filename)))
      blue_screen("failed to load animation");
  }

  free(bck);
  return anim;
}

void draw_anim(struct anim *anim, int x, int y, unsigned long jiffies) {
  if (anim->jiffies + anim->delay <= jiffies || anim->jiffies > jiffies) {
    anim->jiffies = jiffies;
    anim->current_img = (anim->current_img + 1) % anim->nr_img;
  }

  draw_image_alpha(anim->imgs[anim->current_img], x, y, 0);
}

static void blue_screen_cons(const char *message) {
  char seq[] = {CONS_ESCAPE, CONS_COLOR,
                CONS_BACK(CONS_BLUE) | CONS_FRONT(CONS_WHITE) | CONS_LIGHT,
                CONS_ESCAPE, CONS_CLEAR};
  char fatal[] = {
      CONS_ESCAPE, CONS_SETX,  32,
      CONS_ESCAPE, CONS_SETY,  10,
      CONS_ESCAPE, CONS_COLOR, CONS_BACK(CONS_WHITE) | CONS_FRONT(CONS_BLUE)};
  char msg[] = {CONS_ESCAPE,
                CONS_SETX,
                0,
                CONS_ESCAPE,
                CONS_SETY,
                13,
                CONS_ESCAPE,
                CONS_COLOR,
                CONS_BACK(CONS_BLUE) | CONS_FRONT(CONS_WHITE) | CONS_LIGHT};
  char *chiche1 = "If this is not the first time you encounter";
  char *chiche2 = "this problem, please contact chiche@epita.fr";

  write(seq, sizeof(seq) / sizeof(char));

  write(fatal, sizeof(fatal) / sizeof(char));
  printf("K -- FATAL ERROR", fatal);

  msg[2] = 40 - strlen(message) / 2;
  write(msg, sizeof(msg) / sizeof(char));
  printf("%s", message);
  msg[2] = 40 - strlen(chiche1) / 2;
  msg[5] += 2;
  write(msg, sizeof(msg) / sizeof(char));
  printf("%s", chiche1);
  msg[2] = 40 - strlen(chiche2) / 2;
  msg[5]++;
  write(msg, sizeof(msg) / sizeof(char));
  printf("%s", chiche2);

  while (1)
    continue;
}

static void blue_screen_fb(const char *message) {
  draw_begin();
  draw_clear(CONS_BLUE);

  draw_text("K -- FATAL ERROR", GRAPHIC_WIDTH / 2 - 8 * 8, 60, CONS_BLUE,
            CONS_WHITE);

  draw_text(message, GRAPHIC_WIDTH / 2 - (strlen(message) / 2) * 8, 90,
            CONS_WHITE, CONS_BLUE);

  draw_text("If this problem repeats,", GRAPHIC_WIDTH / 2 - 12 * 8, 120,
            CONS_WHITE, CONS_BLUE);
  draw_text("Please contact chiche@epita.fr", GRAPHIC_WIDTH / 2 - 15 * 8, 130,
            CONS_WHITE, CONS_BLUE);

  draw_end();

  while (1)
    continue;
}

void (*blue_screen)(const char *message) = blue_screen_cons;

void switch_graphic(void) {
  if (setvideo(VIDEO_GRAPHIC))
    blue_screen("Unable to switch to graphic mode");
  blue_screen = blue_screen_fb;
}

void switch_text(void) {
  if (setvideo(VIDEO_TEXT))
    blue_screen("Unable to switch to text mode");
  blue_screen = blue_screen_cons;
}
