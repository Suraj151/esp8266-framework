/************************ Dynamic html tag builder ****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_HTML_PAGE_HELPER_H_
#define _EW_HTML_PAGE_HELPER_H_

#include <Arduino.h>
#include <utility/Utility.h>

/**
 * @var html open close tags
 */
static const char HTML_TR_OPEN_TAG   []PROGMEM = "<tr>";
static const char HTML_TR_CLOSE_TAG  []PROGMEM = "</tr>";
static const char HTML_TD_OPEN_TAG   []PROGMEM = "<td>";
static const char HTML_TD_CLOSE_TAG  []PROGMEM = "</td>";
static const char HTML_INPUT_OPEN    []PROGMEM = "<input ";
static const char HTML_SELECT_OPEN   []PROGMEM = "<select ";
static const char HTML_SELECT_CLOSE  []PROGMEM = "</select>";
static const char HTML_OPTION_OPEN   []PROGMEM = "<option ";
static const char HTML_OPTION_CLOSE  []PROGMEM = "</option>";
static const char HTML_DIV_OPEN_TAG  []PROGMEM =  "<div";
static const char HTML_DIV_CLOSE_TAG []PROGMEM =  "</div>";
static const char HTML_H2_OPEN_TAG   []PROGMEM = "<h2>";
static const char HTML_H2_CLOSE_TAG  []PROGMEM = "</h2>";

/**
 * @var html attributes
 */
static const char HTML_MAXLEN_ATTR   []PROGMEM = " maxlength=";
static const char HTML_TYPE_ATTR     []PROGMEM = " type=";
static const char HTML_NAME_ATTR     []PROGMEM = " name=";
static const char HTML_VALUE_ATTR    []PROGMEM = " value=";
static const char HTML_STYLE_ATTR    []PROGMEM = " style=";
static const char HTML_CLASS_ATTR    []PROGMEM = " class=";
static const char HTML_SELECTED_ATTR []PROGMEM =  " selected='selected' ";
static const char HTML_CHECKED_ATTR  []PROGMEM =  " checked='checked' ";
static const char HTML_DISABLED_ATTR  []PROGMEM =  " disabled ";

static const char HTML_SUCCESS_FLASH []PROGMEM =  "Config saved Successfully";

/**
 * @var html tag types
 */
#define HTML_INPUT_TEXT_TAG_TYPE "text"
#define HTML_INPUT_CHECKBOX_TAG_TYPE "checkbox"
#define HTML_INPUT_TAG_DEFAULT_MAXLENGTH  20

/**
 * @var flash message enum
 */
enum FLASH_MSG_TYPE {
  ALERT_DANGER,
  ALERT_SUCCESS,
  ALERT_WARNING
};

void concat_tr_input_html_tags( char* _page, PGM_P _label, PGM_P _name, char* _value, int _maxlength=HTML_INPUT_TAG_DEFAULT_MAXLENGTH, char* _type=HTML_INPUT_TEXT_TAG_TYPE, bool _checked=false, bool _disabled=false );

void concat_tr_input_html_tags( char* _page, char* _label, char* _name, char* _value, int _maxlength=HTML_INPUT_TAG_DEFAULT_MAXLENGTH, char* _type=HTML_INPUT_TEXT_TAG_TYPE, bool _checked=false, bool _disabled=false );

void concat_tr_select_html_tags( char* _page, char* _label, char* _name, char** _options, int _size, int _selected, int _exception=0, bool _disabled=false );

void concat_tr_select_html_tags( char* _page, PGM_P _label, PGM_P _name, char** _options, int _size, int _selected, int _exception=0, bool _disabled=false );

void concat_tr_header_html_tags( char* _page, PGM_P _header );

void concat_flash_message_div( char* _page, PGM_P _message, int _status );

void concat_flash_message_div( char* _page, char* _message, int _status );

void concat_graph_axis_title_div( char* _page, char* _title, char* _style="" );

#endif
