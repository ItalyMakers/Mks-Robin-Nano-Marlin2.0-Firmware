/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "../../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>

#include "../../../../inc/MarlinConfig.h"

extern lv_group_t *g;
static lv_obj_t *scr;

enum {
  ID_LEVEL_RETURN = 1,
  ID_LEVEL_POSITION,
  ID_LEVEL_COMMAND,
  ID_LEVEL_ZOFFSET,
  ID_LEVEL_BLTOUCH,
  ID_LEVEL_TOUCHMI         
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  switch (obj->mks_obj_id) {
    case ID_LEVEL_RETURN:
      lv_clear_level_settings();
      lv_draw_return_ui();
      break;
    case ID_LEVEL_POSITION:
      lv_clear_level_settings();
      lv_draw_manual_level_pos_settings();
      break;
    case ID_LEVEL_COMMAND:
      keyboard_value = autoLevelGcodeCommand;
      lv_clear_level_settings();
      lv_draw_keyboard();
      break;
    #if HAS_BED_PROBE
      case ID_LEVEL_ZOFFSET:
        lv_clear_level_settings();
        lv_draw_auto_level_offset_settings();
        break;
    #endif
    #if ENABLED(BLTOUCH)
      case ID_LEVEL_BLTOUCH:
        last_disp_state = LEVELING_PARA_UI;
        lv_clear_level_settings();
        zoffset_do_init(false);
        lv_draw_zoffset_settings();
        break;
    #endif
    #if ENABLED(TOUCH_MI_PROBE)
      case ID_LEVEL_TOUCHMI:
        lv_clear_level_settings();
        lv_draw_touchmi_settings();
        break;
    #endif
  }
}

void lv_draw_level_settings(void) {
 
  scr = lv_screen_create(LEVELING_PARA_UI, machine_menu.LevelingParaConfTitle);
  lv_screen_menu_item(scr, machine_menu.LevelingManuPosConf, PARA_UI_POS_X, PARA_UI_POS_Y, event_handler, ID_LEVEL_POSITION, 0);
  lv_screen_menu_item(scr, machine_menu.LevelingAutoCommandConf, PARA_UI_POS_X, PARA_UI_POS_Y * 2, event_handler, ID_LEVEL_COMMAND, 1);
  #if HAS_BED_PROBE
    lv_screen_menu_item(scr, machine_menu.LevelingAutoZoffsetConf, PARA_UI_POS_X, PARA_UI_POS_Y * 3, event_handler, ID_LEVEL_ZOFFSET, 2);
  #endif
  #if ENABLED(BLTOUCH)
    lv_screen_menu_item(scr, machine_menu.BLTouchLevelingConf, PARA_UI_POS_X, PARA_UI_POS_Y * 4, event_handler, ID_LEVEL_BLTOUCH, 3);
  #endif
  #if ENABLED(TOUCH_MI_PROBE)
    lv_screen_menu_item(scr, machine_menu.LevelingTouchmiConf, PARA_UI_POS_X, PARA_UI_POS_Y * 4, event_handler, ID_LEVEL_TOUCHMI, 3);
  #endif
  lv_screen_menu_item_return(scr, event_handler, ID_LEVEL_RETURN);
}

void lv_clear_level_settings() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
