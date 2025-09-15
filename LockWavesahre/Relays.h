#include "WS_Relay.h"

#define REL1 GPIO_PIN_CH1
#define REL2 GPIO_PIN_CH2
#define REL3 GPIO_PIN_CH3
#define REL4 GPIO_PIN_CH4
#define REL5 GPIO_PIN_CH5
#define REL6 GPIO_PIN_CH6
#define REL7 GPIO_PIN_CH7
#define REL8 GPIO_PIN_CH8

unsigned long relay1StartTime = 0;
unsigned long relay2StartTime = 0;
unsigned long relay3StartTime = 0;
unsigned long relay4StartTime = 0;
unsigned long relay5StartTime = 0;
unsigned long relay6StartTime = 0;
unsigned long relay7StartTime = 0;
unsigned long relay8StartTime = 0;
byte unlockTimeSeconds = 30;

byte openRelay[] = {0x01,         // Device address = 1
                    0x05, 0x02,   // flash open
                    0x00,         // Relay Address  = 0
                    0x00, 0x07,   // Interval time  = 3s, Delay time is data*100ms Value: 0x001E, delay: 30*100MS = 3000MS
                    0x00, 0x00,   // CRC16
                    };  

void setupRelay() {
  GPIO_Init();
  I2C_Init();
  Relay_Init();
}

void loopRelay() {
  // check timeout... do not use direct < comparsion due to possible variable overflow
  uint64_t diff = xTaskGetTickCount() - relay1StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL1);
  }
  diff = xTaskGetTickCount() - relay2StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL2);
  }
  
  diff = xTaskGetTickCount() - relay3StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL3);
  }
  diff = xTaskGetTickCount() - relay4StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL4);
  }
  diff = xTaskGetTickCount() - relay5StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL5);
  }
  diff = xTaskGetTickCount() - relay6StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL6);
  }
  diff = xTaskGetTickCount() - relay7StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL7);
  }
  diff = xTaskGetTickCount() - relay8StartTime;
  if (diff > (1000 * unlockTimeSeconds)) {
    Relay_Closs(REL8);
  }

}

// Compute the MODBUS RTU CRC
uint16_t ModRTU_CRC(byte buf[], int len) {
  uint16_t crc = 0xFFFF;
  
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc
  
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;  
}

void printHexByte(uint8_t num) {
  char Hex_Array[3];

  sprintf(Hex_Array, "%02X ", num);
  Serial.print(Hex_Array);
}

void printHex(byte d[], int L) {
  for (int i = 0; i < L; i++) {
    printHexByte(d[i]);
  }
  Serial.println();
}

void sendData(byte d[], int L) {
  byte crc[2];

  uint16_t crcc = ModRTU_CRC(d, L - 2);
  uint8_t crc_l = (uint8_t)(crcc >> 8);
  uint8_t crc_h = (uint8_t)crcc;
  d[L-2] = crc_h;
  d[L-1] = crc_l;

  Serial1.write(d, L);
  Serial1.flush();

  printHex(d, L);

  Serial.println();
}

void unlockDoor(byte relayNumber, byte timeSeconds) {
  uint16_t time = timeSeconds * 10;

  openRelay[3] = relayNumber - 1;
  openRelay[4] = (uint8_t) (time >> 8);
  openRelay[5] = (uint8_t) time;
  sendData(openRelay, sizeof(openRelay)); 
}

void unlockLocalDoor(byte relayNumber, byte timeSeconds) {
  unlockTimeSeconds = timeSeconds;
  uint16_t time = timeSeconds * 10;

  uint8_t channel = 0;
  if (relayNumber == 1) {
    channel = REL1;
    relay1StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 2) {
    channel = REL2;
    relay2StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 3) {
    channel = REL3;
    relay3StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 4) {
    channel = REL4;
    relay4StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 5) {
    channel = REL5;
    relay5StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 6) {
    channel = REL6;
    relay6StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 7) {
    channel = REL7;
    relay7StartTime = xTaskGetTickCount();
  } else
  if (relayNumber == 8) {
    channel = REL8;
    relay8StartTime = xTaskGetTickCount();
  }

  if (channel > 0) {
    Relay_Open(channel); 
  }
}