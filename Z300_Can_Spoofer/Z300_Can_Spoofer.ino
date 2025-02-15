/* Can Comms on Car:
// BMS - done
// Charger
// Heater  - Done
// Shunt
// Inverter
// Car
*/

/* BMS
/* 0x355 53 00 64 00 3E 03 02 00 - SoC / SoH
 * 0x356 58 92 00 01 51 00 52 0F - pack voltage / avg temp / avg voltage
 * 0x35A 00 00 00 00 00 00 00 00 - warnings 
 * 0x373 12 0F 50 0F 11 01 19 01 - high/low voltages/temps
 * 0x379 00 10 00 10 00 10 00 10 - remaining amp hours / range estimate
 * 
 * HEATER
 * 0x398 ID 00 09 00 3B 3B 00 00 00 at 100ms byte 3 seems to be water temperature inlet, byte 4 water temperature outlet
 * 0x630 ID 00 00 00 00 00 00 00 00 at 500ms
 * 0x062D ID 00 00 00 00 00 00 00 00 at 500ms
 * 0x06BD ID 00 00 00 00 00 00 00 00 at 500ms
 * When HV is connected 0x398 changes to the message below.
 * 0x398 ID 00 00 00 3B 3B 00 00 00 at 100ms

*/

#include <SPI.h>
#include <mcp2515.h>

#define LEDpin 3
#define sendModePin 6
#define quietModePin 5
int sendMode;
int quietMode;

struct can_frame canMsg1;
struct can_frame canMsg2;
struct can_frame canMsg3;
struct can_frame canMsg4;

MCP2515 mcp2515(10);

void setup() {
  /* BMS */
  canMsg1.can_id  = 0x355;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x53;
  canMsg1.data[1] = 0x00;
  canMsg1.data[2] = 0x64;
  canMsg1.data[3] = 0x00;
  canMsg1.data[4] = 0x3E;
  canMsg1.data[5] = 0x03;
  canMsg1.data[6] = 0x02;
  canMsg1.data[7] = 0x00;

  canMsg2.can_id  = 0x356;
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = 0x58;
  canMsg2.data[1] = 0x92;
  canMsg2.data[2] = 0x00;
  canMsg2.data[3] = 0x01;
  canMsg2.data[4] = 0x51;
  canMsg2.data[5] = 0x00;
  canMsg2.data[6] = 0x52;
  canMsg2.data[7] = 0x0F;

  canMsg3.can_id  = 0x35A;
  canMsg3.can_dlc = 8;
  canMsg3.data[0] = 0x00;
  canMsg3.data[1] = 0x00;
  canMsg3.data[2] = 0x00;
  canMsg3.data[3] = 0x00;
  canMsg3.data[4] = 0x00;
  canMsg3.data[5] = 0x00;
  canMsg3.data[6] = 0x00;
  canMsg3.data[7] = 0x00;

  canMsg4.can_id  = 0x373;
  canMsg4.can_dlc = 8;
  canMsg4.data[0] = 0x12;
  canMsg4.data[1] = 0x0F;
  canMsg4.data[2] = 0x50;
  canMsg4.data[3] = 0x0F;
  canMsg4.data[4] = 0x11;
  canMsg4.data[5] = 0x01;
  canMsg4.data[6] = 0x19;
  canMsg4.data[7] = 0x01;

/* HEATER */
  canMsg5.can_id  = 0x398;
  canMsg5.can_dlc = 8;
  canMsg5.data[0] = 0x00;
  canMsg5.data[1] = 0x00;
  canMsg5.data[2] = 0x00;
  canMsg5.data[3] = 0x3B;
  canMsg5.data[4] = 0x3B;
  canMsg5.data[5] = 0x00;
  canMsg5.data[6] = 0x00;
  canMsg5.data[7] = 0x00;
  
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);
  pinMode(sendModePin, INPUT_PULLUP);
  pinMode(quietModePin, INPUT_PULLUP);
  
  Serial.println("Z300S CANBus Spoofer");
  Serial.println("Plays back canbus messages from Z3 and EV component canbus including: BMS");
}

void loop() {
  sendMode = digitalRead(sendModePin);
  quietMode = digitalRead(quietModePin);
  if(sendMode == LOW && quietMode == HIGH) {
    Serial.println("Send Mode");
    digitalWrite(LEDpin, HIGH);
    mcp2515.sendMessage(&canMsg1);
    mcp2515.sendMessage(&canMsg2);
    mcp2515.sendMessage(&canMsg3);
    mcp2515.sendMessage(&canMsg4);
    mcp2515.sendMessage(&canMsg5);    
    Serial.println("Messages sent");
    delay(500);
    digitalWrite(LEDpin, LOW);
    delay(500);
  }  else if (sendMode == HIGH && quietMode == LOW){
    Serial.println("Quiet Mode");
    digitalWrite(LEDpin, HIGH);
    delay(1000);
    } else {}
  } 
