#include <string.h>
#include "SD_CARD.h"

char logBuffer[LOG_BUFFER_SIZE]; // LOG_BUFFER_SIZE = 640
size_t bufferOffset = 0;
File logFile;
int frameCount = 0; // Counter for frames


bool SDCard_Init() {
    if (!SD.begin(21,SPI,40000000)) {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    // Criação do cabeçalho CSV se necessário
    if (!SD.exists("/log_can.csv")) {
        logFile = SD.open("/log_can.csv", FILE_WRITE);
        if (logFile) {
            logFile.println("NEW_LOG");
            //logFile.close();
            Serial.println("Cabeçalho CSV criado em /log_can.csv");
        } else {
            Serial.println("Erro ao criar /log_can.csv");
        }
    } else {
        Serial.println("Ficheiro CSV já existe, cabeçalho não será duplicado.");
    }
    // Abre ficheiro para logging contínuo
    // logFile = SD.open("/log_can.csv", FILE_APPEND);
    // if (!logFile) {
    //     Serial.println("Erro ao abrir log_can.csv para escrita contínua.");
    //     return false;
    // }

    frameCount = 0; // Initialize frame counter
    return true;
}
// Flush do buffer para o cartão SD
void flushBufferToSD() {
    if (bufferOffset > 0 && logFile) {
        logFile.write((const uint8_t*)logBuffer, bufferOffset);
        logFile.flush(); // Force write to SD card
        bufferOffset = 0;
        frameCount = 0; // Reset frame counter after flush
        //Serial.println("Buffer flushed to SD card"); // Optional: for debugging
    }
}


// Função de logging com buffer
void Save_GVRET(const CAN_FRAME &frame) {
    // if (bufferOffset >= LOG_BUFFER_SIZE - 30) { // evitar overflow
    //     flushBufferToSD();
    // }
    memcpy(logBuffer + bufferOffset,&frame ,sizeof(CAN_FRAME));
    // int n = snprintf(logBuffer + bufferOffset, LOG_BUFFER_SIZE - bufferOffset,
    //     "%d,0x%X,%d,1,0,%d,",
    //     frame.timestamp,
    //     frame.id,
    //     frame.extended,
    //     frame.length
    // );
    bufferOffset += sizeof(CAN_FRAME);
    // for (int i = 0; i < frame.length; i++) {
    //     bufferOffset += snprintf(logBuffer + bufferOffset, LOG_BUFFER_SIZE - bufferOffset, "0x%02X,", frame.data.byte[i]);
    // }
    // for (int i = frame.length; i < 8; i++) {
    //     bufferOffset += snprintf(logBuffer + bufferOffset, LOG_BUFFER_SIZE - bufferOffset, ",");
    // }
    // logBuffer[bufferOffset - 1] = '\n'; // substitui última vírgula~

    frameCount++; // Increment frame counter
    
    // Flush every 20 frames
    if (frameCount >= 40) {
        flushBufferToSD();
    }
}
// Fecha o ficheiro ao terminar
void closeLogFile() {
    flushBufferToSD();
    if (logFile) {
        logFile.close();
    }
}