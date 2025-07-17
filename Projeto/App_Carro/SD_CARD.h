/************************************************************//**
 * @file		SD_CARD.h
 * @brief		SD_CARD - Header File
 * @author		Ricardo Duarte
 * @version		1.0
 * @date		19 fev 2025
 ************************************************************/
/**
 * @defgroup DRIVERS DRIVERS
 * @{
 * @addtogroup SD_CARD SD_CARD
 * @{Functions for the SD_CARD Sensor
 */


#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "CAN.h"

/**
 * @brief Inicializa o cartão SD.
 */
bool SDCard_Init();

/**
 * @brief Lê dados do Serial e salva em um arquivo.
 */
void saveMessageToFile();

/**
 * @brief Lista o conteúdo de um diretório.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param dirname Nome do diretório a ser listado.
 * @param levels Nível de profundidade a ser listado.
 */
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

/**
 * @brief Cria um diretório.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do diretório a ser criado.
 */
void createDir(fs::FS &fs, const char * path);

/**
 * @brief Remove um diretório.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do diretório a ser removido.
 */
void removeDir(fs::FS &fs, const char * path);

/**
 * @brief Lê um arquivo e imprime seu conteúdo no Serial.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do arquivo a ser lido.
 */
void readFile(fs::FS &fs, const char * path);

/**
 * @brief Escreve uma mensagem em um arquivo.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do arquivo a ser escrito.
 * @param message Mensagem a ser escrita no arquivo.
 */
void writeFile(fs::FS &fs, const char * path, const char * message);

/**
 * @brief Anexa uma mensagem a um arquivo existente.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do arquivo a ser anexado.
 * @param message Mensagem a ser anexada ao arquivo.
 */
void appendFile(fs::FS &fs, const char * path, const char * message);

/**
 * @brief Renomeia um arquivo.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path1 Caminho do arquivo original.
 * @param path2 Novo nome para o arquivo.
 */
void renameFile(fs::FS &fs, const char * path1, const char * path2);

/**
 * @brief Deleta um arquivo.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do arquivo a ser removido.
 */
void deleteFile(fs::FS &fs, const char * path);

/**
 * @brief Testa operações de entrada/saída de arquivo.
 * 
 * @param fs Sistema de arquivos a ser utilizado.
 * @param path Caminho do arquivo a ser testado.
 */
void testFileIO(fs::FS &fs, const char * path);

void Save_GVRET(const CAN_FRAME &frame);

#endif
