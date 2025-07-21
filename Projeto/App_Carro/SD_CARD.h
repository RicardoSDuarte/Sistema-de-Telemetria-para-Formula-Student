#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include "CAN.h"
#define LOG_BUFFER_SIZE 2048  // 20 frames × 24 bytes + margem de segurança (~25% extra)

// Inicialização do cartão SD
bool SDCard_Init();

// Salva um frame no buffer e escreve no cartão SD se necessário
void Save_GVRET(const CAN_FRAME &frame);

// Escreve o conteúdo do buffer no cartão SD imediatamente
void flushBufferToSD();

// Fecha o ficheiro e salva dados pendentes
void closeLogFile();

#endif
