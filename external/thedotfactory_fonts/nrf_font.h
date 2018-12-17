#pragma once
#include "stdint.h"

typedef struct {
  uint8_t widthBits;                  /**< Width in bits (or pixels) of the character. */
  uint16_t offset;                    /**< Offset of the character's bitmap, in bytes, into the FONT_INFO's data array. */
  } FONT_CHAR_INFO;

typedef struct {
  uint8_t height;                     /**< Height in bits (or pixels), of the font's characters. */
  uint8_t startChar;                  /**< The first character in the font (e.g. in charInfo and data). */
  uint8_t endChar;                    /**< The last character in the font. */
  uint8_t spacePixels;                /**< Number of pixels that the space character takes up. */
  const FONT_CHAR_INFO * charInfo;    /**< Pointer to array of char information. */
  const uint8_t * data;               /**< Pointer to a generated array of character visual representation. */
  } FONT_INFO;
