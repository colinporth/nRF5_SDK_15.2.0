#pragma once

#include <nrfx.h>

typedef enum {
  NRF_LCD_ROTATE_0 = 0,       /**< Rotate 0 degrees, clockwise. */
  NRF_LCD_ROTATE_90,          /**< Rotate 90 degrees, clockwise. */
  NRF_LCD_ROTATE_180,         /**< Rotate 180 degrees, clockwise. */
  NRF_LCD_ROTATE_270          /**< Rotate 270 degrees, clockwise. */
  } nrf_lcd_rotation_t;

typedef struct {
  nrfx_drv_state_t state;         /**< State of LCD instance. */
  uint16_t height;                /**< LCD height. */
  uint16_t width;                 /**< LCD width. */
  nrf_lcd_rotation_t rotation;    /**< LCD rotation. */
  } lcd_cb_t;

typedef struct {
  //{{{
  /**
   * @brief Function for initializing the LCD controller.
   */
  ret_code_t (* lcd_init)(void);
  //}}}

  //{{{
  /**
   * @brief Function for uninitializing the LCD controller.
   */
  void (* lcd_uninit)(void);
  //}}}
  //{{{
  /**
   * @brief Function for drawing a single pixel.
   *
   * @param[in] x             Horizontal coordinate of the pixel.
   * @param[in] y             Vertical coordinate of the pixel.
   * @param[in] color         Color of the pixel in LCD accepted format.
   */
  void (* lcd_pixel_draw)(uint16_t x, uint16_t y, uint32_t color);
  //}}}
  //{{{
  /**
   * @brief Function for drawing a filled rectangle.
   *
   * @param[in] x             Horizontal coordinate of the point where to start drawing the rectangle.
   * @param[in] y             Vertical coordinate of the point where to start drawing the rectangle.
   * @param[in] width         Width of the image.
   * @param[in] height        Height of the image.
   * @param[in] color         Color with which to fill the rectangle in LCD accepted format.
   */
  void (* lcd_rect_draw)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
  //}}}
   //{{{
   /**
    * @brief Function for displaying data from an internal frame buffer.
    *
    * This function may be used when functions for drawing do not write directly to
    * LCD but to an internal frame buffer. It could be implemented to write data from this
    * buffer to LCD.
   */
   void (* lcd_display)(void);
   //}}}
  //{{{
  /**
   * @brief Function for rotating the screen.
   *
   * @param[in] rotation      Rotation as enumerated value.
   */
  void (* lcd_rotation_set)(nrf_lcd_rotation_t rotation);
  //}}}
  //{{{
  /**
   * @brief Function for setting inversion of colors on the screen.
   *
   * @param[in] invert        If true, inversion will be set.
   */
  void (* lcd_display_invert)(bool invert);
  //}}}
  lcd_cb_t * p_lcd_cb;
  } nrf_lcd_t;
