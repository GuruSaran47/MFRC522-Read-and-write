#include"Read_Write_RFID.h"
void setup() {
  // put your setup code here, to run once:
  run_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  byte write_block[] = {
    0x10,0x11,0x12,0x13,
    0x10,0x11,0x12,0x13,
    0x10,0x11,0x12,0x13,
    0x10,0x11,0x12,0x13
  };
  // if(sizeof(write_block)==0x10)
  {
    // write_to_card(write_block,2,8,11);
    read_from_card(2,8,11);
  }
}
