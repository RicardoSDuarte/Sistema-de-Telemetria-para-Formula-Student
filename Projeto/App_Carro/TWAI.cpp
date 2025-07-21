#include "TWAI.h"
#include <Arduino.h>


TWAILib::TWAILib() : driver_installed(false) {}

bool TWAILib::TWAI_Init() {
    // 1. Configura o pino EN como saída e inicia desabilitado (LOW)
    pinMode(CAN_EN_PIN, OUTPUT);
    digitalWrite(CAN_EN_PIN, LOW);  // Desabilita o transceptor inicialmente

    // 2. Configuração padrão do TWAI/CAN
    // twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
        // 2. Configuração personalizada do TWAI/CAN com filas maiores
    twai_general_config_t g_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = (gpio_num_t)TX_PIN,
        .rx_io = (gpio_num_t)RX_PIN,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 32,
        .rx_queue_len = 256,
        .alerts_enabled = TWAI_ALERT_NONE,
        .clkout_divider = 0,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .general_flags = {0}
    };
    
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // 3. Instala o driver TWAI
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("Failed to install driver");
        digitalWrite(CAN_EN_PIN, LOW);  // Garante que o transceptor permaneça desabilitado em caso de falha
        return false;
    }

    // 4. Inicia o driver TWAI
    if (twai_start() != ESP_OK) {
        Serial.println("Failed to start driver");
        digitalWrite(CAN_EN_PIN, LOW);  // Desabilita o transceptor se a inicialização falhar
        return false;
    }

    // 5. Habilita o transceptor CAN (pino EN HIGH)
    digitalWrite(CAN_EN_PIN, HIGH);
    Serial.println("TWAI transceiver enabled");

    // 6. Configura os alertas do TWAI
    uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
    if (twai_reconfigure_alerts(alerts_to_enable, NULL) != ESP_OK) {
        Serial.println("Failed to reconfigure alerts");
        // Não desativa o transceptor aqui (já está operacional)
        return false;
    }

    Serial.println("TWAI driver installed and started");
    driver_installed = true;
    return true;
}


void TWAILib::checkAlerts() {
    if (!driver_installed) return;

    uint32_t alerts_triggered;
    if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS)) == ESP_OK) {
        handleAlerts(alerts_triggered);
        receiveMessages();
    }
}

CAN_FRAME TWAILib::Check_frame() {
    CAN_FRAME frame = {};
    if (!driver_installed) return frame;

    uint32_t alerts_triggered;
    if (twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS)) == ESP_OK) {
        handleAlerts(alerts_triggered);
        //frame = Receive_CAN();
    
        if (alerts_triggered & TWAI_ALERT_RX_DATA) {
            frame = Receive_CAN();  // You must ensure this function handles empty/error cases
        }

    }
    return frame;
}


void TWAILib::receiveMessages() {
    twai_message_t message;
    while (twai_receive(&message, 0) == ESP_OK) {
        handleRxMessage(message);
    }
}

CAN_FRAME TWAILib::Receive_CAN() {
    CAN_FRAME FCAN;
    twai_message_t message;
    while (twai_receive(&message, 0) == ESP_OK) {
        FCAN = Read_Message(message);
    }
    return FCAN;
}

void TWAILib::handleRxMessage(twai_message_t &message) {
    if (message.extd) {
        Serial.println("Message is in Extended Format");
    } else {
        Serial.println("Message is in Standard Format");
    }

    Serial.printf("ID: %lx\nByte:", message.identifier);
    if (!(message.rtr)) {
        for (int i = 0; i < message.data_length_code; i++) {
            Serial.printf(" %d = %02x,", i, message.data[i]);
        }
        Serial.println("");
    }
}

CAN_FRAME TWAILib::Read_Message(twai_message_t &message) 
{
    CAN_FRAME msg;

    msg.timestamp = millis();
    msg.id = message.identifier;
    msg.length = message.data_length_code;
    memcpy(msg.data.uint8, message.data, message.data_length_code);


    return msg;

}

void TWAILib::handleAlerts(uint32_t alerts_triggered) {
    twai_status_info_t status;
    twai_get_status_info(&status);

    if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
        Serial.println("Alert: TWAI controller has become error passive.");
    }   

    if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
        Serial.println("Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
        Serial.printf("Bus error count: %lu\n", status.bus_error_count);
    }

    if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {
        Serial.println("Alert: The RX queue is full causing a received frame to be lost.");
        Serial.printf("RX buffered: %lu\t", status.msgs_to_rx);
        Serial.printf("RX missed: %lu\t", status.rx_missed_count);
        Serial.printf("RX overrun %lu\n", status.rx_overrun_count);
    }
}

void TWAILib::enableTransceiver() {
    digitalWrite(CAN_EN_PIN, HIGH);
    Serial.println("CAN Transceiver enabled");
}

void TWAILib::disableTransceiver() {
    digitalWrite(CAN_EN_PIN, LOW);
    Serial.println("CAN Transceiver disabled");
}
