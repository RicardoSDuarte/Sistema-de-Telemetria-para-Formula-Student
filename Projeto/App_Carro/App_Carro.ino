#include <TWAI.h>
#include <CANFilter.h>
#include <ESPNOW.h>
#include <CAN.h>
#include <SD_CARD.h>
#include <LedControl.h>

#define Led_SDCard 1
#define MESSAGE_CAN_FRAMES_TO_SEND 10

// === Variáveis e objetos globais ===
TWAILib canbus;
CANFilter filter;
uint8_t broadcastAddress[] = {0xdc, 0xda, 0x0c, 0x57, 0x63, 0x94};
ESPNOW esp(broadcastAddress); // MAC do transmissor

bool SDC;

QueueHandle_t QueueHandle;
const int QueueElementSize = 512;
LedControl LSDcard(Led_SDCard);

// === Buffer para comandos ESP-NOW ===
volatile bool newCommandAvailable = false;
uint8_t latestCommand[10];
int latestCommandLen = 0;

// === Prototipagem ===
void taskCanFilterControl(void *pvParameters);
void Task_CANRead(void *pvParameters);
void Task_CANSend(void *pvParameters);

// === Setup principal ===
void setup() {
  Serial.begin(1000000);
  canbus.TWAI_Init();
  LSDcard.init();

  SDC = SDCard_Init();
  Serial.println(SDC ? "SD OK" : "SD Falhou");

  esp.ESPNOW_Init();

  esp_wifi_set_ps(WIFI_PS_NONE);
  Serial.println("Power saving desativado");


  //  esp_now_set_peer_rate_config(const int *peer_addr, esp_now_rate_config_t *config);

  
  esp_now_register_recv_cb(ESPNOW::onDataReceive);



  QueueFiltroHandle = xQueueCreate(QueueElementSize, sizeof(FILTRO));
  QueueHandle = xQueueCreate(QueueElementSize, sizeof(CAN_FRAME));
  if (QueueHandle == NULL) {
    Serial.println("Erro ao criar fila");
  }

  xTaskCreatePinnedToCore(Task_CANRead, "CANRead", 8192, NULL, configMAX_PRIORITIES - 1, NULL, 0);
  xTaskCreatePinnedToCore(Task_CANSend, "CANSend", 32768, NULL, configMAX_PRIORITIES - 1, NULL, 1);
  xTaskCreatePinnedToCore(taskCanFilterControl, "CanFilterControl", 8192, NULL, configMAX_PRIORITIES - 1, NULL, 1);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void Task_CANRead(void *pvParameters) {
  int c=0;
  CAN_FRAME msg;
  while (true) {
    msg = canbus.Check_frame();
    while (msg.id != 0) {
      if(xQueueSend(QueueHandle, &msg, 0) != pdPASS) Serial.println("Erro ao enviar para a QueueHandle");
      msg = canbus.Check_frame();
    }
    taskYIELD(); // ou vTaskDelay(0);
  }
}

void Task_CANSend(void *pvParameters) {
    int c = 0;
    CAN_FRAME msg;
    CAN_FRAME msg_array[MESSAGE_CAN_FRAMES_TO_SEND];
    uint8_t can_frame_idx = 0;
    unsigned long now;
    static unsigned long lastFlush = 0;
    bool ESPFULL =false;

    while (true) {

      //só ler se o destinatario disponivel
      // Recebe mensagem da queue
      //if(!ESPFULL) // Só vai buscar uma frame à fila caso os buffer do esp e do sd card não tiverem cheios
      //{
        while (xQueueReceive(QueueHandle, &msg, pdMS_TO_TICKS(10)) == pdPASS) 
        {
          
          if (SDC) // Guarda no SD card se ativo 
          {
            Save_GVRET(msg);
          }
          
          if (!CANFilter::isFilterEnabled() || filter.isValidId(msg)) // Só envia se o filtro estiver desligado ou se o id estiver na listas
          { 
            msg_array[can_frame_idx++] = msg;
            
          }
          if(can_frame_idx>=MESSAGE_CAN_FRAMES_TO_SEND){
            ESPFULL = esp.sendFrame_v2(msg_array, 5, MESSAGE_CAN_FRAMES_TO_SEND);
            if (!ESPFULL)
            {
              Serial.println("Falha crítica no envio ESP-NOW");
            }
            can_frame_idx=0;
          }
        //}
      }  
        // Flush periódico do buffer para SD
      now = xTaskGetTickCount();
      if (now - lastFlush >= 1000) {  // Flush a cada 1000ms
          if (SDC) {
              flushBufferToSD();
          }
          lastFlush = now;
      }

      // Delay para permitir outras tasks
      vTaskDelay(pdMS_TO_TICKS(1));  
  }
}


void taskCanFilterControl(void *pvParameters) {
  FILTRO f;
  bool fof = CANFilter::isFilterEnabled();
  while (1) {
    if (xQueueReceive(QueueFiltroHandle,&f,0) == pdPASS) {
      
      if(f.code == ADD){
        Serial.println("add");
        filter.insertID(f.id);
      }
      else if(f.code == REMOVE){
        Serial.println("Remove");
        filter.deleteID(f.id);
      }
      else if(f.code == ON){
        Serial.println("Filter ON");
        filter.setFilterEnabled(true);
      }
      else if(f.code == OFF){
        Serial.println("Filter OFF");
        filter.setFilterEnabled(false);
      }
      else if(f.code == CLEAR){
        Serial.println("Filter CLEAR");
        filter.clearList();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}
