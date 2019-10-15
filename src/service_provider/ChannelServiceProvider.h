#ifndef _CHANNEL_SERVICE_PROVIDER_H_
#define _CHANNEL_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <config/Config.h>

#define CHANNEL_BUFFER_LENGTH 100

#define CLIENT_BYTE_READ_TIME 3

typedef struct {
  IPAddress host_ip;
  int port;
  uint8_t* in_buffer;
  uint8_t* out_buffer;
  uint16_t buffer_length;
} channel_t;

/**
 * ChannelServiceProvider class
 */
class ChannelServiceProvider{

  public:

    /**
     * ChannelServiceProvider constructor.
     */
    ChannelServiceProvider(){

    }

    /**
     * ChannelServiceProvider destructor.
     */
    ~ChannelServiceProvider(){

      this->delete_channels();
    }

    void delete_channels( void ){

      if(this->wifi_client != NULL) {
        delete this->wifi_client;
        this->wifi_client = NULL;
      }
      if(this->wifi_server != NULL) {
        delete this->wifi_server;
        this->wifi_server = NULL;
      }

      delete[] this->server_channel.in_buffer;
      delete[] this->server_channel.out_buffer;
      delete[] this->client_channel.in_buffer;
      delete[] this->client_channel.out_buffer;
    }

    void beginChannel( ESP8266WiFiClass* _wifi, int _port ){

      #ifdef EW_SERIAL_LOG
      Logln(F("Channel: begin Connection"));
      #endif
      this->wifi= _wifi;

      this->server_channel.buffer_length = CHANNEL_BUFFER_LENGTH;
      this->server_channel.in_buffer = new uint8_t[this->server_channel.buffer_length + 1];
      this->server_channel.out_buffer = new uint8_t[this->server_channel.buffer_length + 1];
      memset(this->server_channel.in_buffer, 0, this->server_channel.buffer_length + 1);
      memset(this->server_channel.out_buffer, 0, this->server_channel.buffer_length + 1);
      this->server_channel.port = _port;

      this->client_channel.buffer_length = CHANNEL_BUFFER_LENGTH;
      this->client_channel.in_buffer = new uint8_t[this->client_channel.buffer_length + 1];
      this->client_channel.out_buffer = new uint8_t[this->client_channel.buffer_length + 1];
      memset(this->client_channel.in_buffer, 0, this->client_channel.buffer_length + 1);
      memset(this->client_channel.out_buffer, 0, this->client_channel.buffer_length + 1);
      this->client_channel.port = _port;

      this->handleChannelConnections();
    }

    void handleChannelConnections(void){

      if( this->wifi->isConnected() && this->wifi->localIP().isSet() ){

        this->client_channel.host_ip = this->wifi->gatewayIP();
        if ( !this->wifi_client ) {
          this->wifi_client = new WiFiClient;
        }
        if( !this->connected(this->wifi_client) ) this->connectServer();
      }

      WiFiMode_t _wifi_mode = this->wifi->getMode();

      if( _wifi_mode == WIFI_AP || _wifi_mode == WIFI_AP_STA ){

        if ( !this->wifi_server ) {
          this->wifi_server = new WiFiServer(this->server_channel.port);
        }
        if( this->wifi_server->status() == CLOSED ) this->wifi_server->begin();
      }
    }

    bool send_to_server( char *buffer ){
      uint16_t _buf_len = strlen(buffer);
      memcpy(this->client_channel.out_buffer, buffer, _buf_len < this->client_channel.buffer_length ? _buf_len : this->client_channel.buffer_length );
    }

    void broadcastToClients(void){

      unsigned char number_client;
      struct station_info *stat_info;

      #ifdef ENABLE_NAPT_FEATURE
      struct ip_addr *IPaddress;
      #else
      struct ip4_addr *IPaddress;
      #endif

      IPAddress address;
      int i=1;

      number_client= wifi_softap_get_station_num(); // Count of stations which are connected to ESP8266 soft-AP
      stat_info = wifi_softap_get_station_info();

      #ifdef EW_SERIAL_LOG
      Log(F(" Total connected_clients : "));
      Logln(number_client);
      #endif

      while (stat_info != NULL) {

        IPaddress = &stat_info->ip;
        address = IPaddress->addr;

        #ifdef EW_SERIAL_LOG
        Log(F("client: ")); Log(i);
        Log(F(" ip address: ")); Log((address));
        Log(F(" mac address: "));
        Log_format(stat_info->bssid[0],HEX);
        Log_format(stat_info->bssid[1],HEX);
        Log_format(stat_info->bssid[2],HEX);
        Log_format(stat_info->bssid[3],HEX);
        Log_format(stat_info->bssid[4],HEX);
        Logln_format(stat_info->bssid[5],HEX);
        #endif

        stat_info = STAILQ_NEXT(stat_info, next);
        i++;
      }
    }

    void handleServerChannelCommunication(void){

      // Check if a client has connected
      WiFiClient client = this->wifi_server->available();
      if ( client ){
        #ifdef EW_SERIAL_LOG
        Logln(F("Channel: client connected"));
        #endif
        client.setTimeout(500);
        this->readChannelPacket( (void*)&client, this->server_channel.in_buffer, this->server_channel.buffer_length, 500 );
        if( strlen((char*)this->server_channel.in_buffer) > 0 ){
          #ifdef EW_SERIAL_LOG
          Log(F("Channel: Reading from "));Log(client.remoteIP());Log(F(" : "));Logln(client.remotePort());
          for (uint16_t i = 0; i < strlen((char*)this->server_channel.in_buffer); i++) {
            Log((char)this->server_channel.in_buffer[i]);
          }
          memset(this->server_channel.in_buffer, 0, this->server_channel.buffer_length + 1);
          Logln();
          #endif
        }
        memcpy(this->server_channel.out_buffer, "ok", 2 );
      }

      if( strlen((char*)this->server_channel.out_buffer) > 0 ){
        this->sendChannelPacket( (void*)&client, this->server_channel.out_buffer, this->server_channel.buffer_length );
        memset((char*)this->server_channel.out_buffer, 0, this->server_channel.buffer_length + 1);
        // this->disconnect(&client);
      }

    }

    void handleClientChannelCommunication(void){

      if( strlen((char*)this->client_channel.out_buffer) > 0 ){
        this->sendChannelPacket( (void*)this->wifi_client, this->client_channel.out_buffer, this->client_channel.buffer_length );
        memset((char*)this->client_channel.out_buffer, 0, this->client_channel.buffer_length + 1);
        // this->disconnect(this->wifi_client);
      }

      this->readChannelPacket( (void*)this->wifi_client, this->client_channel.in_buffer, this->client_channel.buffer_length, 500 );
      if( strlen((char*)this->client_channel.in_buffer) > 0 ){
        #ifdef EW_SERIAL_LOG
        Log(F("Channel: Reading from "));Log(this->wifi_client->remoteIP());Log(F(" : "));Logln(this->wifi_client->remotePort());
        for (uint16_t i = 0; i < strlen((char*)this->client_channel.in_buffer); i++) {
          Log((char)this->client_channel.in_buffer[i]);
        }
        memset(this->client_channel.in_buffer, 0, this->client_channel.buffer_length + 1);
        Logln();
        #endif
      }
    }

    void handleCommunication(void){

      this->handleServerChannelCommunication();
      this->handleClientChannelCommunication();
    }

    /*   WiFi client support functions */

    bool connectServer() {

      #ifdef EW_SERIAL_LOG
      Log(F("Channel: Connecting to "));Log(this->client_channel.host_ip);Log(F(" : "));Logln(this->client_channel.port);
      #endif
      this->wifi_client->setTimeout(500);
      // this->wifi_client->keepAlive();
      int result = this->wifi_client->connect(this->client_channel.host_ip, this->client_channel.port);
      #ifdef EW_SERIAL_LOG
      Log(F("Channel: Connect result -: ")); Logln(result);
      #endif
      return result != 0;
    }

    bool disconnect( WiFiClient* _client ) {

      return this->connected(_client) ? _client->stop(500) : true;
    }

    bool connected( WiFiClient* _client ) {

      return _client ? _client->connected() : false;
    }

    uint16_t readChannelPacket( void* _client, uint8_t *buffer, uint16_t maxlen, int16_t timeout ) {

      uint16_t len = 0;
      int16_t t = timeout;
      WiFiClient* client = (WiFiClient*)_client;

      // #ifdef EW_SERIAL_LOG
      // Log(F("Channel: Reading from "));Log(client->remoteIP());Log(F(" : "));Logln(client->remotePort());
      // Log(F("Channel: Reading packets : "));
      // #endif
      while ( this->connected(client) && ( timeout >= 0 ) ) {

        while ( client->available() ) {

          char c = client->read();
          timeout = t;  // reset the timeout
          buffer[len] = c;
          len++;
          // #ifdef EW_SERIAL_LOG
          // Log(c);
          // #endif

          if( maxlen == 0 ){
            // #ifdef EW_SERIAL_LOG
            // Logln();
            // #endif
            return 0;
          }

          if( len == maxlen ){
            // #ifdef EW_SERIAL_LOG
            // Logln();
            // #endif
            return len;
          }
        }
        timeout -= CLIENT_BYTE_READ_TIME;
        delay(CLIENT_BYTE_READ_TIME);
      }
      // #ifdef EW_SERIAL_LOG
      // Logln();
      // #endif
      return len;
    }

    // bool sendChannelPacket_flush( void* _client, uint8_t *buffer, uint16_t len ) {
    //   this->sendChannelPacket( _client, buffer, len );
    //   memset(buffer, 0, len);
    // }

    bool sendChannelPacket( void* _client, uint8_t *buffer, uint16_t len ) {
      uint16_t ret = 0;
      WiFiClient* client = (WiFiClient*)_client;
      uint16_t _buf_len = strlen((char*)buffer);

      while (len > 0) {

        #ifdef EW_SERIAL_LOG
        Log(F("Channel: Sending to "));Log(client->remoteIP());Log(F(" : "));Logln(client->remotePort());
        #endif
        len =  _buf_len < len ? _buf_len : len;

        if ( this->connected(client) ) {
          // send 250 bytes at most at a time, can adjust this later based on Client

          uint16_t sendlen = len > 250 ? 250 : len;
          //Serial.print("Sending: "); Serial.println(sendlen);
          ret = client->write( buffer, sendlen);
          #ifdef EW_SERIAL_LOG
          Log(F("Channel: sending packets : "));
          for (int i = 0; i < sendlen; i++) {
            Log((char)buffer[i]);
          }
          Logln();
          Log(F("Channel: send packet return ")); Logln(ret);
          #endif
          len -= ret;

          if (len == 0) {
    	      return true;
          }
          if (ret != sendlen) {
            #ifdef EW_SERIAL_LOG
            Logln(F("Channel: send packet - failed to send"));
            #endif
    	      return false;
          }
        } else {
          #ifdef EW_SERIAL_LOG
          Logln(F("Channel: send packet - connection failed"));
          #endif
          return false;
        }
      }
      return true;
    }

  protected:

    channel_t client_channel, server_channel;
    WiFiServer* wifi_server;
    WiFiClient* wifi_client;
    ESP8266WiFiClass* wifi;
};




#endif
