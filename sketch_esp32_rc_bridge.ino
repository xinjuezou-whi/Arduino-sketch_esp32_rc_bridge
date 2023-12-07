/******************************************************************
Bridge the SBUS between Remote controller and other IIC devices

Features:
- SBUS signal
- I2C with master
- xxx

Dependency:
- xxx

Written by Xinjue Zou, xinjue.zou@outlook.com

Apache License Version 2.0, check LICENSE for more information.
All text above must be included in any redistribution.

Changelog:
2023-11-21: initial version
2023-xx-xx: xxx
******************************************************************/
#include "sbus.h"
#include <HardwareSerial.h>
#include <Wire.h>

const String VERSION("00.02");

// hardware serial: UART1
HardwareSerial SBUSS(1);

// SBUS object, reading SBUS
bfs::SbusRx sbus_rx(&SBUSS, 16, 17, true);
// SBUS object, writing SBUS
bfs::SbusTx sbus_tx(&SBUSS, 16, 17, true);
// SBUS data
bfs::SbusData data;

// RC related
const uint8_t MAX_CHANNELS = 16;
int from_min_max_[MAX_CHANNELS][2] = 
{ 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800}, 
  {200, 1800},
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}, 
  {994, 1998}
};

int to_min_max_[MAX_CHANNELS][2] = 
{ 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100},
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}, 
  {0, 100}
};

// iic related
const int DEVICE_ADDR = 8;
const uint8_t SEND_DATA_SIZE = MAX_CHANNELS; // 1 byte for ecah rc channel
const uint8_t RECV_DATA_SIZE = 32; // Wire library has a softlimit up to 32 bytes
byte raw_data_[SEND_DATA_SIZE] = { 0 };
byte send_data_[SEND_DATA_SIZE] = { 0 };
byte recv_data_[RECV_DATA_SIZE] = { 0 };

// function that executes whenever data is read by master
// this function is registered as an event, see setup()
void requestEvent()
{
  // check if data is valid
  for (uint8_t i = 0; i < SEND_DATA_SIZE; ++i)
  {
    send_data_[i] = raw_data_[i];
    if (send_data_[i] < to_min_max_[i][0] || send_data_[i] > to_min_max_[i][1])
    {
      for (uint8_t j = 0; j < SEND_DATA_SIZE; ++j)
      {
        send_data_[j] = 255;
      }

      break;
    }
  }

  Wire.write(send_data_, SEND_DATA_SIZE);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int Length)
{
  for (int i = 0; i < Length; ++i)
  {
    recv_data_[i] = Wire.read();
  }
}

void setup() {
  /* Serial to display data */
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(VERSION);
  Serial.println("\nReady");
  /* Begin the SBUS communication */
  sbus_rx.Begin();
  sbus_tx.Begin();

  // iic
  Wire.begin(DEVICE_ADDR);      // join i2c bus with address
  Wire.onRequest(requestEvent); // register events
  Wire.onReceive(receiveEvent);
}

void loop () {
  if (sbus_rx.Read())
  {
    /* Grab the received data */
    data = sbus_rx.data();
    /* Display the received data */
    for (int8_t i = 0; i < data.NUM_CH; i++)
    {
      raw_data_[i] = byte(map(int(data.ch[i]), from_min_max_[i][0], from_min_max_[i][1], to_min_max_[i][0], to_min_max_[i][1]));
#ifdef DEBUG
      // Serial.print(data.ch[i]);
      Serial.print(raw_data_[i]);
      Serial.print(",");
#endif
    }
#ifdef DEBUG
    /* Display lost frames and failsafe data */
    Serial.print(data.lost_frame);
    Serial.print("_");
    Serial.println(data.failsafe);
#endif
  }
}
