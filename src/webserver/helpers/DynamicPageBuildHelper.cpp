/************************ Dynamic html tag builder ****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "DynamicPageBuildHelper.h"

/**
 * build and append tr input html tag to html page. It build html tr/input tag with
 * given type, name, value, label etc. attributes.
 * this function uses program memory arguments to optimise ram
 *
 * @param	char*	_page
 * @param	PGM_P _label
 * @param	PGM_P _name
 * @param	char*	_value
 * @param	int   _maxlength
 * @param	char*	_type
 * @param	bool  _checked
 */
void concat_tr_input_html_tags( char* _page, PGM_P _label, PGM_P _name, char* _value, int _maxlength, char* _type, bool _checked ){

  bool _is_checkbox = false;
  char _maxlen[7]; memset(_maxlen, 0, 7);
  itoa( _maxlength, _maxlen, 10 );

  if( 0 <= __strstr( _type, HTML_INPUT_CHECKBOX_TAG_TYPE, 10 ) )
    _is_checkbox = true;

  strcat_P( _page, HTML_TR_OPEN_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, _label );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_INPUT_OPEN );
  strcat_P( _page, HTML_TYPE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _type );
  strcat( _page, "'" );

  if( !_is_checkbox ){
    strcat_P( _page, HTML_MAXLEN_ATTR );
    strcat( _page, "'" );
    strcat( _page, _maxlen );
    strcat( _page, "'" );
  }

  if( _checked ){
    strcat_P( _page, HTML_CHECKED_ATTR );
  }

  strcat_P( _page, HTML_NAME_ATTR );
  strcat( _page, "'" );
  strcat_P( _page, _name );
  strcat( _page, "'" );
  strcat_P( _page, HTML_VALUE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _value );
  strcat_P( _page, PSTR("'/>") );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TR_CLOSE_TAG );
}

/**
 * build and append tr input html tag to html page. It build html tr/input tag with
 * given type, name, value, label etc. attributes
 *
 * @param	char*	_page
 * @param	char* _label
 * @param	char* _name
 * @param	char*	_value
 * @param	int   _maxlength
 * @param	char*	_type
 * @param	bool  _checked
 */
void concat_tr_input_html_tags( char* _page, char* _label, char* _name, char* _value, int _maxlength, char* _type, bool _checked ){

  bool _is_checkbox = false;
  char _maxlen[7]; memset(_maxlen, 0, 7);
  itoa( _maxlength, _maxlen, 10 );

  if( 0 <= __strstr( _type, HTML_INPUT_CHECKBOX_TAG_TYPE, 10 ) )
    _is_checkbox = true;

  strcat_P( _page, HTML_TR_OPEN_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat( _page, _label );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_INPUT_OPEN );
  strcat_P( _page, HTML_TYPE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _type );
  strcat( _page, "'" );

  if( !_is_checkbox ){
    strcat_P( _page, HTML_MAXLEN_ATTR );
    strcat( _page, "'" );
    strcat( _page, _maxlen );
    strcat( _page, "'" );
  }

  if( _checked ){
    strcat_P( _page, HTML_CHECKED_ATTR );
  }

  strcat_P( _page, HTML_NAME_ATTR );
  strcat( _page, "'" );
  strcat( _page, _name );
  strcat( _page, "'" );
  strcat_P( _page, HTML_VALUE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _value );
  strcat_P( _page, PSTR("'/>") );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TR_CLOSE_TAG );
}

/**
 * build and append tr select html tag to html page. It build html tr/select tag with
 * given name, options, label etc. attributes
 *
 * @param	char*	_page
 * @param	char* _label
 * @param	char* _name
 * @param	char**	_options
 * @param	int   _size
 * @param	int   _selected
 * @param	int   _exception
 */
void concat_tr_select_html_tags( char* _page, char* _label, char* _name, char** _options, int _size, int _selected, int _exception ){

  strcat_P( _page, HTML_TR_OPEN_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat( _page, _label );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_SELECT_OPEN );
  strcat_P( _page, HTML_NAME_ATTR );
  strcat( _page, "'" );
  strcat( _page, _name );
  strcat( _page, "'>" );

  for (int i = 0; i < _size; i++) {

    if( strlen(_options[i]) > 0 && ( ( i+1 ) != _exception ) ){
      char buf[3];
      memset( buf, 0, 3 );
      itoa( i+1, buf, 10 );
      strcat_P( _page, HTML_OPTION_OPEN );
      strcat_P( _page, HTML_VALUE_ATTR );
      strcat( _page, "'" );
      strcat( _page, buf );
      strcat( _page, "'" );
      if( _selected == i+1 )
      strcat_P( _page, HTML_SELECTED_ATTR );
      strcat( _page, ">" );
      strcat( _page, _options[i] );
      strcat_P( _page, HTML_OPTION_CLOSE );
    }
  }
  strcat_P( _page, HTML_SELECT_CLOSE );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TR_CLOSE_TAG );
}

/**
 * build and append tr select html tag to html page. It build html tr/select tag with
 * given name, options, label etc. attributes
 * this function uses program memory arguments to optimise ram
 *
 * @param	char*	_page
 * @param	PGM_P _label
 * @param	PGM_P _name
 * @param	char**	_options
 * @param	int   _size
 * @param	int   _selected
 * @param	int   _exception
 */
void concat_tr_select_html_tags( char* _page, PGM_P _label, PGM_P _name, char** _options, int _size, int _selected, int _exception ){

  strcat_P( _page, HTML_TR_OPEN_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, _label );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_SELECT_OPEN );
  strcat_P( _page, HTML_NAME_ATTR );
  strcat( _page, "'" );
  strcat_P( _page, _name );
  strcat( _page, "'>" );

  for (int i = 0; i < _size; i++) {

    if( strlen(_options[i]) > 0 && ( ( i+1 ) != _exception ) ){
      char buf[3];
      memset( buf, 0, 3 );
      itoa( i+1, buf, 10 );
      strcat_P( _page, HTML_OPTION_OPEN );
      strcat_P( _page, HTML_VALUE_ATTR );
      strcat( _page, "'" );
      strcat( _page, buf );
      strcat( _page, "'" );
      if( _selected == i+1 )
      strcat_P( _page, HTML_SELECTED_ATTR );
      strcat( _page, ">" );
      strcat( _page, _options[i] );
      strcat_P( _page, HTML_OPTION_CLOSE );
    }
  }
  strcat_P( _page, HTML_SELECT_CLOSE );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TR_CLOSE_TAG );
}

/**
 * build and append tr header html tag to html page. It build html tr/header tag with
 * inner html attributes
 * this function uses program memory arguments to optimise ram
 *
 * @param	char*	_page
 * @param	PGM_P _header
 */
void concat_tr_header_html_tags( char* _page, PGM_P _header ){

  strcat_P( _page, HTML_TR_OPEN_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_H2_OPEN_TAG );
  strcat_P( _page, _header );
  strcat_P( _page, HTML_H2_CLOSE_TAG );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TD_OPEN_TAG );
  strcat_P( _page, HTML_TD_CLOSE_TAG );
  strcat_P( _page, HTML_TR_CLOSE_TAG );
}

/**
 * build and append div html tag to html page. It build html div tag with inner html
 * this function uses program memory arguments to optimise ram
 *
 * @param	char*	_page
 * @param	PGM_P _message
 * @param	int   _status
 */
void concat_flash_message_div( char* _page, PGM_P _message, int _status ){

  strcat_P( _page, HTML_DIV_OPEN_TAG );
  strcat_P( _page, HTML_CLASS_ATTR );
  strcat_P( _page, PSTR("'msg'") );
  strcat_P( _page, HTML_STYLE_ATTR );
  strcat_P( _page, PSTR("'background:") );
  strcat_P( _page, _status==ALERT_DANGER ? PSTR("#ffb2c7"): _status==ALERT_SUCCESS ? PSTR("#a6eaa8") : PSTR("#f9dc87") );
  strcat_P( _page, PSTR(";'>") );
  strcat_P( _page, _message );
  strcat_P( _page, HTML_DIV_CLOSE_TAG );
}

/**
 * build and append div html tag to html page. It build html div tag with inner html
 *
 * @param	char*	_page
 * @param	char* _message
 * @param	int   _status
 */
void concat_flash_message_div( char* _page, char* _message, int _status ){

  strcat_P( _page, HTML_DIV_OPEN_TAG );
  strcat_P( _page, HTML_CLASS_ATTR );
  strcat_P( _page, PSTR("'msg'") );
  strcat_P( _page, HTML_STYLE_ATTR );
  strcat_P( _page, PSTR("'background:") );
  strcat_P( _page, _status==ALERT_DANGER ? PSTR("#ffb2c7"): _status==ALERT_SUCCESS ? PSTR("#a6eaa8") : PSTR("#f9dc87") );
  strcat_P( _page, PSTR(";'>") );
  strcat( _page, _message );
  strcat_P( _page, HTML_DIV_CLOSE_TAG );
}

/**
 * build and append div html tag for graph axis title.
 *
 * @param	char*	_page
 * @param	char* _title
 * @param	char* _style
 */
void concat_graph_axis_title_div( char* _page, char* _title, char* _style ){

  strcat_P( _page, HTML_DIV_OPEN_TAG );
  strcat_P( _page, HTML_STYLE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _style );
  strcat( _page, ";'>" );
  strcat( _page, _title );
  strcat_P( _page, HTML_DIV_CLOSE_TAG );
}
