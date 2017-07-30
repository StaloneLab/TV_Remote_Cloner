//============================================================//
//                                                            //
//  Desc:    Arduino Code to clone a TV remote command        //
//  Dev:     Titi_Alone                                       //
//  Date:    July 2017                                        //
//  Updated: July 2017                                        //
//  Note:    Somes changes to be done for TVs                 //
//                              others than Samsung           //
//                                                            //
//============================================================//

// Memory
#include <avr/pgmspace.h>
#include <EEPROM.h>
// IR lib
#include <IRremote.h>

// Pin constants
PROGMEM const int PIN_SEND      = 3; // Library default, don't change
PROGMEM const int SEND_RECV_PIN = 7; // ON/OFF inverter
PROGMEM const int PIN_RECEIVE   = 8; // Pin on which to receive IR commands (not required)
PROGMEM const int BUTTON_SEND   = 9; // Simple Pushbutton

// Define our emit / receive instances
IRsend emitter;
IRrecv receiver(PIN_RECEIVE);

// How much bits do the remote works with
PROGMEM const int REMOTE_BITS = 32;

unsigned long command;

void setup() {
  // Begin computer Serial communication
  Serial.begin(9600);

  pinMode(BUTTON_SEND,   INPUT); // Button to fire the IR command
  pinMode(PIN_RECEIVE,   INPUT); // Pin to receive the IR
  pinMode(SEND_RECV_PIN, INPUT); // LOW = send ; HIGH = receive and store

  // Get the command already stored in the EEPROM
  command = readLong(16);
}

void loop() {
  // Check for the send / receive pin
  boolean send_recv = digitalRead(SEND_RECV_PIN);
  
  if(send_recv == LOW) {
    // Check if we want to send something
    boolean state = digitalRead(BUTTON_SEND);
    
    if(state == HIGH) {
      Serial.write("Sending the IR command: ");
      Serial.println(command, HEX);

      // Send the IR command to a Samsung TV (change for other models)
      emitter.sendSAMSUNG(command, REMOTE_BITS);

      // Wait for the button to be depressed before starting a new loop
      while(state != LOW) {
        state = digitalRead(BUTTON_SEND);
        delay(100);
      }
    }
  } else {
    recvLoop(); // Enter another loop
  }

  // Allow to manually store the IR code via serial
  if(Serial.available()) {
    unsigned long manual = receiveLong();

    if(manual != 0) {
      Serial.write("Storing the value: ");
      Serial.println(manual, HEX);
  
      saveLong(16, manual);
      command = manual;
    }
  }
}

void recvLoop() {
  decode_results decoded;

  // Try to get a code
  receiver.enableIRIn();
  Serial.println("Waiting for a code...");
  
  while(1) {
    // When the code is received...
    if(receiver.decode(&decoded)) {
      Serial.write("Storing the value: ");
      Serial.println(decoded.value, HEX);

      // Store it and use it
      saveLong(16, decoded.value);
      command = decoded.value;

      // Return to main loop
      break;
    }
  }
}

// Convenience for writing a long to the EEPROM
void saveLong(int addr, unsigned long val) {
  unsigned char part[4];

  part[0] = val & 0x000000FF;
  part[1] = (val >> 8) & 0x000000FF;
  part[2] = (val >> 16) & 0x000000FF;
  part[3] = (val >> 24) & 0x000000FF;

  EEPROM.write(addr, part[0]);
  EEPROM.write(addr + 1, part[1]);
  EEPROM.write(addr + 2, part[2]);
  EEPROM.write(addr + 3, part[3]);
}

// Convenience for reading a long from the EEPROM
unsigned long readLong(int addr) {
  unsigned long val = 0;
  unsigned char part[4];

  part[0] = EEPROM.read(addr);
  part[1] = EEPROM.read(addr + 1);
  part[2] = EEPROM.read(addr + 2);
  part[3] = EEPROM.read(addr + 3);

  val = part[3];
  val = val << 8;
  val = val | part[2];
  val = val << 8;
  val = val | part[1];
  val = val << 8;
  val = val | part[0];

  return val;
}

// Convert an ASCII hexadecimal string to some value
unsigned char ASCIIToHex(unsigned char ascii) {
  if(ascii >= 48 && ascii <= 57) {
    return ascii - '0';
  } else if(ascii >= 65 && ascii <= 70) {
    return ascii - 'A' + 10;
  } else if(ascii >= 97 && ascii <= 102) {
    return ascii - 'a' + 10;
  }
}

// Receive a long from the Serial
unsigned long receiveLong() {
  unsigned long val = 0;
  unsigned char recv[8];
  
  Serial.readBytes(recv, REMOTE_BITS / 4);

  if(   recv[0] != 10 && recv[0] != 13
     && recv[1] != 10 && recv[1] != 13) {
    val = ASCIIToHex(recv[0]);
    val = val << 4;
    val = val | ASCIIToHex(recv[1]);
    val = val << 4;
    val = val | ASCIIToHex(recv[2]);
    val = val << 4;
    val = val | ASCIIToHex(recv[3]);
    val = val << 4;
    val = val | ASCIIToHex(recv[4]);
    val = val << 4;
    val = val | ASCIIToHex(recv[5]);
    val = val << 4;
    val = val | ASCIIToHex(recv[6]);
    val = val << 4;
    val = val | ASCIIToHex(recv[7]);
  }

  return val;
}
