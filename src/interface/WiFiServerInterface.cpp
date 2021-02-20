/************************* WiFi Server Interface *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "WiFiServerInterface.h"

/**
 * WiFiServerInterface constructor.
 */
WiFiServerInterface::WiFiServerInterface(){
}

/**
 * WiFiServerInterface destructor.
 */
WiFiServerInterface::~WiFiServerInterface(){
}

/**
 * begin
 */
void WiFiServerInterface::begin(){
  this->m_server.begin();
}

/**
 * begin
 */
void WiFiServerInterface::begin(uint16_t port){
  this->m_server.begin(port);
}

/**
 * handleClient
 */
void WiFiServerInterface::handleClient(){
  this->m_server.handleClient();
}

/**
 * close
 */
void WiFiServerInterface::close(){
  this->m_server.close();
}

/**
 * stop
 */
void WiFiServerInterface::stop(){
  this->m_server.stop();
}

/**
 * on
 */
void WiFiServerInterface::on(const String &uri, THandlerFunction handler){
  this->m_server.on(uri, handler);
}

/**
 * on
 */
void WiFiServerInterface::on(const String &uri, HTTP_Method method, THandlerFunction fn){
  this->m_server.on(uri, method, fn);
}

// /**
//  * on
//  */
// void WiFiServerInterface::on(const String &uri, HTTP_Method method, THandlerFunction fn, THandlerFunction ufn){
//   this->m_server.on(uri, method, fn, ufn);
// }

/**
 * onNotFound
 * called when handler is not assigned
 */
void WiFiServerInterface::onNotFound(THandlerFunction fn){
  this->m_server.onNotFound(fn);
}

/**
 * onFileUpload
 * handle file uploads
 */
void WiFiServerInterface::onFileUpload(THandlerFunction fn){
  this->m_server.onFileUpload(fn);
}

/**
 * arg
 * get request argument value by name
 */
const String& WiFiServerInterface::arg(const String& name) const{
  return this->m_server.arg(name);
}

/**
 * arg
 * get request argument value by number
 */
const String& WiFiServerInterface::arg(int i) const{
  return this->m_server.arg(i);
}

/**
 * argName
 * get request argument name by number
 */
const String& WiFiServerInterface::argName(int i) const{
  return this->m_server.argName(i);
}

/**
 * args
 * get arguments count
 */
int WiFiServerInterface::args() const{
  return this->m_server.args();
}

/**
 * hasArg
 * check if argument exists
 */
bool WiFiServerInterface::hasArg(const String& name) const{
  return this->m_server.hasArg(name);
}

/**
 * collectHeaders
 * set the request headers to collect
 */
void WiFiServerInterface::collectHeaders(const char* headerKeys[], const size_t headerKeysCount){
  this->m_server.collectHeaders(headerKeys, headerKeysCount);
}

/**
 * header
 * get request header value by name
 */
const String& WiFiServerInterface::header(const String& name) const{
  return this->m_server.header(name);
}

/**
 * header
 * get request header value by number
 */
const String& WiFiServerInterface::header(int i) const{
  return this->m_server.header(i);
}

/**
 * headerName
 * get request header name by number
 */
const String& WiFiServerInterface::headerName(int i) const{
  return this->m_server.headerName(i);
}

/**
 * headers
 * get header count
 */
int WiFiServerInterface::headers() const{
  return this->m_server.headers();
}

/**
 * hasHeader
 * check if header exists
 */
bool WiFiServerInterface::hasHeader(const String& name) const{
  return this->m_server.hasHeader(name);
}

/**
 * hostHeader
 * get request host header if available or empty String if not
 */
const String& WiFiServerInterface::hostHeader() const{
  return this->m_server.hostHeader();
}


/**
 * send
 */
void WiFiServerInterface::send(int code, const char* content_type, const String& content){
  this->m_server.send(code, content_type, content);
}

/**
 * send
 */
void WiFiServerInterface::send(int code, char* content_type, const String& content){
  this->m_server.send(code, content_type, content);
}

/**
 * send
 */
void WiFiServerInterface::send(int code, const String& content_type, const String& content){
  this->m_server.send(code, content_type, content);
}

/**
 * send
 */
void WiFiServerInterface::send(int code, const char *content_type, const char *content){
  this->m_server.send(code, content_type, content);
}

/**
 * send
 */
void WiFiServerInterface::send(int code, const char *content_type, const char *content, size_t content_length){
  this->m_server.send(code, content_type, content, content_length);
}

/**
 * send
 */
void WiFiServerInterface::send(int code, const char *content_type, const uint8_t *content, size_t content_length){
  this->m_server.send(code, content_type, content, content_length);
}

/**
 * send_P
 */
void WiFiServerInterface::send_P(int code, PGM_P content_type, PGM_P content){
  this->m_server.send_P(code, content_type, content);
}

/**
 * send_P
 */
void WiFiServerInterface::send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength){
  this->m_server.send_P(code, content_type, content, contentLength);
}

/**
 * sendHeader
 */
void WiFiServerInterface::sendHeader(const String& name, const String& value, bool first){
  this->m_server.sendHeader(name, value, first);
}


WiFiServerInterface __wifi_server_interface;
