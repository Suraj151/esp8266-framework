/******************************** MQTT File ***********************************
This file is part of the Ewings Esp8266 Stack. It is written with the reference
of https://github.com/tuanpmt/esp_mqtt


This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#include <config/Config.h>

#if defined(ENABLE_MQTT_SERVICE)

#include "Mqtt.h"

#define MQTT_SEND_TIMEOUT           5
#define MQTT_READ_TIMEOUT           10

#ifndef QUEUE_BUFFER_SIZE
#define QUEUE_BUFFER_SIZE           2048
#endif

void mqttConnectedCb( uint32_t *args ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: connected cb"));
    #endif
    MQTTClient* _client = ( MQTTClient* )args;

    _client->mqttClient.mqtt_connected = true;
}

void mqttDisconnectedCb( uint32_t *args ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: disconnect cb"));
    #endif
    MQTTClient* _client = ( MQTTClient* )args;

    _client->mqttClient.mqtt_connected = false;
    _client->clear_all_subscribed_topics();
}

void mqttPublishedCb( uint32_t *args ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: published cb"));
    #endif
}

void mqttSubscribedCb( uint32_t *args ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: subscribed cb"));
    #endif
}

void mqttUnsubscribedCb( uint32_t *args ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: unsubscribed cb"));
    #endif
}

void mqttDataCb( uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len){

    char *topicBuf = new char[topic_len+1], *dataBuf = new char[data_len+1];

    memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: data cb"));
    Serial.printf("MQTT: Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    #endif

    delete[] topicBuf; delete[] dataBuf;
}


bool MQTTClient::is_mqtt_connected(){

    if(
      this->mqttClient.connState == WIFI_CONNECTING_ERROR ||
      this->mqttClient.connState == MQTT_DELETING ||
      this->mqttClient.connState == MQTT_DISCONNECTED
    ) return false;
    return ( this->connected() && this->mqttClient.mqtt_connected );
}

bool MQTTClient::is_topic_subscribed( char* _topic ){

  for ( uint16_t i = 0; i < this->mqttClient.subscribed_topics.size(); i++) {

    if( __are_str_equals( this->mqttClient.subscribed_topics[i].topic, _topic, strlen( this->mqttClient.subscribed_topics[i].topic ) ) )
      return true;
  }return false;
}

void MQTTClient::clear_all_subscribed_topics(){

  for ( uint16_t i = 0; i < this->mqttClient.subscribed_topics.size(); i++) {

    delete[] this->mqttClient.subscribed_topics[i].topic;
  } this->mqttClient.subscribed_topics.clear();
}

void MQTTClient::add_to_subscribed_topics( char* _topic, uint8_t _qos ){

  int _len = strlen(_topic);
  mqtt_subscribed_topics_t _subscribe_topic;
  _subscribe_topic.topic = new char[_len + 1];
  strcpy( _subscribe_topic.topic, _topic);
  _subscribe_topic.topic[_len] = 0;
  _subscribe_topic.qos = _qos;

  this->mqttClient.subscribed_topics.push_back( _subscribe_topic );
}

bool MQTTClient::remove_from_subscribed_topics( char* _topic ){

  for ( uint16_t i = 0; i < this->mqttClient.subscribed_topics.size(); i++) {

    if( __are_str_equals( this->mqttClient.subscribed_topics[i].topic, _topic, strlen( this->mqttClient.subscribed_topics[i].topic ) ) ){

      this->mqttClient.subscribed_topics.erase( this->mqttClient.subscribed_topics.begin() + i );
      return true;
    }
  }return false;
}


void MQTTClient::deliver_publish(  uint8_t* message, int length ){

    mqtt_event_data_t event_data;

    event_data.topic_length = length;
    event_data.topic = (char*)mqtt_get_publish_topic(message, &event_data.topic_length);
    event_data.data_length = length;
    event_data.data = (char*)mqtt_get_publish_data(message, &event_data.data_length);

    if (this->dataCb)
        this->dataCb( this->mqttDataCallbackArgs, event_data.topic, event_data.topic_length, event_data.data, event_data.data_length);
    else
        mqttDataCb( this->mqttDataCallbackArgs, event_data.topic, event_data.topic_length, event_data.data, event_data.data_length);
}

// void mqtt_wificlient_delete( MQTT_Client *mqttClient ){
//
//     if ( mqttClient->wifi_client != NULL) {
//         #ifdef EW_SERIAL_LOG
//         Logln(F("MQTT: deleting wifi Client"));
//         #endif
//         mqttClient->wifi_client->stopAll();
//         delete mqttClient->wifi_client;
//         mqttClient->wifi_client = NULL;
//     }
// }

void MQTTClient::mqtt_client_recv(){

    int len = this->readFullPacket( this->mqttClient.mqtt_state.in_buffer, this->mqttClient.mqtt_state.in_buffer_length, MQTT_READ_TIMEOUT*10 );
    #ifdef EW_SERIAL_LOG
    Log(F("MQTT: recieved packet size :"));Logln(len);
    Log(F("MQTT: recieved packets :"));
    for (int i = 0; i < len; i++) {
      Log_format( this->mqttClient.mqtt_state.in_buffer[i], HEX ); Log(" ");
      delay(0);
    }
    Logln();
    #endif
    delay(0);

    if ( len < MQTT_BUF_SIZE && len > 0 ) {

        uint8_t msg_type = mqtt_get_type(this->mqttClient.mqtt_state.in_buffer);
        uint8_t msg_qos = mqtt_get_qos(this->mqttClient.mqtt_state.in_buffer);
        uint16_t msg_id = mqtt_get_id(this->mqttClient.mqtt_state.in_buffer, this->mqttClient.mqtt_state.in_buffer_length);

        if( !( msg_type == MQTT_MSG_TYPE_PUBLISH && msg_qos == 0 ) )
        this->mqttClient.keepAliveTick = 0;
        this->mqttClient.readTimeout = 0;
        this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;

        #ifdef EW_SERIAL_LOG
        Log(F("MQTT: recieved msg type :"));Logln(msg_type);
        Log(F("MQTT: recieved msg qos :"));Logln(msg_qos);
        #endif

        switch ( this->mqttClient.connState ) {

            case MQTT_CONNECT_SENT:

                if ( msg_type == MQTT_MSG_TYPE_CONNACK ) {

                    if ( this->mqttClient.mqtt_state.pending_msg_type != MQTT_MSG_TYPE_CONNECT ) {

                        #ifdef EW_SERIAL_LOG
                        Logln(F("MQTT: Invalid packet recieved"));
                        #endif
                        this->mqttClient.connState = MQTT_HOST_RECONNECT_REQ;
                    } else {

                        #ifdef EW_SERIAL_LOG
                        Logln(F("MQTT: CONNACK recieved"));
                        #endif
                        this->mqttClient.connState = MQTT_DATA;
                        if (this->connectedCb)
                            this->connectedCb( (uint32_t*)this );
                    }

                }
                break;
            case MQTT_DISCONNECT_SENT:
                //ignore
                if( this->disconnectedCb ) this->disconnectedCb( (uint32_t*)this );
                this->mqttClient.connState = MQTT_DISCONNECTED;
                break;
            case MQTT_DATA_SENT:
            case MQTT_PING_SENT:
            case MQTT_DATA:
            case MQTT_KEEPALIVE_REQ:

                this->mqttClient.mqtt_state.message_length_read = len;
                this->mqttClient.mqtt_state.message_length = mqtt_get_total_length( this->mqttClient.mqtt_state.in_buffer, this->mqttClient.mqtt_state.message_length_read );
                this->mqttClient.connState = MQTT_DATA;

                switch (msg_type){

                    case MQTT_MSG_TYPE_SUBACK:

                        if ( this->mqttClient.mqtt_state.pending_msg_type == MQTT_MSG_TYPE_SUBSCRIBE && this->mqttClient.mqtt_state.pending_msg_id == msg_id )
                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: Subscribe successful"));
                            #endif
                            if (this->subscribedCb)
                                this->subscribedCb( (uint32_t*)this );
                        break;

                    case MQTT_MSG_TYPE_UNSUBACK:

                        if ( this->mqttClient.mqtt_state.pending_msg_type == MQTT_MSG_TYPE_UNSUBSCRIBE && this->mqttClient.mqtt_state.pending_msg_id == msg_id )
                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: UnSubscribe successful"));
                            #endif
                            if (this->unsubscribedCb)
                                this->unsubscribedCb( (uint32_t*)this );
                        break;

                    case MQTT_MSG_TYPE_PUBLISH:

                        if ( msg_qos == 1 )

                            this->mqttClient.mqtt_state.outbound_message = mqtt_msg_puback( &this->mqttClient.mqtt_state.mqtt_connection, msg_id );
                        else if (msg_qos == 2)

                            this->mqttClient.mqtt_state.outbound_message = mqtt_msg_pubrec( &this->mqttClient.mqtt_state.mqtt_connection, msg_id );
                        if (msg_qos == 1 || msg_qos == 2) {

                            #ifdef EW_SERIAL_LOG
                            Serial.printf("MQTT: Queue response QoS: %d\r\n", msg_qos);
                            #endif
                            if ( QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
                              #ifdef EW_SERIAL_LOG
                              Logln(F("MQTT: Queue full"));
                              #endif
                            }
                        }

                        this->deliver_publish( this->mqttClient.mqtt_state.in_buffer, this->mqttClient.mqtt_state.message_length_read );
                        break;

                    case MQTT_MSG_TYPE_PUBACK:

                        if (this->mqttClient.mqtt_state.pending_msg_type == MQTT_MSG_TYPE_PUBLISH && this->mqttClient.mqtt_state.pending_msg_id == msg_id) {

                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: received MQTT_MSG_TYPE_PUBACK, finish QoS1 publish"));
                            #endif
                            if (this->publishedCb)
                                this->publishedCb( (uint32_t*)this );
                        }

                        break;

                    case MQTT_MSG_TYPE_PUBREC:

                        this->mqttClient.mqtt_state.outbound_message = mqtt_msg_pubrel(&this->mqttClient.mqtt_state.mqtt_connection, msg_id);

                        if (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: Queue full"));
                            #endif
                        }
                        break;

                    case MQTT_MSG_TYPE_PUBREL:

                        this->mqttClient.mqtt_state.outbound_message = mqtt_msg_pubcomp(&this->mqttClient.mqtt_state.mqtt_connection, msg_id);

                        if (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: Queue full"));
                            #endif
                        }
                        break;

                    case MQTT_MSG_TYPE_PUBCOMP:

                        if (this->mqttClient.mqtt_state.pending_msg_type == MQTT_MSG_TYPE_PUBLISH && this->mqttClient.mqtt_state.pending_msg_id == msg_id) {

                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: received MQTT_MSG_TYPE_PUBCOMP, finish QoS2 publish"));
                            #endif
                            if (this->publishedCb)
                                this->publishedCb( (uint32_t*)this );
                        }
                        break;

                    case MQTT_MSG_TYPE_PINGREQ:

                        this->mqttClient.mqtt_state.outbound_message = mqtt_msg_pingresp(&this->mqttClient.mqtt_state.mqtt_connection);

                        if (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {

                            #ifdef EW_SERIAL_LOG
                            Logln(F("MQTT: Queue full"));
                            #endif
                        }
                        break;

                    case MQTT_MSG_TYPE_PINGRESP:
                        #ifdef EW_SERIAL_LOG
                        Logln(F("MQTT: MQTT_MSG_TYPE_PINGRESP recieved"));
                        #endif
                        break;
                    default :
                        #ifdef EW_SERIAL_LOG
                        Logln(F("MQTT: Uknown msg type received."));
                        #endif
                        break;
                }
                break;
            default :
                #ifdef EW_SERIAL_LOG
                Logln(F("MQTT: Uknown msg type received."));
                #endif
                break;
        }
    } else if( len == 0 ){
      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: No response received yet"));
      #endif
    }else {
      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: response too long"));
      #endif
    }
    // if( this->mqttClient.connState == MQTT_DATA ) this->MQTT_Task();
}

void MQTTClient::mqtt_client_connect(){

    mqtt_msg_init(&this->mqttClient.mqtt_state.mqtt_connection, this->mqttClient.mqtt_state.out_buffer, this->mqttClient.mqtt_state.out_buffer_length);
    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_connect(&this->mqttClient.mqtt_state.mqtt_connection, this->mqttClient.mqtt_state.connect_info);
    this->mqttClient.mqtt_state.pending_msg_type = mqtt_get_type(this->mqttClient.mqtt_state.outbound_message->data);
    this->mqttClient.mqtt_state.pending_msg_id = mqtt_get_id(this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length);

    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: Sending, type: %d, id: %04X\r\n", this->mqttClient.mqtt_state.pending_msg_type, this->mqttClient.mqtt_state.pending_msg_id);
    #endif
    bool result = sendPacket( this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length );

    if( result ){

      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: connect packet sent"));
      #endif
      this->mqttClient.connState = MQTT_CONNECT_SENT;
      this->mqttClient.readTimeout = 0;
    }else{

      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: connect packet send failed"));
      #endif
      this->mqttClient.connState = MQTT_CONNECT_FAILED;
      this->mqttClient.host_connect_tick = 0;
      this->disconnectServer();
    }
    this->mqttClient.mqtt_state.outbound_message = NULL;
    // this->MQTT_Task();
}

void MQTTClient::mqtt_client_disconnect(){

    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_disconnect(&this->mqttClient.mqtt_state.mqtt_connection);
    this->mqttClient.mqtt_state.pending_msg_type = mqtt_get_type(this->mqttClient.mqtt_state.outbound_message->data);
    this->mqttClient.mqtt_state.pending_msg_id = mqtt_get_id(this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length);

    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: Sending, type: %d, id: %04X\r\n", this->mqttClient.mqtt_state.pending_msg_type, this->mqttClient.mqtt_state.pending_msg_id);
    #endif
    bool result = sendPacket( this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length );

    if( result ){

      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: disconnect packet sent"));
      #endif
      this->mqttClient.connState = MQTT_DISCONNECT_SENT;
      this->mqttClient.readTimeout = 0;
    }else{

      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: disconnect packet send failed"));
      #endif
      this->mqttClient.connState = MQTT_DISCONNECT_FAILED;
      this->mqttClient.host_connect_tick = 0;
    }
    this->disconnectServer();
    this->mqttClient.mqtt_state.outbound_message = NULL;
}

void MQTTClient::mqtt_send_keepalive(){

    #ifdef EW_SERIAL_LOG
    Serial.printf("\r\nMQTT: Send keepalive packet to %s:%d!\r\n", this->host, this->port);
    #endif
    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_pingreq(&this->mqttClient.mqtt_state.mqtt_connection);
    // this->mqttClient.mqtt_state.pending_msg_type = MQTT_MSG_TYPE_PINGREQ;
    this->mqttClient.mqtt_state.pending_msg_type = mqtt_get_type(this->mqttClient.mqtt_state.outbound_message->data);
    this->mqttClient.mqtt_state.pending_msg_id = mqtt_get_id(this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length);


    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: Sending, type: %d, id: %04X\r\n", this->mqttClient.mqtt_state.pending_msg_type, this->mqttClient.mqtt_state.pending_msg_id);
    #endif
    bool result = sendPacket( this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length );

    if(result) {
        this->mqttClient.keepAliveTick = 0;
        this->mqttClient.readTimeout = 0;
        this->mqttClient.connState = MQTT_PING_SENT;
    }else {
        this->mqttClient.host_connect_tick = 0;
        this->mqttClient.connState = MQTT_PING_FAILED;
        this->disconnectServer();
    }
    // this->MQTT_Task();
    this->mqttClient.mqtt_state.outbound_message = NULL;
}

void MQTTClient::MQTT_Task(){

    #ifdef EW_SERIAL_LOG
    Log(F("MQTT: Task ")); Logln(this->mqttClient.connState);
    #endif

    uint8_t dataBuffer[MQTT_BUF_SIZE];  uint16_t dataLen;
    memset( dataBuffer, 0, MQTT_BUF_SIZE );

    switch (this->mqttClient.connState) {

      default:
          break;
      case MQTT_HOST_RECONNECT_REQ:
      case MQTT_HOST_RECONNECT:
          this->disconnectServer();
          this->Connect();
          break;
      case MQTT_CONNECT_SENT:
      case MQTT_DISCONNECT_SENT:
      case MQTT_DATA_SENT:
      case MQTT_PING_SENT:
          this->mqtt_client_recv();
          break;
      case WIFI_CONNECTING_ERROR:
          if( this->wifi->isConnected() ){
            this->mqttClient.connState = MQTT_HOST_RECONNECT_REQ;
          }
          break;
      case MQTT_HOST_CONNECTING:
          if( !this->wifi->isConnected() ){
            #ifdef EW_SERIAL_LOG
            Logln( F("MQTT: WiFi not connected. waiting..") );
            #endif
            this->mqttClient.host_connect_tick = 0;
            this->mqttClient.connState = WIFI_CONNECTING_ERROR;
          }else{

            if( this->connected() ){
              #ifdef EW_SERIAL_LOG
              Serial.printf("MQTT: Connected to broker %s:%d\r\n", this->host, this->port);
              #endif
              this->mqtt_client_connect();
            }
          }
          break;
      case MQTT_DISCONNECT_REQ:
          this->mqtt_client_disconnect();
          break;
      case MQTT_KEEPALIVE_REQ:
          this->mqtt_send_keepalive();
          break;
      case MQTT_DATA:
          // if ( QUEUE_IsEmpty(&this->mqttClient.msgQueue) || this->mqttClient.sendTimeout != 0 ) {
          if ( QUEUE_IsEmpty(&this->mqttClient.msgQueue) ) {
              this->mqtt_client_recv();
              break;
          }
          if ( QUEUE_Gets(&this->mqttClient.msgQueue, dataBuffer, &dataLen, MQTT_BUF_SIZE) == 0 ) {

              #ifdef EW_SERIAL_LOG
              Log(F("MQTT: getting qued packets :"));
              for (int i = 0; i < dataLen; i++) {
                // Log_format( dataBuffer[i], HEX ); Log(" ");
                Log( (char)dataBuffer[i] ); Log(" ");
                delay(0);
              }
              Logln();
              #endif

              uint8_t msg_qos = mqtt_get_qos(dataBuffer);
              uint8_t msg_type = mqtt_get_type(dataBuffer);

              this->mqttClient.mqtt_state.pending_msg_type = msg_type;
              this->mqttClient.mqtt_state.pending_msg_id = mqtt_get_id(dataBuffer, dataLen);

              this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
              bool result = this->sendPacket( dataBuffer, dataLen );

              if( result ){

                #ifdef EW_SERIAL_LOG
                Log(F("MQTT: data packet sent of id: "));Logln(this->mqttClient.mqtt_state.pending_msg_id);
                #endif
                this->mqttClient.connState = (msg_type==MQTT_MSG_TYPE_PUBLISH&&msg_qos==0)?MQTT_DATA:MQTT_DATA_SENT;
                this->mqttClient.readTimeout = 0;
              }else{

                #ifdef EW_SERIAL_LOG
                Logln(F("MQTT: data packet send failed"));
                #endif
                this->mqttClient.connState = MQTT_DATA_FAILED;
                this->mqttClient.host_connect_tick = 0;
                this->disconnectServer();
              }

              this->mqttClient.mqtt_state.outbound_message = NULL;
          }
          break;
    }
    delay(0); // yield purpose
}

void MQTTClient::mqtt_client_delete(){

    // mqtt_wificlient_delete(mqttClient);
    if (this->host != NULL) {
      delete[] this->host;
      this->host = NULL;
    }

    // if (this->mqttClient.user_data != NULL) {
    //   delete this->mqttClient.user_data;
    //   this->mqttClient.user_data = NULL;
    // }

    if(this->mqttClient.connect_info.client_id != NULL) {
      delete[] this->mqttClient.connect_info.client_id;
      this->mqttClient.connect_info.client_id = NULL;
    }

    if(this->mqttClient.connect_info.username != NULL) {
      delete[] this->mqttClient.connect_info.username;
      this->mqttClient.connect_info.username = NULL;
    }

    if(this->mqttClient.connect_info.password != NULL) {
      delete[] this->mqttClient.connect_info.password;
      this->mqttClient.connect_info.password = NULL;
    }

    if(this->mqttClient.connect_info.will_topic != NULL) {
      delete[] this->mqttClient.connect_info.will_topic;
      this->mqttClient.connect_info.will_topic = NULL;
    }

    if(this->mqttClient.connect_info.will_message != NULL) {
      delete[] this->mqttClient.connect_info.will_message;
      this->mqttClient.connect_info.will_message = NULL;
    }

    if(this->mqttClient.mqtt_state.in_buffer != NULL) {
      delete[] this->mqttClient.mqtt_state.in_buffer;
      this->mqttClient.mqtt_state.in_buffer = NULL;
    }

    if(this->mqttClient.mqtt_state.out_buffer != NULL) {
      delete[] this->mqttClient.mqtt_state.out_buffer;
      this->mqttClient.mqtt_state.out_buffer = NULL;
    }

    if(this->mqttClient.msgQueue.buf != NULL) {
      delete[] this->mqttClient.msgQueue.buf;
      this->mqttClient.msgQueue.buf = NULL;
    }

    if(this->wifi_client != NULL) {
      delete this->wifi_client;
      this->wifi_client = NULL;
    }

    this->clear_all_subscribed_topics();
}

bool MQTTClient::Subscribe( char* topic, uint8_t qos){

    uint8_t dataBuffer[MQTT_BUF_SIZE];  uint16_t dataLen;
    memset( dataBuffer, 0, MQTT_BUF_SIZE );

    if( !this->connected() ) return false;

    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_subscribe( &this->mqttClient.mqtt_state.mqtt_connection,
                                          topic, qos,
                                          &this->mqttClient.mqtt_state.pending_msg_id);

    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: queue subscribe, topic \"%s\", id: %d\r\n", topic, this->mqttClient.mqtt_state.pending_msg_id );
    #endif
    // bool result = sendPacket( this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length );

    while (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: Queue full"));
      #endif
        if (QUEUE_Gets(&this->mqttClient.msgQueue, dataBuffer, &dataLen, MQTT_BUF_SIZE) == -1) {
          #ifdef EW_SERIAL_LOG
          Logln(F("MQTT: Serious buffer error"));
          #endif
          return false;
        }
    }

    if( !this->is_topic_subscribed( topic ) )
      this->add_to_subscribed_topics( topic, qos );
    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    return true;
}

bool MQTTClient::UnSubscribe( char* topic ) {

    uint8_t dataBuffer[MQTT_BUF_SIZE];  uint16_t dataLen;
    memset( dataBuffer, 0, MQTT_BUF_SIZE );

    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_unsubscribe(&this->mqttClient.mqtt_state.mqtt_connection,
                                          topic,
                                          &this->mqttClient.mqtt_state.pending_msg_id);

    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: queue un-subscribe, topic \"%s\", id: %d\r\n", topic, this->mqttClient.mqtt_state.pending_msg_id);
    #endif

    while (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
        #ifdef EW_SERIAL_LOG
        Logln(F("MQTT: Queue full"));
        #endif
        if (QUEUE_Gets(&this->mqttClient.msgQueue, dataBuffer, &dataLen, MQTT_BUF_SIZE) == -1) {
            #ifdef EW_SERIAL_LOG
            Logln(F("MQTT: Serious buffer error"));
            #endif
            return false;
        }
    }

    this->remove_from_subscribed_topics( topic );
    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    return true;
}

bool MQTTClient::Publish( const char* topic, const char* data, int data_length, int qos, int retain ){

    uint8_t dataBuffer[MQTT_BUF_SIZE];  uint16_t dataLen;
    memset( dataBuffer, 0, MQTT_BUF_SIZE );

    if( !this->connected() ) return false;

    this->mqttClient.mqtt_state.outbound_message = mqtt_msg_publish(&this->mqttClient.mqtt_state.mqtt_connection,
                                          topic, data, data_length,
                                          qos, retain,
                                          &this->mqttClient.mqtt_state.pending_msg_id);
    if (this->mqttClient.mqtt_state.outbound_message->length == 0) {
        #ifdef EW_SERIAL_LOG
        Logln(F("MQTT: Queuing publish failed"));
        #endif
        return false;
    }
    #ifdef EW_SERIAL_LOG
    Serial.printf("MQTT: queuing publish, length: %d, queue size(%d/%d)\r\n", this->mqttClient.mqtt_state.outbound_message->length, this->mqttClient.msgQueue.rb.fill_cnt, this->mqttClient.msgQueue.rb.size);
    #endif
    while (QUEUE_Puts(&this->mqttClient.msgQueue, this->mqttClient.mqtt_state.outbound_message->data, this->mqttClient.mqtt_state.outbound_message->length) == -1) {
        #ifdef EW_SERIAL_LOG
        Logln(F("MQTT: Queue full"));
        #endif
        if (QUEUE_Gets(&this->mqttClient.msgQueue, dataBuffer, &dataLen, MQTT_BUF_SIZE) == -1) {
            #ifdef EW_SERIAL_LOG
            Logln(F("MQTT: Serious buffer error"));
            #endif
            return false;
        }
    }
    this->mqttClient.sendTimeout = MQTT_SEND_TIMEOUT;
    return true;
}


void MQTTClient::InitConnection( char* host, int port, uint8_t security ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: InitConnection"));
    #endif

    int _host_len = strlen(host);
    memset( &this->mqttClient, 0, sizeof(MQTT_Client) );
    this->host = new char[_host_len + 1];
    strcpy( this->host, host );
    this->host[_host_len] = 0;
    this->port = port;
    this->security = security;
}

void MQTTClient::InitClient( char* client_id, char* client_user, char* client_pass, uint32_t keepAliveTime, uint8_t cleanSession ){

    uint32_t temp;

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: InitClient"));
    #endif

    memset(&this->mqttClient.connect_info, 0, sizeof(mqtt_connect_info_t));

    int _len = strlen(client_id);
    this->mqttClient.connect_info.client_id = new char[_len + 1];
    strcpy( this->mqttClient.connect_info.client_id, client_id);
    this->mqttClient.connect_info.client_id[_len] = 0;

    if (client_user){

        _len = strlen(client_user);
        this->mqttClient.connect_info.username = new char[_len + 1];
        strcpy( this->mqttClient.connect_info.username, client_user);
        this->mqttClient.connect_info.username[_len] = 0;
    }

    if (client_pass){

        _len = strlen(client_pass);
        this->mqttClient.connect_info.password = new char[_len + 1];
        strcpy( this->mqttClient.connect_info.password, client_pass);
        this->mqttClient.connect_info.password[_len] = 0;
    }


    this->mqttClient.connect_info.keepalive = keepAliveTime > 5 ? keepAliveTime : MQTT_DEFAULT_KEEPALIVE;
    this->mqttClient.connect_info.clean_session = cleanSession;

    this->mqttClient.mqtt_state.in_buffer = new uint8_t[MQTT_BUF_SIZE];
    this->mqttClient.mqtt_state.in_buffer_length = MQTT_BUF_SIZE;
    this->mqttClient.mqtt_state.out_buffer =  new uint8_t[MQTT_BUF_SIZE];
    this->mqttClient.mqtt_state.out_buffer_length = MQTT_BUF_SIZE;
    this->mqttClient.mqtt_state.connect_info = &this->mqttClient.connect_info;

    mqtt_msg_init( &this->mqttClient.mqtt_state.mqtt_connection, this->mqttClient.mqtt_state.out_buffer, this->mqttClient.mqtt_state.out_buffer_length );

    QUEUE_Init(&this->mqttClient.msgQueue, QUEUE_BUFFER_SIZE);

    // this->MQTT_Task();
}

void MQTTClient::InitLWT( char* will_topic, char* will_msg, uint8_t will_qos, uint8_t will_retain ){

    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: InitLWT"));
    #endif
    int _len = strlen(will_topic);
    this->mqttClient.connect_info.will_topic = new char[_len + 1];
    strcpy(this->mqttClient.connect_info.will_topic, will_topic);
    this->mqttClient.connect_info.will_topic[_len] = 0;

    _len = strlen(will_msg);
    this->mqttClient.connect_info.will_message = new char[_len + 1];
    strcpy(this->mqttClient.connect_info.will_message, will_msg);
    this->mqttClient.connect_info.will_message[_len] = 0;


    this->mqttClient.connect_info.will_qos = will_qos;
    this->mqttClient.connect_info.will_retain = will_retain;
}


void MQTTClient::mqtt_timer(){

    #ifdef EW_SERIAL_LOG
    Log( F("MQTT: mqtt timer : ") ); Log( this->connected() );
    Log( F("\t mqtt state : ") ); Logln( this->mqttClient.connState );

    Log( F("MQTT: subscribed topics(QoS) : ") );
    for ( uint16_t i = 0; i < this->mqttClient.subscribed_topics.size(); i++) {

      Log( this->mqttClient.subscribed_topics[i].topic );
      Log( F("(") );
      Log( this->mqttClient.subscribed_topics[i].qos );
      Log( F("), ") );
    }
    Logln();
    #endif
    // if( this->mqttClient.connState == MQTT_DELETING ) return;

    if (this->mqttClient.connState == MQTT_DATA) {

        this->mqttClient.keepAliveTick ++;
        int _keep_alive = this->mqttClient.mqtt_state.connect_info->keepalive * 0.85;
        if ( this->mqttClient.keepAliveTick > _keep_alive ) {
            this->mqttClient.connState = MQTT_KEEPALIVE_REQ;
        }
        this->MQTT_Task();
    } else if (
      this->mqttClient.connState == MQTT_CONNECT_SENT ||
      this->mqttClient.connState == MQTT_DISCONNECT_SENT ||
      this->mqttClient.connState == MQTT_PING_SENT ||
      this->mqttClient.connState == MQTT_DATA_SENT
    ) {

        this->mqttClient.readTimeout ++;
        if (this->mqttClient.readTimeout > MQTT_READ_TIMEOUT) {
            this->mqttClient.readTimeout = 0;
            this->mqttClient.connState = MQTT_HOST_RECONNECT_REQ;
            if (this->timeoutCb)
                this->timeoutCb( (uint32_t*)this );
        }
        this->MQTT_Task();
    } else if (
      this->mqttClient.connState == MQTT_HOST_RECONNECT_REQ ||
      this->mqttClient.connState == WIFI_CONNECTING_ERROR
    ) {

        this->MQTT_Task();
    } else if (
      this->mqttClient.connState == MQTT_HOST_CONNECTING ||
      this->mqttClient.connState == MQTT_CONNECT_FAILED ||
      this->mqttClient.connState == MQTT_DISCONNECT_FAILED ||
      this->mqttClient.connState == MQTT_PING_FAILED ||
      this->mqttClient.connState == MQTT_DATA_FAILED
    ) {

      this->mqttClient.host_connect_tick ++;
      if (this->mqttClient.host_connect_tick > MQTT_HOST_CONNECT_TIMEOUT) {
          this->mqttClient.host_connect_tick = 0;
          #ifdef EW_SERIAL_LOG
          Logln( F("MQTT: host connect error. trying reconnect") );
          #endif
          this->mqttClient.connState = MQTT_HOST_RECONNECT;
          this->MQTT_Task();
      }
    }else{

      //probably this->mqttClient.connState == MQTT_DELETING
    }

    if (this->mqttClient.sendTimeout > 0)  this->mqttClient.sendTimeout --;
    delay(0); // yield purpose
}

void MQTTClient::Connect(){

    if ( !this->wifi_client ) {

      this->wifi_client = new WiFiClient;
    }
    this->mqttClient.keepAliveTick = 0;
    this->mqttClient.host_connect_tick = 0;

    this->mqttClient.connState = MQTT_HOST_CONNECTING;

    this->connectServer();
    this->MQTT_Task();
}

void MQTTClient::Disconnect(){

    this->mqttClient.connState = MQTT_DISCONNECT_REQ;
    this->MQTT_Task();
}

void MQTTClient::DeleteClient(){

    this->mqttClient.connState = MQTT_DELETING;
    this->disconnectServer();
    this->mqtt_client_delete();
    // this->MQTT_Task();
}

void MQTTClient::OnConnected( MqttCallback connectedCb ){

    this->connectedCb = connectedCb;
}

void MQTTClient::OnDisconnected( MqttCallback disconnectedCb ){

    this->disconnectedCb = disconnectedCb;
}

void MQTTClient::OnData( MqttDataCallback dataCb ){

    this->dataCb = dataCb;
}

void MQTTClient::OnPublished( MqttCallback publishedCb ){

    this->publishedCb = publishedCb;
}

void MQTTClient::OnSubscribed( MqttCallback subscribedCb ){

    this->subscribedCb = subscribedCb;
}

void MQTTClient::OnUnsubscribed( MqttCallback unsubscribedCb ){

    this->unsubscribedCb = unsubscribedCb;
}

void MQTTClient::OnTimeout( MqttCallback timeoutCb ){

    this->timeoutCb = timeoutCb;
}



/*   WiFi client support functions */


bool MQTTClient::connectServer( ) {

  #ifdef EW_SERIAL_LOG
  Serial.printf("MQTT: Connecting to %s:%d\r\n", this->host, this->port);
  #endif
  this->wifi_client->setTimeout(3000);
  int result = this->wifi_client->connect(this->host, this->port);
  #ifdef EW_SERIAL_LOG
  Log(F("MQTT: Connect result -: ")); Logln(result);
  #endif
  return result != 0;
}

bool MQTTClient::disconnectServer() {

  if( this->disconnectedCb ) this->disconnectedCb( (uint32_t*)this );
  return this->connected() ? this->wifi_client->stop(500) : true;
}

bool MQTTClient::connected() {

  // bool _is_live = this->wifi_client->connected();
  // if ( !_is_live ) {
  //   this->wifi_client->stop();
  // }
  // return _is_live;
  return this->wifi_client ? this->wifi_client->connected() : false;
}

uint16_t MQTTClient::readFullPacket( uint8_t *buffer, uint16_t maxsize, uint16_t timeout ) {

  uint8_t *pbuff = buffer;
  uint16_t rlen;

  memset( buffer, 0, maxsize );
  // read the packet type:
  rlen = this->readPacket( pbuff, 1, timeout );
  if (rlen != 1) return 0;

  // #ifdef EW_SERIAL_LOG
  // Log(F("MQTT: Packet Type:\t")); Logln((char*)pbuff);
  // #endif

  pbuff++;

  uint32_t value = 0;
  uint32_t multiplier = 1;
  uint8_t encodedByte;

  do {
    rlen = this->readPacket( pbuff, 1, timeout );
    if (rlen != 1) return 0;
    encodedByte = pbuff[0]; // save the last read val
    pbuff++; // get ready for reading the next byte
    uint32_t intermediate = encodedByte & 0x7F;
    intermediate *= multiplier;
    value += intermediate;
    multiplier *= 128;
    if (multiplier > (128UL*128UL*128UL)) {
      #ifdef EW_SERIAL_LOG
      Log(F("Malformed packet len"));
      #endif
      return 0;
    }
  } while (encodedByte & 0x80);

  // #ifdef EW_SERIAL_LOG
  // Log(F("MQTT: Packet Length:\t"));Logln(value);
  // #endif

  if (value > (maxsize - (pbuff-buffer) - 1)) {
    #ifdef EW_SERIAL_LOG
    Logln(F("MQTT: Packet too big for buffer"));
    #endif
    rlen = this->readPacket( pbuff, (maxsize - (pbuff-buffer) - 1), timeout );
  } else {
    rlen = this->readPacket( pbuff, value, timeout );
  }
  return ((pbuff - buffer)+rlen);
}

uint16_t MQTTClient::readPacket( uint8_t *buffer, uint16_t maxlen, int16_t timeout ) {

  uint16_t len = 0;
  int16_t t = timeout;

  while ( this->connected() && ( timeout >= 0 ) ) {

    while ( this->wifi_client->available() ) {

      char c = this->wifi_client->read();
      timeout = t;  // reset the timeout
      buffer[len] = c;
      len++;

      if( maxlen == 0 ){
        return 0;
      }

      if( len == maxlen ){
        // #ifdef EW_SERIAL_LOG
        // Log(F("MQTT: Read Data ")); Logln((char*)buffer);
        // #endif
        return len;
      }
    }
    timeout -= MQTT_CLIENT_READINTERVAL_MS;
    delay(MQTT_CLIENT_READINTERVAL_MS);
  }
  return len;
}

bool MQTTClient::sendPacket( uint8_t *buffer, uint16_t len ) {
  uint16_t ret = 0;

  while (len > 0) {
    if ( this->connected() ) {
      // send 250 bytes at most at a time, can adjust this later based on Client

      // uint16_t sendlen = len > 250 ? 250 : len;
      uint16_t sendlen = len;
      //Serial.print("Sending: "); Serial.println(sendlen);
      ret = this->wifi_client->write( buffer, sendlen);
      #ifdef EW_SERIAL_LOG
      // Log(F("MQTT: sending packets : "));
      // for (int i = 0; i < len; i++) {
      //   Log((char)buffer[i]);
      //   Log(F(" "));
      // }
      // Logln();
      Log(F("MQTT: send packet return ")); Logln(ret);
      #endif
      len -= ret;

      if (ret != sendlen) {
        #ifdef EW_SERIAL_LOG
        Logln(F("MQTT: send packet - failed to send"));
        #endif
	      return false;
      }
    } else {
      #ifdef EW_SERIAL_LOG
      Logln(F("MQTT: send packet - connection failed"));
      #endif
      return false;
    }
  }
  return true;
}

#endif
