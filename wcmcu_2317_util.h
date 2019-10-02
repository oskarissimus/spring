/*
 Example 41.1 - Microchip MCP23017 with Arduino
*/
// pins 15~17 to GND, I2C bus address is 0x20


//A4 (SDA), A5 (SCL)
//- diody do gpio, + do 5V, LOW włącza diodę


#include "Wire.h"


#define BUS_INPUT  0
#define BUS_OUTPUT 1

#define BUS_A 0
#define BUS_B 1

#define I2C_ADDR 0x20




void set_BUS_mode (byte bus, byte mode) {
  byte IODIR_REG = 0;
  if (bus == BUS_A) IODIR_REG = 0x00;
  else if (bus == BUS_B) IODIR_REG = 0x01;

  if (mode == BUS_OUTPUT) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(IODIR_REG); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
  }
  else if (mode == BUS_INPUT) {
    ;//nothing to do, INPUT is default
  }

}

void BUS_write (byte bus, byte pin, byte val) {
  byte BUS_ADDR = 0;
  if (bus == BUS_A) BUS_ADDR = 0x12;
  else if (bus == BUS_B) BUS_ADDR = 0x13;
  
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUS_ADDR); // GPIOA
  Wire.write(val << pin); // port A
  Wire.endTransmission();
}

bool BUS_read (byte bus, byte pin) {
  byte BUS_ADDR = 0;
  if (bus == BUS_A) BUS_ADDR = 0x12;
  else if (bus == BUS_B) BUS_ADDR = 0x13;
  
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUS_ADDR);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR, 1); // request one byte of data from MCP20317
  byte inputs=Wire.read();
  //Serial.println(inputs,BIN);
  return ( inputs & ( 0x00000001 << pin )  ? true : false );
}
/*
void setup()
{
  Serial.begin(9600);
  Wire.begin(); // wake up I2C bus
  set_BUS_mode (BUS_A, BUS_OUTPUT);
  set_BUS_mode (BUS_B, BUS_INPUT);
}



void loop()
{
  bool isPushed = BUS_read (BUS_B,4);
  Serial.println(isPushed);
  BUS_write(BUS_A,1,isPushed);
}*/
