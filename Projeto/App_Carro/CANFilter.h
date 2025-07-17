#ifndef CANFILTER_H
#define CANFILTER_H

#include <Arduino.h>
#include <CAN.h> // ou o cabeçalho da lib CAN usada
#include <vector>

// Definição da estrutura FILTRO
typedef struct {
  uint16_t id;
  uint8_t code;
} FILTRO;

typedef enum{
   ADD,
   REMOVE,
   ON,
   OFF,
   CLEAR
} Filter_typedef;


class CANFilter {
public:
    static bool isValidId(const CAN_FRAME& msg);
    static void insertID(uint32_t id);
    static void deleteID(uint32_t id);
    static void sortIDs();
    static void clearList();
    static void setFilterEnabled(bool enabled);
    static bool isFilterEnabled();

private:
    static std::vector<uint32_t> validIds;
    static bool filterEnabled;
};

#endif
