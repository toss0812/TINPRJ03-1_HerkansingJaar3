// #######
// # tinprj03-1 : Herkansing jaar 3
// # Tom Dingenouts
// #######

// #include <Arduino.h>
#include <Wire.h>
// #include <Stings.h>

// General things
const int pin_reed = 2;
const int pin_story_led = 3;
const int pin_door_led = 4;
boolean state_mag = false;


// button related 
const int b_up = 5;
const int b_up_led = 6;
boolean b_up_state = false;
const int b_dn = 7;
const int b_dn_led = 8;
boolean b_dn_state = false;


// movement related
boolean mov_up = false;
boolean mov_dn = false; 
boolean mov_onfloor = false;
int mov_pos = 0;


// shift register related
const int sr_latch = 12;
const int sr_clock = 10;
const int sr_data = 11;
int sr_display_array[9] = {3, 159, 37, 13, 153, 73, 65, 31, 1}; // dependant on if 7-segment is Common Annode / - Cathode

// pin arrays used in setup()
int pins_input[] = {pin_reed, b_up, b_dn};
int pins_output[] = {pin_story_led, pin_door_led, b_up_led, b_dn_led, sr_latch, sr_data, sr_clock};


// I2C related
byte i2c_sm_array[3] = {0, 0, 0};
const int i2c_adress = 8;


//================================================== SETUP
//
void setup() {
    Serial.begin(9600);

    // I2C init stuff
    Wire.begin(i2c_adress);
    Wire.onRequest(I2C_OnRequest);
    Wire.onReceive(I2C_OnReceive);
  
    // lazy way of pinMode initialisation
    for (int i = 0; i < 2; i++) {
      pinMode(pins_input[i], INPUT);
    }
  
    for (int i = 0; i < 6; i++) {
      pinMode(pins_output[i], OUTPUT);
    }
}


//================================================== MAIN LOOP
//
void loop( ){
    // check buttons
    state_mag = digitalRead(pin_reed);
    b_up_state = digitalRead(b_up);
    b_dn_state = digitalRead(b_dn);

    // change movement state based on button states
    if (b_up_state == true) { // set movement state to up
        mov_up = true;
        mov_dn = false;
    }

    if (b_dn_state == true) { // set movement state to down
        mov_up = false;
        mov_dn = true;
    }

    if (state_mag == true) { // reset momevent state if the carrige passed the story
        mov_onfloor = true;
        mov_up = false;
        mov_dn = false;
    }

    // change led states based on movement states
    if (mov_up == true) { // up light
        digitalWrite(b_up_led, HIGH);
    } else {
        digitalWrite(b_up_led, LOW);
    }

    if (mov_dn == true) { // down light
        digitalWrite(b_dn_led, HIGH);
    } else {
        digitalWrite(b_dn_led, LOW);
    }

    if (mov_onfloor == true) { // floor indicator light
        digitalWrite(pin_story_led, HIGH);

        if (mov_up == true || mov_dn == true) { // door indicator light
            digitalWrite(pin_door_led, HIGH);
        }
    } else { // reset indiccator lights
        digitalWrite(pin_story_led, LOW); 
        digitalWrite(pin_door_led, LOW);
    }

    SR_write(mov_pos); // write current carrige postition to 7-segment display
}


//================================================== Custom Functions
//

//======================================== Shift Register Write
// write number to 7-segment disply
void SR_write(int data) {
    digitalWrite(sr_latch, HIGH);
    shiftOut(sr_data, sr_clock, MSBFIRST, sr_display_array[data]); // data is the indicator used in an array
    digitalWrite(sr_latch, LOW);
}


//======================================== On Request
// send movement data to master
void I2C_OnRequest() {
    i2c_sm_array[0] = mov_up; // movement up state ?
    i2c_sm_array[1] = mov_dn; // movement down state ?
    i2c_sm_array[2] = mov_onfloor; // carrige on floor ?

    for (int i = 0; i < 3; i++) { 
        Wire.write(i2c_sm_array[i]); // send data to master
    }
}


//======================================== Master on Receive
// receive data from master
void I2C_OnReceive(int a) {
    while (Wire.available()) {
        mov_pos = Wire.read(); // read postition data
    }
}
