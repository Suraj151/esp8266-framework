#include "DynamicPageBuildHelper.h"

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

void concat_graph_axis_title_div( char* _page, char* _title, char* _style ){

  strcat_P( _page, HTML_DIV_OPEN_TAG );
  strcat_P( _page, HTML_STYLE_ATTR );
  strcat( _page, "'" );
  strcat( _page, _style );
  strcat( _page, ";'>" );
  strcat( _page, _title );
  strcat_P( _page, HTML_DIV_CLOSE_TAG );
}
