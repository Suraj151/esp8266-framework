/*********** SMTP (Simple Mail Transfer Protocol) Driver **********************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "SMTPDriver.h"


bool SMTPdriver::begin( WiFiClient* _client, char*_host, uint16_t _port ){

  this->client = _client;

  int _host_len = strlen(_host);
  this->host = new char[_host_len + 1];
  this->host[_host_len] = 0;
  this->responseBuffer = new char[SMTP_RESPONSE_BUFFER_SIZE];
  this->lastResponseCode = -1;

  memset( this->host, 0, _host_len + 1 );
  memset( this->responseBuffer, 0, SMTP_RESPONSE_BUFFER_SIZE );

  strcpy( this->host, _host );
  this->port = _port;

  int respcode = SMTP_STATUS_SERVICE_UNAVAILABLE;
  if( connectToServer( this->client, this->host, this->port, 3000 ) ){
    respcode = this->sendCommandAndGetCode( (char*)"" );
  }
  return respcode == SMTP_STATUS_SERVER_READY;
}

void SMTPdriver::readResponse(){

	char _crlfBuf[2];
	char _crlfFound, crlfCount=1;
  bool _bufferFull = false;

  if( this->responseReaderStatus == SMTP_RESPONSE_STARTING){
		_crlfFound = 0;
		memset(_crlfBuf, 0, 2);
		this->responseReaderStatus = SMTP_RESPONSE_WAITING;
	}

	if( millis() - this->lastReceive >= this->timeOut ){
		this->responseReaderStatus = SMTP_RESPONSE_TIMEOUT;
		return;
	}

  if( _bufferFull ) {
		this->responseReaderStatus = SMTP_RESPONSE_BUFFER_FULL;
		return;
	}

	int availableBytes = this->client->available();
	while(availableBytes) {

		char nextChar = (char)this->client->read();
		--availableBytes;
    if(this->responseBufferIndex < SMTP_RESPONSE_BUFFER_SIZE) {
			this->responseBuffer[this->responseBufferIndex++] = nextChar;
		}else{
      _bufferFull = true;
    }

		memmove(_crlfBuf, _crlfBuf + 1, 1);
		_crlfBuf[1] = nextChar;

		if(!strncmp(_crlfBuf, "\r\n", 2)) {

			if(++_crlfFound == crlfCount) {

        while ( this->client->available() ) {
          if(this->responseBufferIndex < SMTP_RESPONSE_BUFFER_SIZE) {
      			this->responseBuffer[this->responseBufferIndex++] = (char)this->client->read();
          }
          if( millis() - this->lastReceive >= this->timeOut ){
        		this->responseReaderStatus = SMTP_RESPONSE_TIMEOUT;
        		return;
        	}
          delay(0);
        }
				this->responseReaderStatus = SMTP_RESPONSE_FINISHED;
				return;
			}
		}
    delay(0);
	}
  delay(0);
}

void SMTPdriver::flushClient(){
	this->client->flush();
  uint32_t _now = millis();
	while(this->client->read() > -1 && (millis()-_now) < SMTP_DEFAULT_TIMEOUT );
}

void SMTPdriver::startReadResponse( uint16_t _timeOut ) {

  this->responseBufferIndex = 0;
	this->lastReceive = millis();
	this->responseReaderStatus = SMTP_RESPONSE_STARTING;
	this->timeOut = _timeOut;
	memset(this->responseBuffer, 0, SMTP_RESPONSE_BUFFER_SIZE);
}

void SMTPdriver::waitForResponse( uint16_t _timeOut ) {

	this->startReadResponse( _timeOut );
	do {
 		this->readResponse();
	} while(this->responseReaderStatus == SMTP_RESPONSE_WAITING);

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP status: "));
	Logln(this->responseReaderStatus);
	Log(F("SMTP response: "));
	Logln(this->responseBuffer);
	#endif
}

bool SMTPdriver::waitForExpectedResponse(	char* expectedResponse, uint16_t _timeOut ){

	this->waitForResponse( _timeOut );

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP espected response: "));
	Logln(expectedResponse);
	#endif
	return (this->responseReaderStatus != SMTP_RESPONSE_TIMEOUT) && (__strstr( (char*)this->responseBuffer, expectedResponse ) > 0);
}

bool SMTPdriver::sendCommandAndExpect( char* command, char* expectedResponse, uint16_t _timeOut ){

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending command: "));
	Logln(command);
	#endif
  if( !isConnected( this->client ) ){
    this->responseReaderStatus = SMTP_RESPONSE_CONN_ERR;
    return false;
  }
  this->flushClient();
  this->client->println( command );
	return this->waitForExpectedResponse( expectedResponse, _timeOut );
}

int SMTPdriver::sendCommandAndGetCode( PGM_P command, uint16_t _timeOut ){

  int respcode = -1;
  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending command: "));
	Logln(command);
	#endif
  if( isConnected( this->client ) ){

    this->flushClient();
    if( strlen_P(command) > 0 )
    this->client->println( command );
    this->waitForResponse( _timeOut );
    respcode = (int)StringToUint16( this->responseBuffer );
  }else{
    this->responseReaderStatus = SMTP_RESPONSE_CONN_ERR;
  }
  this->lastResponseCode = respcode;
	return respcode;
}

int SMTPdriver::sendCommandAndGetCode( char* command, uint16_t _timeOut ){

  int respcode = -1;
  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending command: "));
	Logln(command);
	#endif
  if( isConnected( this->client ) ){

    this->flushClient();
    if( strlen(command) > 0 )
    this->client->println( command );
    this->waitForResponse( _timeOut );
    respcode = (int)StringToUint16( this->responseBuffer );
  }else{
    this->responseReaderStatus = SMTP_RESPONSE_CONN_ERR;
  }
  this->lastResponseCode = respcode;
	return respcode;
}

bool SMTPdriver::sendHello( char* domain ){

  char* cmd = new char[100];

  memset( cmd, 0, 100 );
  strcpy_P( cmd, SMTP_COMMAND_EHLO );
  strcat( cmd, " " );
  strcat( cmd, domain );
  int respcode = this->sendCommandAndGetCode( cmd );

  delete[] cmd;
	return respcode == SMTP_STATUS_ACTION_COMPLETED;
}

bool SMTPdriver::sendAuthLogin( char* username, char* password ){

  char* _username = base64Encode( username, strlen(username) );
  char* _password = base64Encode( password, strlen(password) );

  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_AUTH );

  if( respcode < SMTP_STATUS_SERVICE_UNAVAILABLE ){
    int respcode = this->sendCommandAndGetCode( _username );
  }

  if( respcode < SMTP_STATUS_SERVICE_UNAVAILABLE ){
    int respcode = this->sendCommandAndGetCode( _password );
  }

  free(_username); free(_password);
	return respcode < SMTP_STATUS_SERVICE_UNAVAILABLE;
}

bool SMTPdriver::sendFrom( char* sender ){

  char* cmd = new char[128];

  memset( cmd, 0, 128 );
  strcpy_P( cmd, SMTP_COMMAND_FROM );
  strcat_P( cmd, SMTP_COMMAND_SEPARATOR );
  strcat_P( cmd, SMTP_COMMAND_OPENING_ANG_BRACKET );
  strcat( cmd, sender );
  strcat_P( cmd, SMTP_COMMAND_CLOSING_ANG_BRACKET );
  int respcode = this->sendCommandAndGetCode( cmd );

  delete[] cmd;
	return respcode == SMTP_STATUS_ACTION_COMPLETED;
}

bool SMTPdriver::sendTo( char* recipient ){

  char* cmd = new char[128];

  memset( cmd, 0, 128 );
  strcpy_P( cmd, SMTP_COMMAND_TO );
  strcat_P( cmd, SMTP_COMMAND_SEPARATOR );
  strcat_P( cmd, SMTP_COMMAND_OPENING_ANG_BRACKET );
  strcat( cmd, recipient );
  strcat_P( cmd, SMTP_COMMAND_CLOSING_ANG_BRACKET );
  int respcode = this->sendCommandAndGetCode( cmd );

  delete[] cmd;
	return respcode == SMTP_STATUS_ACTION_COMPLETED;
}

bool SMTPdriver::sendDataCommand(){

  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_DATA );
	return respcode == SMTP_STATUS_START_MAIL_INPUT;
}

void SMTPdriver::sendDataHeader( char* sender, char* recipient, char* subject ){

  this->client->print(SMTP_COMMAND_DATA_HEADER_TO);
  this->client->println(recipient);

  this->client->print(SMTP_COMMAND_DATA_HEADER_FROM);
  this->client->println(sender);

  this->client->print(SMTP_COMMAND_DATA_HEADER_SUBJECT);
  this->client->println(subject);
  this->client->print(SMTP_COMMAND_CRLF);
}

bool SMTPdriver::sendDataBody( String body ){

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending data: "));
	Logln(body);
	#endif
  // this->client->println( body );
  sendPacket( this->client, (uint8_t*)body.c_str(), body.length()+1 );
  this->client->println();
  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_DATA_TERMINATOR );
  return respcode < SMTP_STATUS_SERVICE_UNAVAILABLE;
}

bool SMTPdriver::sendDataBody( char* body ){

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending data: "));
	Logln(body);
	#endif
  // this->client->println( body );
  sendPacket( this->client, (uint8_t*)body, strlen(body) );
  this->client->println();
  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_DATA_TERMINATOR );
  return respcode < SMTP_STATUS_SERVICE_UNAVAILABLE;
}

bool SMTPdriver::sendDataBody( PGM_P body ){

  #ifdef EW_SERIAL_LOG
  Log(F("SMTP sending data: "));
	Logln(body);
	#endif
  this->client->println( body );
  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_DATA_TERMINATOR );
  return respcode < SMTP_STATUS_SERVICE_UNAVAILABLE;
}

bool SMTPdriver::sendQuit(){

  int respcode = this->sendCommandAndGetCode( SMTP_COMMAND_QUIT );
  return respcode == SMTP_STATUS_SERVER_ENDING_CON;
}

void SMTPdriver::end(){

  disconnect( this->client );
  this->client = NULL;
  if( this->host ) delete[] this->host;
  if( this->responseBuffer ) delete[] this->responseBuffer;
}
