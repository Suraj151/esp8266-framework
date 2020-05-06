/******************************* Email service *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_EMAIL_SERVICE)

#include "EmailServiceProvider.h"


/**
 * begin email service with wifi and client
 *
 * @param ESP8266WiFiClass*	_wifi
 * @param WiFiClient*	      _wifi_client
 */
void EmailServiceProvider::begin( ESP8266WiFiClass* _wifi, WiFiClient* _wifi_client ){

  this->wifi = _wifi;
  this->wifi_client = _wifi_client;
  this->_mail_handler_cb_id = 0;
}

/**
 * handle emails
 *
 */
void EmailServiceProvider::handleEmail(){

  this->_mail_handler_cb_id = __task_scheduler.updateInterval(
    this->_mail_handler_cb_id,
    [&]() {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling email"));
      #endif
      String _payload = "";

      // _payload += "MIME-Version: 1.0\n";
      // _payload += "Content-Type: multipart/mixed;";
      // _payload += "boundary=\"jncvdcsjnss\"\r\n\n";
      //
      // _payload += "--jncvdcsjnss\n";
      // _payload += "Content-Type: text/plain; charset=utf-8\n";
      // _payload += "Content-Transfer-Encoding: quoted-printable\r\n\n";
      //
      // _payload += "this is test file.\r\n\n";
      //
      // _payload += "--jncvdcsjnss\n";
      // _payload += "Content-Type: text/plain; name=test.txt\n";
      // _payload += "Content-Transfer-Encoding: base64\n";
      // _payload += "Content-Disposition: attachment; filename=test.txt\r\n\n";
      //
      // _payload += "dGVzdCBmaWxlIAo=\r\n\n";
      //
      // _payload += "--jncvdcsjnss--\n";

      #ifdef ENABLE_GPIO_SERVICE

      __gpio_service.appendGpioJsonPayload( _payload );
      #endif

      _payload += "\n\nHello from Esp\n";
      _payload += this->wifi->macAddress();

      if( this->sendMail( _payload ) ){
        __task_scheduler.clearInterval(this->_mail_handler_cb_id);
        this->_mail_handler_cb_id = 0;
      }
    },
    180000
  );
}


// /**
//  * This template send email and return the result of operation
//  *
//  * @param	  T*	mail_body
//  * @return  bool
//  */
// template <typename T> bool EmailServiceProvider::sendMail ( T mail_body ){
//
//   email_config_table _email_config = __database_service.get_email_config_table();
//
//   bool ret = false;
//
//   if( __status_wifi.wifi_connected && __status_wifi.internet_available &&
//     strlen(_email_config.mail_host) > 0 && strlen(_email_config.sending_domain) > 0 &&
//     strlen(_email_config.mail_from) > 0 && strlen(_email_config.mail_to) > 0
//   ){
//
//     ret = this->smtp.begin( this->wifi_client, _email_config.mail_host, _email_config.mail_port );
//     if( ret ){
//       ret = this->smtp.sendHello( _email_config.sending_domain );
//     }
//     if( ret ){
//       ret = this->smtp.sendAuthLogin( _email_config.mail_username, _email_config.mail_password );
//     }
//     if( ret ){
//       ret = this->smtp.sendFrom( _email_config.mail_from );
//     }
//     if( ret ){
//       ret = this->smtp.sendTo( _email_config.mail_to );
//     }
//     if( ret ){
//       ret = this->smtp.sendDataCommand();
//     }
//     if( ret ){
//       this->smtp.sendDataHeader( _email_config.mail_from_name, _email_config.mail_to, _email_config.mail_subject );
//       ret = this->smtp.sendDataBody( mail_body );
//     }
//     if( ret ){
//       ret = this->smtp.sendQuit();
//     }
//
//     this->smtp.end();
//   }
//
//   _ClearObject(&_email_config);
//   return ret;
// }

/**
 * send email and return the result of operation
 *
 * @param	  String*	mail_body
 * @return  bool
 */
bool EmailServiceProvider::sendMail( String mail_body ){

  email_config_table _email_config = __database_service.get_email_config_table();

  bool ret = false;

  if( __status_wifi.wifi_connected && __status_wifi.internet_available &&
    strlen(_email_config.mail_host) > 0 && strlen(_email_config.sending_domain) > 0 &&
    strlen(_email_config.mail_from) > 0 && strlen(_email_config.mail_to) > 0
  ){

    ret = this->smtp.begin( this->wifi_client, _email_config.mail_host, _email_config.mail_port );
    if( ret ){
      ret = this->smtp.sendHello( _email_config.sending_domain );
    }
    if( ret ){
      ret = this->smtp.sendAuthLogin( _email_config.mail_username, _email_config.mail_password );
    }
    if( ret ){
      ret = this->smtp.sendFrom( _email_config.mail_from );
    }
    if( ret ){
      ret = this->smtp.sendTo( _email_config.mail_to );
    }
    if( ret ){
      ret = this->smtp.sendDataCommand();
    }
    if( ret ){
      this->smtp.sendDataHeader( _email_config.mail_from_name, _email_config.mail_to, _email_config.mail_subject );
      ret = this->smtp.sendDataBody( mail_body );
    }
    if( ret ){
      ret = this->smtp.sendQuit();
    }

    this->smtp.end();
  }

  _ClearObject(&_email_config);
  return ret;
}

/**
 * send email and return the result of operation
 *
 * @param	  char*	mail_body
 * @return  bool
 */
bool EmailServiceProvider::sendMail( char* mail_body ){

  email_config_table _email_config = __database_service.get_email_config_table();

  bool ret = false;

  if( __status_wifi.wifi_connected && __status_wifi.internet_available &&
    strlen(_email_config.mail_host) > 0 && strlen(_email_config.sending_domain) > 0 &&
    strlen(_email_config.mail_from) > 0 && strlen(_email_config.mail_to) > 0
  ){

    ret = this->smtp.begin( this->wifi_client, _email_config.mail_host, _email_config.mail_port );
    if( ret ){
      ret = this->smtp.sendHello( _email_config.sending_domain );
    }
    if( ret ){
      ret = this->smtp.sendAuthLogin( _email_config.mail_username, _email_config.mail_password );
    }
    if( ret ){
      ret = this->smtp.sendFrom( _email_config.mail_from );
    }
    if( ret ){
      ret = this->smtp.sendTo( _email_config.mail_to );
    }
    if( ret ){
      ret = this->smtp.sendDataCommand();
    }
    if( ret ){
      this->smtp.sendDataHeader( _email_config.mail_from_name, _email_config.mail_to, _email_config.mail_subject );
      ret = this->smtp.sendDataBody( mail_body );
    }
    if( ret ){
      ret = this->smtp.sendQuit();
    }

    this->smtp.end();
  }

  _ClearObject(&_email_config);
  return ret;
}

/**
 * send email and return the result of operation
 *
 * @param	  PGM_P	mail_body
 * @return  bool
 */
bool EmailServiceProvider::sendMail( PGM_P mail_body ){

  email_config_table _email_config = __database_service.get_email_config_table();

  bool ret = false;

  if( __status_wifi.wifi_connected && __status_wifi.internet_available ){

    ret = this->smtp.begin( this->wifi_client, _email_config.mail_host, _email_config.mail_port );
    if( ret ){
      ret = this->smtp.sendHello( _email_config.sending_domain );
    }
    if( ret ){
      ret = this->smtp.sendAuthLogin( _email_config.mail_username, _email_config.mail_password );
    }
    if( ret ){
      ret = this->smtp.sendFrom( _email_config.mail_from );
    }
    if( ret ){
      ret = this->smtp.sendTo( _email_config.mail_to );
    }
    if( ret ){
      ret = this->smtp.sendDataCommand();
    }
    if( ret ){
      this->smtp.sendDataHeader( _email_config.mail_from_name, _email_config.mail_to, _email_config.mail_subject );
      ret = this->smtp.sendDataBody( mail_body );
    }
    if( ret ){
      ret = this->smtp.sendQuit();
    }

    this->smtp.end();
  }

  _ClearObject(&_email_config);
  return ret;
}

#ifdef EW_SERIAL_LOG
/**
 * print email configs
 */
void EmailServiceProvider::printEmailConfigLogs(){

  email_config_table _email_config = __database_service.get_email_config_table();

  Logln(F("\nEmail Configs :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));

  Log(_email_config.sending_domain); Log("\t");
  Log(_email_config.mail_host); Log("\t");
  Log(_email_config.mail_port); Log("\t");
  Log(_email_config.mail_username); Log("\t");
  Log(_email_config.mail_password); Log("\t\n");
  Log(_email_config.mail_from); Log("\t");
  Log(_email_config.mail_from_name); Log("\t");
  Log(_email_config.mail_to); Log("\t");
  Log(_email_config.mail_subject); Log("\t");
  Logln(_email_config.mail_frequency);
}
#endif

EmailServiceProvider __email_service;

#endif
