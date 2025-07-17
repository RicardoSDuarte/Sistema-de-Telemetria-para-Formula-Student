
//APP BASE

#ifndef ESPNOW_H
#define ESPNOW_H
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <can_common.h>
//#include "CAN.h"


// Definição da estrutura FILTRO
typedef struct {
  uint16_t id;
  uint8_t code;
} FILTRO;

// Estrutura para cabeçalho do pacote Wi-Fi
typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6];
  uint8_t addr2[6];
  uint8_t addr3[6];
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

// Declarações de variáveis globais
extern volatile bool newDataAvailable;
extern int rssi_display;
extern CAN_FRAME latestFrame;
extern uint8_t latestMac[6];
extern uint8_t broadcastAddress[6];
extern QueueHandle_t QueueHandle;

void sendFiltroGlobal(const FILTRO &filtro);

class ESPNOW {
  public:
    ESPNOW(const uint8_t *peerAddress);
    void ESPNOW_Init();
    bool sendFiltro(const FILTRO &filtro);
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    void setReceiveCallback(esp_now_recv_cb_t cb);
    
  private:
    esp_now_peer_info_t peerInfo;
    uint8_t peerAddress[6];
};

// Declarações das funções ESP-NOW adicionais
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);
void initEspNow();
void initWiFiandEspNow();
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type);
void enablePromiscuousMode();

#endif