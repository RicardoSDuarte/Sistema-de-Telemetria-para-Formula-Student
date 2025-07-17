#include "SD_CARD.h"

bool SDCard_Init() {
    // Serial.begin(115200);
    // while (!Serial);

    if (!SD.begin(21)) {
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

    
    if (!SD.exists("/log_can.csv")) {
        File file = SD.open("/log_can.csv", FILE_WRITE);
        if (file) {
            file.println("Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8");
            file.close();
            Serial.println("Cabeçalho CSV criado em /log_can.csv");
        } else {
            Serial.println("Erro ao criar /log_can.csv");
        }
    } else {
        Serial.println("Ficheiro CSV já existe, cabeçalho não será duplicado.");
    }
    return true;
}

// Função para ler dados do Serial e salvar no arquivo
void saveMessageToFile() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');  // Lê a linha do Serial Monitor
        appendFile(SD, "/mensagem.txt", (input + "\n").c_str());  // Escreve no ficheiro mensagem.txt
        Serial.println("Mensagem guardada no SD");
    }
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


void Save_GVRET(const CAN_FRAME &frame)
{
    char output[200]; // Buffer suficiente para a linha completa
    int offset = 0;

    // Adiciona os campos no formato especificado
    offset += sprintf(output + offset, "%d,", frame.timestamp);         // Time Stamp
    offset += sprintf(output + offset, "0x%X,", frame.id);              // ID em hexadecimal
    offset += sprintf(output + offset, "%d,", frame.extended);          // Extended (0 ou 1)
    offset += sprintf(output + offset, "%d,", 1);                       // Dir (0 = envio, 1 = RTR)
    offset += sprintf(output + offset, "0,");                           // Bus (número do barramento)
    offset += sprintf(output + offset, "%d,", frame.length);            // LEN

    // Dados D1 a D8 em hexadecimal
    for (int i = 0; i < frame.length; i++)
    {
        offset += sprintf(output + offset, "0x%02X,", frame.data.byte[i]);
    }

    // Preenche os dados restantes com vírgulas se frame.length < 8
    for (int i = frame.length; i < 8; i++)
    {
        offset += sprintf(output + offset, ",");
    }

    // Substitui a última vírgula por quebra de linha
    if (offset > 0 && output[offset - 1] == ',') {
        output[offset - 1] = '\n';
    } else {
        output[offset++] = '\n';
    }

    // Finaliza a string
    output[offset] = '\0';

    // Salva e imprime
    appendFile(SD, "/log_can.csv", output);
    //Serial.print("Gravado (CSV): ");
    //Serial.print(output);
}

