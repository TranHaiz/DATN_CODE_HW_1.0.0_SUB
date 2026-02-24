/* Simple touch-driven menu module using z_touch_XPT2046 and display APIs */
#ifndef __Z_TOUCH_XPT2046_MENU_H
#define __Z_TOUCH_XPT2046_MENU_H

#include "z_displ_ILI9XXX.h"

#include <stdbool.h>
#include <stdint.h>

typedef void (*menu_btn_cb_t)(void);

typedef struct
{
  uint16_t      x, y, w, h;
  const char   *label;
  menu_btn_cb_t cb;
} menu_btn_t;

typedef struct
{
  menu_btn_t *buttons;
  uint8_t     max_buttons;
  uint8_t     btn_count;
} menu_config_t;

/** Initialize menu with pre-allocated button array (no dynamic alloc).
 *  The provided array must remain valid for the lifetime of the menu.
 */
void menu_init(menu_config_t *cfg, menu_btn_t *btn_array, uint8_t max_buttons);

/** Add a button (stored in pre-allocated array). */
bool menu_add_button(menu_btn_t *btn);

/** Render all buttons (simple filled rect + label). */
void menu_render(void);

/** Poll touch and invoke callbacks if a button was pressed. */
void menu_process_touch(void);

#endif
/*
 * z_touch_XPT2046_menu.h
 *
 *  Created on: 13 giu 2022
 *      Author: mauro
 *
 *  licensing: https://github.com/maudeve-it/ILI9XXX-XPT2046-STM32/blob/c097f0e7d569845c1cf98e8d930f2224e427fd54/LICENSE
 *
 */

#ifndef INC_Z_TOUCH_XPT2046_MENU_H_
#define INC_Z_TOUCH_XPT2046_MENU_H_

// Menu data definitions and declarations

typedef struct
{
  uint16_t X;  // position and size of the menu element
  uint16_t Y;
  uint16_t W;
  uint16_t H;
  char     Desc[20];  // text of the menu item
  sFONT    font;      // font to be used
  uint8_t  fontSize;
  uint16_t BkgUnsel;  // background color when item is not selected
  uint16_t BorUnsel;  // border color when item is not selected
  uint16_t InkUnsel;  // text color when item is not selected
  uint16_t BkgSel;    // background color with item selected
  uint16_t BorSel;    // border color with item selected
  uint16_t InkSel;    // text color with item selected
} sMenuItem;

void    InitMenu();
void    DrawMenu(sMenuItem *menu, uint8_t menusize);
uint8_t CheckMenu(sMenuItem *menu, uint8_t menusize);
void    RunMenu2();
void    RunMenu1();

#endif /* INC_Z_TOUCH_XPT2046_MENU_H_ */
