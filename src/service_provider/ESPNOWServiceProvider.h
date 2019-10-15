#ifndef _ESPNOW_SERVICE_PROVIDER_H_
#define _ESPNOW_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <config/Config.h>
#include <utility/Utility.h>
#include <database/EwingsDefaultDB.h>
extern "C" {
#include <espnow.h>
}

// #define ESP_NOW_KEY     "jo bole so nihal"
#define ESP_NOW_KEY     "lets kill people"
#define ESP_NOW_KEY_LENGTH 16
#define ESP_NOW_CHANNEL 4
#define ESP_NOW_MAX_PEER 8
#define ESP_NOW_DEVICE_TABLE_MAX_SIZE 20
#define ESP_NOW_MAX_BUFF_SIZE 250

enum esp_now_state {
  ESP_NOW_STATE_EMPTY,
  ESP_NOW_STATE_INIT,
  ESP_NOW_STATE_SENT,
	ESP_NOW_STATE_SENT_SUCCEED,
  ESP_NOW_STATE_SENT_FAILED,
  ESP_NOW_STATE_DATA_AVAILABLE,
  ESP_NOW_STATE_RECV_AVAILABLE
};

typedef struct {
  uint8 mac[6];
  uint8_t mesh_level;
} esp_now_device_t;

typedef struct {
  uint8 mac[6];
  esp_now_role role;
  uint8_t channel;
  esp_now_state state;
  uint8_t* buffer;
  uint8_t data_length;
  uint32_t last_receive;
} esp_now_peer_t;

typedef struct {
  uint8_t mesh_level;
  struct global_config global_config;
  uint8_t ssid[WIFI_CONFIGS_BUF_SIZE];
  uint8_t pswd[WIFI_CONFIGS_BUF_SIZE];
  esp_now_device_t device_table[ESP_NOW_DEVICE_TABLE_MAX_SIZE];
  uint8_t device_count;
} esp_now_payload_t;


// esp_now_peer_t esp_now_peers[ESP_NOW_MAX_PEER];

void add_device_to_table(esp_now_device_t *_device);
int is_device_exist_in_table(esp_now_device_t *_device);
void remove_device_from_table(esp_now_device_t *_device);

void esp_now_encrypt_payload(uint8_t *payload, uint8_t len);
void esp_now_decrypt_payload(uint8_t *payload, uint8_t len);
void esp_now_recv_cb(uint8_t *macaddr, uint8_t *data, uint8_t len);
void esp_now_send_cb(uint8_t *macaddr, uint8_t status);

/**
 * ESPNOWServiceProvider class
 */
class ESPNOWServiceProvider{

  public:

    /**
     * ESPNOWServiceProvider constructor.
     */
    ESPNOWServiceProvider(){

      this->flushPeersToDefaults();
    }

    /**
     * ESPNOWServiceProvider destructor.
     */
    ~ESPNOWServiceProvider(){

      this->closeAll();
      this->ew_db = NULL;
      this->wifi = NULL;
    }

    void beginEspNow( ESP8266WiFiClass* _wifi, EwingsDefaultDB* db_handler );
    void handlePeers(void);
    void scanPeers(void);
    void printPeers(void);

    void registerCallbacks(void);
    void unregisterCallbacks(void);

    bool isPeerExist(uint8_t* mac_addr);
    bool isApPeer(uint8_t* mac_addr);
    bool setPeerRole(uint8_t* mac_addr, uint8_t role=ESP_NOW_ROLE_COMBO);
    bool addPeer(uint8_t* mac_addr, uint8_t role=ESP_NOW_ROLE_COMBO, uint8_t channel=ESP_NOW_CHANNEL, uint8_t* key=(uint8_t*)ESP_NOW_KEY, uint8_t key_len=ESP_NOW_KEY_LENGTH);
    bool addInPeers(uint8_t* mac_addr, uint8_t role=ESP_NOW_ROLE_COMBO, uint8_t channel=ESP_NOW_CHANNEL);
    void freePeers(void);

    bool sendToPeer(uint8_t* mac_addr, uint8_t* packet, uint8_t len);
    void receiveFromPeers(void);
    bool broadcastToPeers(uint8_t* packet, uint8_t len);
    bool broadcastToAll(uint8_t* packet, uint8_t len);
    void broadcastConfigData(void);

    void closeAll(void);
    bool deletePeer(uint8_t _peer_index);
    void flushPeersToDefaults(void);
    void setPeerToDefaults(uint8_t _peer_index);

  protected:

    /**
		 * @var	EwingsDefaultDB*  ew_db
		 */
    EwingsDefaultDB* ew_db;
    /**
		 * @var	ESP8266WiFiClass*  wifi
		 */
    ESP8266WiFiClass* wifi;

};




#endif
