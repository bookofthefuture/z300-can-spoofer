/* Can Comms on Car:
// BMS - done
// Charger
// Heater  - Done
// Shunt
// Inverter
// Car
*/

/* BMS
/* 0x355 53 00 64 00 3E 03 02 00 at 1000ms for test purposes - SoC / SoH
 * 0x356 58 92 00 01 51 00 52 0F at 1000ms for test purposes  - pack voltage / avg temp / avg voltage
 * 0x35A 00 00 00 00 00 00 00 00 at 1000ms for test purposes  - warnings 
 * 0x373 12 0F 50 0F 11 01 19 01 at 1000ms for test purposes  - high/low voltages/temps
 * 0x379 00 10 00 10 00 10 00 10 at 1000ms for test purposes  - remaining amp hours / range estimate - NOT IN TEST
 * 
 * BMS CHARGE CONTROL
 * 0x285 00 00 B6 00 00 00 00 00 at 10ms
 * 0x286 00 4A 50 25 00 00 00 00 at 100ms for testing
 * 
 * CHARGER
 * 0x377 05 9E 00 80 42 43 3E 22 at 100ms for testing - 8bytes DC-DC converter status
 * B0+B1 = 12V Battery voltage (h04DC=12,45V -> 0,01V/bit)
 * B2+B3 = 12V Supply current  (H53=8,3A -> 0,1A/bit)  
 * B4 = Temperature 1    (starts at -40degC, +1degC/bit) 
 * B5 = Temperature 2    (starts at -40degC, +1degC/bit) 
 * B6 = Temperature 3    (starts at -40degC, +1degC/bit) 
 * B7 = Statusbyte     (0x20=standby, 0x21=error, 0x22=in operation)
 * 
 * 0x389 9F 00 00 4E 54 C0 00 00 at 100ms for testing  
 * B0 = Battery Voltage (as seen by the charger), needs to be scaled x 2, so can represent up to 255*2V; used to monitor battery during charge 
 * B1 = Charger supply voltage, no scaling needed  
 * B6 = AC Supply Current x 10 
 * B7 = DC side current x 10 (should equal commanded current)
 * 
 * 0x38A 45 52 A7 19 04 00 00 00 at 100ms for testing 
 * B0 = temp x 2?  
 * B1 = temp x 2?  
 * B3 = EVSE Control Duty Cycle (granny cable ~26 = 26%)  
 * 
 * HEATER
 * 0x398 00 09 00 3B 3B 00 00 00 at 100ms byte 3 seems to be water temperature inlet, byte 4 water temperature outlet
 * 0x630 00 00 00 00 00 00 00 00 at 500ms - NOT USED FOR TESTING
 * 0x62D 00 00 00 00 00 00 00 00 at 500ms - NOT USED FOR TESTING
 * 0x6BD 00 00 00 00 00 00 00 00 at 500ms - NOT USED FOR TESTING
 * When HV is connected 0x398 changes to the message below.
 * 0x398 ID 00 00 00 3B 3B 00 00 00 at 100ms
 * 
 * HEATER CONTROLLER
 * 0x188 03 50 4D 00 00 00 00 at 100ms
*/

#include <SPI.h>
#include <mcp2515.h>
#include <TaskScheduler.h> // https://github.com/arkhipenko/TaskScheduler

#define LEDpin 7
#define sendModePin 6
#define quietModePin 5
int LEDstate = 0;

void ms10Task();
void ms100Task();
void ms500Task();
void ms1000Task();

Task ms10(10, -1, &ms10Task);
Task ms100(100, -1, &ms100Task);
Task ms500(500, -1, &ms500Task);
Task ms1000(1000, -1, &ms1000Task);

Scheduler runner;

int ms10count = 0;
int ms100count = 0;
int ms500count = 0;

struct can_frame canMsg1;
struct can_frame canMsg2;
struct can_frame canMsg3;
struct can_frame canMsg4;
struct can_frame canMsg5;
struct can_frame canMsg6;
struct can_frame canMsg7;
struct can_frame canMsg8;
struct can_frame canMsg9;
struct can_frame canMsg10;

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

/* BMS CHARGE CONTROL */
  canMsg5.can_id  = 0x285;
  canMsg5.can_dlc = 8;
  canMsg5.data[0] = 0x00;
  canMsg5.data[1] = 0x00;
  canMsg5.data[2] = 0xB6;
  canMsg5.data[3] = 0x00;
  canMsg5.data[4] = 0x00;
  canMsg5.data[5] = 0x00;
  canMsg5.data[6] = 0x00;
  canMsg5.data[7] = 0x00;

  canMsg6.can_id  = 0x286;
  canMsg6.can_dlc = 8;
  canMsg6.data[0] = 0x00;
  canMsg6.data[1] = 0x4A;
  canMsg6.data[2] = 0x50;
  canMsg6.data[3] = 0x25;
  canMsg6.data[4] = 0x00;
  canMsg6.data[5] = 0x00;
  canMsg6.data[6] = 0x00;
  canMsg6.data[7] = 0x00;

/* CHARGER */
  canMsg7.can_id  = 0x377;
  canMsg7.can_dlc = 8;
  canMsg7.data[0] = 0x05;
  canMsg7.data[1] = 0x90;
  canMsg7.data[2] = 0x00;
  canMsg7.data[3] = 0x80;
  canMsg7.data[4] = 0x42;
  canMsg7.data[5] = 0x43;
  canMsg7.data[6] = 0x3E;
  canMsg7.data[7] = 0x22;

  canMsg8.can_id  = 0x389;
  canMsg8.can_dlc = 8;
  canMsg8.data[0] = 0x9F;
  canMsg8.data[1] = 0x00;
  canMsg8.data[2] = 0x00;
  canMsg8.data[3] = 0x4E;
  canMsg8.data[4] = 0x54;
  canMsg8.data[5] = 0xC0;
  canMsg8.data[6] = 0x00;
  canMsg8.data[7] = 0x00;

  canMsg9.can_id  = 0x38A;
  canMsg9.can_dlc = 8;
  canMsg9.data[0] = 0x45;
  canMsg9.data[1] = 0x52;
  canMsg9.data[2] = 0xA7;
  canMsg9.data[3] = 0x19;
  canMsg9.data[4] = 0x04;
  canMsg9.data[5] = 0x00;
  canMsg9.data[6] = 0x00;
  canMsg9.data[7] = 0x00;

/* HEATER */
  canMsg10.can_id  = 0x398;
  canMsg10.can_dlc = 8;
  canMsg10.data[0] = 0x00;
  canMsg10.data[1] = 0x00;
  canMsg10.data[2] = 0x00;
  canMsg10.data[3] = 0x3B;
  canMsg10.data[4] = 0x3B;
  canMsg10.data[5] = 0x00;
  canMsg10.data[6] = 0x00;
  canMsg10.data[7] = 0x00;
  
  
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

  runner.init();

  runner.addTask(ms10);
  ms10.enable();

  runner.addTask(ms100);
  ms100.enable();

  runner.addTask(ms100);
  ms500.enable();

  runner.addTask(ms1000);
  ms1000.enable();
}

void loop() {
  if (sendModePin == HIGH && quietModePin == LOW){
    digitalWrite(LEDpin, HIGH);
    LEDstate = 1;
  }
} 

void ms10Task() {
  if(sendModePin == LOW && quietModePin == HIGH) {
      mcp2515.sendMessage(&canMsg5);  
      ms10count++;
      if(ms10count > 100) {
        ms10count = 0;
        Serial.println("Ms10 tasks running");
      }
  } 
}

void ms100Task() {
  if(sendModePin == LOW && quietModePin == HIGH) {
    mcp2515.sendMessage(&canMsg6);  // 0x286
    mcp2515.sendMessage(&canMsg7);  // 0x377
    mcp2515.sendMessage(&canMsg8);  // 0x389
    mcp2515.sendMessage(&canMsg9);  // 0x38A
    mcp2515.sendMessage(&canMsg10); // 0x398     
    ms100count++;
    if(ms100count > 10) {
        ms100count = 0;
        Serial.println("Ms100 tasks running");
      }
  }
}

void ms500Task() {
}
void ms1000Task() {
  if(sendModePin == LOW && quietModePin == HIGH) {
    if(LEDstate == 0){
      digitalWrite(LEDpin, HIGH);
      LEDstate = 1;
    } else {
      digitalWrite(LEDpin, LOW);
      LEDstate = 0;      
    }
    mcp2515.sendMessage(&canMsg1);
    mcp2515.sendMessage(&canMsg2);
    mcp2515.sendMessage(&canMsg3);
    mcp2515.sendMessage(&canMsg4); 
    Serial.println("Ms1000 tasks running");
  }
}
