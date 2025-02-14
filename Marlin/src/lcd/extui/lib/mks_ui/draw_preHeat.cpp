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

#include "../../../../module/temperature.h"
#include "../../../../inc/MarlinConfig.h"
#include "../../../../module/motion.h"

static lv_obj_t *scr;
extern lv_group_t*  g;
static lv_obj_t *buttonType, *buttonStep, *buttonAdd, *buttonDec;
static lv_obj_t *labelType;
static lv_obj_t *labelStep;
static lv_obj_t *tempText1;
static lv_obj_t *btn_pla;
static lv_obj_t *btn_abs;
static lv_obj_t *label_abs;
static lv_obj_t *label_pla;

static lv_style_t btn_style_pre;
static lv_style_t btn_style_rel;

enum {
  ID_P_ADD = 1,
  ID_P_DEC,
  ID_P_TYPE,
  ID_P_STEP,
  ID_P_OFF,
  ID_P_RETURN,
  ID_P_ABS,
  ID_P_PLA
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  switch (obj->mks_obj_id) {
    case ID_P_ADD:
      if (uiCfg.curTempType == 0) {
        #if HAS_HOTEND
          thermalManager.temp_hotend[uiCfg.curSprayerChoose].target += uiCfg.stepHeat;
          if (uiCfg.curSprayerChoose == 0) {
            if ((int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].target > (HEATER_0_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1))) {
              thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = (float)HEATER_0_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1);
            }
          }
        #endif
        #if DISABLED(SINGLENOZZLE) && HAS_MULTI_EXTRUDER
          else if ((int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].target > (HEATER_1_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1))) {
            thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = (float)HEATER_1_MAXTEMP - (WATCH_TEMP_INCREASE + TEMP_HYSTERESIS + 1);
          }
        #endif
        thermalManager.start_watching_hotend(uiCfg.curSprayerChoose);
      }
      #if HAS_HEATED_BED
        else {
          thermalManager.temp_bed.target += uiCfg.stepHeat;
          if ((int)thermalManager.temp_bed.target > BED_MAXTEMP - (WATCH_BED_TEMP_INCREASE + TEMP_BED_HYSTERESIS + 1)) {
            thermalManager.temp_bed.target = (float)BED_MAXTEMP - (WATCH_BED_TEMP_INCREASE + TEMP_BED_HYSTERESIS + 1);
          }
          thermalManager.start_watching_bed();
        }
      #endif
      disp_desire_temp();
      break;
    case ID_P_DEC:
      if (uiCfg.curTempType == 0) {
        #if HAS_HOTEND
          if ((int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].target > uiCfg.stepHeat)
            thermalManager.temp_hotend[uiCfg.curSprayerChoose].target -= uiCfg.stepHeat;
          else
            thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = 0;
          thermalManager.start_watching_hotend(uiCfg.curSprayerChoose);
        #endif
      }
      #if HAS_HEATED_BED
        else {
          if ((int)thermalManager.temp_bed.target > uiCfg.stepHeat)
            thermalManager.temp_bed.target -= uiCfg.stepHeat;
          else
            thermalManager.temp_bed.target = 0;

          thermalManager.start_watching_bed();
        }
      #endif
      disp_desire_temp();
      break;
    case ID_P_TYPE:
      if (uiCfg.curTempType == 0) {
        if (ENABLED(HAS_MULTI_EXTRUDER)) {
          #if DISABLED(SINGLENOZZLE)
            if (uiCfg.curSprayerChoose == 0) {
              uiCfg.curSprayerChoose = 1;
            }
            else if (uiCfg.curSprayerChoose == 1) {
              if (TEMP_SENSOR_BED != 0) {
                uiCfg.curTempType = 1;
              }
              else {
                uiCfg.curTempType      = 0;
                uiCfg.curSprayerChoose = 0;
              }
            }
          #else
            if (TEMP_SENSOR_BED != 0) {
              uiCfg.curTempType = 1;
            }
            else {
              uiCfg.curTempType = 0;
            }
          #endif
        }
        else if (uiCfg.curSprayerChoose == 0) {
          if (TEMP_SENSOR_BED != 0)
            uiCfg.curTempType = 1;
          else
            uiCfg.curTempType = 0;
        }
        // lv_obj_del(btn_pla);
        // lv_obj_del(btn_abs);
      }
      else if (uiCfg.curTempType == 1) {
        uiCfg.curSprayerChoose = 0;
        uiCfg.curTempType      = 0;
        // lv_obj_del(buttonAdd);
        // lv_obj_del(buttonDec);
        // disp_add_dec();
        // disp_ext_heart();
      }
      disp_temp_type();
      break;
    case ID_P_STEP:
      switch (uiCfg.stepHeat) {
        case  1: uiCfg.stepHeat =  5; break;
        case  5: uiCfg.stepHeat = 10; break;
        case 10: uiCfg.stepHeat =  1; break;
        default: break;
      }
      disp_step_heat();
      break;
    case ID_P_OFF:
      if (uiCfg.curTempType == 0) {
        #if HAS_HOTEND
          thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = 0;
          thermalManager.start_watching_hotend(uiCfg.curSprayerChoose);
        #endif
      }
      #if HAS_HEATED_BED
        else {
          thermalManager.temp_bed.target = 0;
          thermalManager.start_watching_bed();
        }
      #endif
      disp_desire_temp();
      break;
    case ID_P_RETURN:
      lv_clear_cur_ui();
      lv_draw_return_ui();
      break;
    case ID_P_ABS:
      if (uiCfg.curTempType == 0){
        thermalManager.setTargetHotend(PREHEAT_2_TEMP_HOTEND, 0);
      }
      else{
        thermalManager.setTargetBed(PREHEAT_2_TEMP_BED);
      }
      break;
    case ID_P_PLA:
      if (uiCfg.curTempType == 0){
        thermalManager.setTargetHotend(PREHEAT_1_TEMP_HOTEND, 0);
      }
      else{
        thermalManager.setTargetBed(PREHEAT_1_TEMP_BED);
      }
      break;
  }
}

void lv_draw_preHeat(void) {

  scr = lv_screen_create(PRE_HEAT_UI);

  disp_add_dec();

  buttonType = lv_imgbtn_create(scr, nullptr, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_TYPE);
  buttonStep = lv_imgbtn_create(scr, nullptr, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_STEP);
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) {
      lv_group_add_obj(g, buttonType);
      lv_group_add_obj(g, buttonStep);
    }
  #endif

  lv_big_button_create(scr, "F:/bmp_zero_temp.bin", preheat_menu.off, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_OFF);
  lv_big_button_create(scr, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_P_RETURN);

  // if(uiCfg.curTempType == 0) {
  // }
    disp_ext_heart();

  // Create labels on the image buttons
  labelType = lv_label_create_empty(buttonType);
  labelStep = lv_label_create_empty(buttonStep);

  #if ENABLED(SINGLENOZZLE)
		uiCfg.curSprayerChoose = 0;
	#else
		uiCfg.curSprayerChoose = active_extruder;
  #endif

  disp_temp_type();
  disp_step_heat();

  tempText1 = lv_label_create_empty(scr);
  lv_obj_set_style(tempText1, &tft_style_label_rel);
  disp_desire_temp();
}

void disp_add_dec() {

  // Create image buttons
  buttonAdd = lv_big_button_create(scr, "F:/bmp_Add.bin", preheat_menu.add, INTERVAL_V, titleHeight, event_handler, ID_P_ADD);
  buttonDec = lv_big_button_create(scr, "F:/bmp_Dec.bin", preheat_menu.dec, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_P_DEC);
}

void disp_ext_heart() {

    btn_abs = lv_btn_create(scr, 160, 40, 80, 40, event_handler, ID_P_ABS);
    btn_pla = lv_btn_create(scr, 260, 40, 80, 40, event_handler, ID_P_PLA);

    lv_btn_set_style(btn_abs, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_abs, LV_BTN_STYLE_REL, &btn_style_rel);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_REL, &btn_style_rel);

    label_abs = lv_label_create(btn_abs, PREHEAT_2_LABEL);
    label_pla = lv_label_create(btn_pla, PREHEAT_1_LABEL);
}

void dis_ext_heart_change(uint8_t mode) {

  if(mode == 0) {
    btn_style_pre.body.opa = 0;
    lv_btn_set_style(btn_abs, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_abs, LV_BTN_STYLE_REL, &btn_style_rel);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_REL, &btn_style_rel);
  }else {
    btn_style_pre.body.opa = 255;
    lv_btn_set_style(btn_abs, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_abs, LV_BTN_STYLE_REL, &btn_style_rel);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_PR, &btn_style_pre);
    lv_btn_set_style(btn_pla, LV_BTN_STYLE_REL, &btn_style_rel);
  }
}

void disp_temp_type() {
  if (uiCfg.curTempType == 0) {

    #if DISABLED(SINGLENOZZLE)
      if (uiCfg.curSprayerChoose == 1) {
        lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru2.bin");
        if (gCfgItems.multiple_language) {
          lv_label_set_text(labelType, preheat_menu.ext2);
          lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
      }
      else {
        lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru1.bin");
        if (gCfgItems.multiple_language) {
          lv_label_set_text(labelType, preheat_menu.ext1);
          lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
        }
      }
    #else
      lv_imgbtn_set_src_both(buttonType, "F:/bmp_extru1.bin");
      if (gCfgItems.multiple_language) {
        lv_label_set_text(labelType, preheat_menu.ext1);
        lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
      }
    #endif
  }
  else {
    lv_imgbtn_set_src_both(buttonType, "F:/bmp_bed.bin");
    if (gCfgItems.multiple_language) {
      lv_label_set_text(labelType, preheat_menu.hotbed);
      lv_obj_align(labelType, buttonType, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
  }
}

void disp_desire_temp() {
  char buf[20] = {0};

  public_buf_l[0] = '\0';

  if (uiCfg.curTempType == 0) {
    #if HAS_HOTEND
      #if DISABLED(SINGLENOZZLE)
        strcat(public_buf_l, uiCfg.curSprayerChoose < 1 ? preheat_menu.ext1 : preheat_menu.ext2);
        sprintf(buf, preheat_menu.value_state, (int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].celsius,  (int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].target);
      #else
        strcat(public_buf_l, preheat_menu.ext1);
        sprintf(buf, preheat_menu.value_state, (int)thermalManager.temp_hotend[0].celsius,  (int)thermalManager.temp_hotend[0].target);
      #endif
    #endif
  }
  #if HAS_HEATED_BED
    else {
      strcat(public_buf_l, preheat_menu.hotbed);
      sprintf(buf, preheat_menu.value_state, (int)thermalManager.temp_bed.celsius,  (int)thermalManager.temp_bed.target);
    }
  #endif
  strcat_P(public_buf_l, PSTR(": "));
  strcat(public_buf_l, buf);
  lv_label_set_text(tempText1, public_buf_l);
  lv_obj_align(tempText1, nullptr, LV_ALIGN_CENTER, 0, -50);
}

void disp_step_heat() {
  if (uiCfg.stepHeat == 1) {
    lv_imgbtn_set_src_both(buttonStep, "F:/bmp_step1_degree.bin");
  }
  else if (uiCfg.stepHeat == 5) {
    lv_imgbtn_set_src_both(buttonStep, "F:/bmp_step5_degree.bin");
  }
  else if (uiCfg.stepHeat == 10) {
    lv_imgbtn_set_src_both(buttonStep, "F:/bmp_step10_degree.bin");
  }

  if (gCfgItems.multiple_language) {
    if (uiCfg.stepHeat == 1) {
      lv_label_set_text(labelStep, preheat_menu.step_1c);
      lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if (uiCfg.stepHeat == 5) {
      lv_label_set_text(labelStep, preheat_menu.step_5c);
      lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
    else if (uiCfg.stepHeat == 10) {
      lv_label_set_text(labelStep, preheat_menu.step_10c);
      lv_obj_align(labelStep, buttonStep, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }
  }
}

void lv_clear_preHeat() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
