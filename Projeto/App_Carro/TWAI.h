#ifndef TWAI_H
#define TWAI_H

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "driver/twai.h"
#include "CAN.h"
//Define RX and TX pins for CAN transceiver
#define RX_PIN 6 // GPIO6
#define TX_PIN 5 // GPIO5
#define CAN_EN_PIN 4 // GPIO4

#define POLLING_RATE_MS 1000

class TWAILib {
public:
    TWAILib();
    bool TWAI_Init();
    void checkAlerts();
    void receiveMessages();
    CAN_FRAME Receive_CAN();
    CAN_FRAME Check_frame();
    void  disableTransceiver();
    void  enableTransceiver();


private:
    bool driver_installed;
    void handleRxMessage(twai_message_t &message);
    void handleAlerts(uint32_t alerts_triggered);
    CAN_FRAME Read_Message(twai_message_t &message);
};

#endif  TWAI_H
