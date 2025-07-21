#ifndef ESPNOW_H
#define ESPNOW_H

#include <esp_now.h>
#include <WiFi.h>
#include "CAN.h"
#include <esp_wifi.h>
#include "CANFilter.h"

// CORRETO – Apenas uma declaração!
extern QueueHandle_t QueueFiltroHandle;


class ESPNOW {
  public:
    ESPNOW(const uint8_t *peerAddress);
    void ESPNOW_Init();
    bool sendFrame(const CAN_FRAME &frame);
    bool sendFrame_v2(const CAN_FRAME *frame, int maxRetries , int size);
    static void onDataSent(const uint8_t*mac_addr, esp_now_send_status_t status);
    void setReceiveCallback(esp_now_recv_cb_t cb);
    void static onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);

  private:
    esp_now_peer_info_t peerInfo;
    uint8_t peerAddress[6];
};

#endif
