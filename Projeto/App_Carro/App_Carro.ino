#include <TWAI.h>
#include <CANFilter.h>
#include <ESPNOW.h>
#include <CAN.h>
#include <SD_CARD.h>
#include <LedControl.h>

#define Led_SDCard 1

// === Variáveis e objetos globais ===
TWAILib canbus;
CANFilter filter;
uint8_t broadcastAddress[] = {0xdc, 0xda, 0x0c, 0x57, 0x63, 0x94};
ESPNOW esp(broadcastAddress); // MAC do transmissor

bool SDC;

QueueHandle_t QueueHandle;
const int QueueElementSize = 128;
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

  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_5M_L);

  //  esp_now_set_peer_rate_config(const int *peer_addr, esp_now_rate_config_t *config);

  
  esp_now_register_recv_cb(ESPNOW::onDataReceive);



  QueueFiltroHandle = xQueueCreate(QueueElementSize, sizeof(FILTRO));
  QueueHandle = xQueueCreate(QueueElementSize, sizeof(CAN_FRAME));
  if (QueueHandle == NULL) {
    Serial.println("Erro ao criar fila");
  }

  xTaskCreatePinnedToCore(Task_CANRead, "CANRead", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(Task_CANSend, "CANSend", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskCanFilterControl, "CanFilterControl", 8192, NULL, 1, NULL, 1);
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
      
      c++;
   //   if(c>68550){
    //    Serial.println(c);
     // }
      if (SDC) {
        Save_GVRET(msg);
        // LSDcard.led_off(); // use se quiser indicar SD OK
      }

      //if (CANFilter::isFilterEnabled()) {
      //  if (filter.isValidId(msg)) {
          xQueueSend(QueueHandle, &msg, 0);
      //  }
      //} 
      msg = canbus.Check_frame();
    }
    taskYIELD(); // ou vTaskDelay(0);
  }
}

void Task_CANSend(void *pvParameters) {
  int c=0;
  CAN_FRAME msg;
  while (true) {
    if (xQueueReceive(QueueHandle, &msg, portMAX_DELAY) == pdPASS) {

    //c++;
     //Serial.println(c);
    esp.sendFrame(msg);
    }
    vTaskDelay(0);  // yield rápido
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
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}
