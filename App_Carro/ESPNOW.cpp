#include "esp_wifi_types_generic.h"
//APP CARRO

#include "esp_now.h"
#include "ESPNOW.h"

uint8_t latestMac[6];
QueueHandle_t QueueFiltroHandle = nullptr;



ESPNOW::ESPNOW(const uint8_t *peerAddr) {
  memcpy(peerAddress, peerAddr, 6);
}

void ESPNOW::ESPNOW_Init() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(ESPNOW::onDataSent);
  esp_now_register_recv_cb(ESPNOW::onDataReceive);

  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(peerInfo.peer_addr)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Falha ao adicionar peer");
    } else {
      Serial.println("Peer adicionado com sucesso!");
    }
  }
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_5M_L);
}

bool ESPNOW::sendFrame(const CAN_FRAME &frame) {
  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&frame, sizeof(CAN_FRAME));
  
  if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("Buffer ESP-NOW cheio!");
    // Implementar retry ou controle de fluxo
  }
  
  return (result == ESP_OK);
}
bool ESPNOW::sendFrame_v2(const CAN_FRAME *frame, int maxRetries, int size) {
    for (int i = 0; i < maxRetries; i++) {
        esp_err_t result = esp_now_send(peerAddress, (uint8_t *)frame, sizeof(CAN_FRAME)*size);
        
        if (result == ESP_OK) {
            return true;
        }
        
        if (result == ESP_ERR_ESPNOW_NO_MEM) {
            vTaskDelay(pdMS_TO_TICKS(1)); // Pequeno delay para buffer esvaziar
            continue;
        }
        
        // Outros erros (peer não encontrado, etc)
        Serial.print("Erro ESP-NOW: ");
        Serial.println(result);
        return false;
    }
    
    Serial.println("Falha após todas as tentativas - Buffer cheio");
    return false;
}

int c=0;
void ESPNOW::onDataSent(const uint8_t*mac_addr, esp_now_send_status_t status) {
  //Serial.print("Status de envio: ");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
  if(status != ESP_NOW_SEND_SUCCESS){
    c++;
    Serial.println("Falha");
    Serial.println(c);
  }
}

void ESPNOW::setReceiveCallback(esp_now_recv_cb_t cb) {
  esp_now_register_recv_cb(cb);
}

void ESPNOW:: onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  static int c = 0;
  if (len != sizeof(FILTRO)) {
    Serial.println("Tamanho de pacote inválido");
    return;
  }
    
  //  c++;
  //  Serial.println(c);
  FILTRO received;
  memcpy(&received, incomingData, sizeof(FILTRO));
  memcpy(latestMac, info->src_addr, 6);
  //receivedFrame.data.value = c;
  Serial.println("CODE");
  Serial.println(received.code);
  Serial.println("ID");
  Serial.println(received.id);
  
  // Envia diretamente para a fila
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xQueueSend(QueueFiltroHandle, &received, 0) != pdTRUE) {
    Serial.println("Falha ao enviar para fila");
  }
  
  // Se uma task de maior prioridade foi acordada, força o context switch
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}
