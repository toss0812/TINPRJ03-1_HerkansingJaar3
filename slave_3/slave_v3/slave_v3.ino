#include <Wire.h>
#include <Arduino.h>
// #include <Stings.h>

// General things
#define pin_reed 3
#define pin_story_led 9
#define pin_door_led 13
boolean state_mag = false;


// button related 
#define b_up 5
#define b_up_led 6
boolean b_up_state = false;
#define b_dn 7
#define b_dn_led 8
boolean b_dn_state = false;


// movement related
boolean mov_up = false;
boolean mov_dn = false; 
boolean mov_onfloor = false;
int mov_pos = 0;


// shift register related
#define sr_latch 12
#define sr_clock 10
#define sr_data 11
int sr_display_array[9] = {3, 159, 37, 13, 153, 73, 65, 31, 1};


// I2C related
byte i2c_sm_array[3] = {0, 0, 0};
const int i2c_adress = 8;


//================================================== SETUP
//
void setup() {
    Serial.begin(9600);

    Wire.begin(i2c_adress);
    Wire.onRequest(I2C_OnRequest);
    Wire.onReceive(I2C_OnReceive);
}


//================================================== MAIN LOOP
//
void loop() {
    // check buttons
    state_mag = digitalRead(pin_reed);
    b_up_state = digitalRead(b_up);
    b_dn_state = digitalRead(b_dn);

    // change movement state based on button states
    if (b_up_state == true) {
        mov_up = true;
        mov_dn = false;
    }

    if (b_dn_state == true) {
        mov_up = false;
        mov_dn = true;
    }

    if (state_mag == true) {
        mov_onfloor = true;
        mov_up = false;
        mov_dn = false;
    }

    // change led states based on movement states
    if (mov_up == true) {
        digitalWrite(b_up_led, HIGH);
    } else {
        digitalWrite(b_up_led, LOW);
    }

    if (mov_dn == true) {
        digitalWrite(b_dn_led, HIGH);
    } else {
        digitalWrite(b_dn_led, LOW);
    }

    if (mov_onfloor == true) {
        digitalWrite(pin_story_led, HIGH);

        if (mov_up == true || mov_dn == true) {
            digitalWrite(pin_door_led, HIGH);
        }
    } else {
        digitalWrite(pin_story_led, LOW);
        digitalWrite(pin_door_led, LOW);
    }

    SR_write(mov_pos);
}


//================================================== Custom Functions
//

//======================================== Shift Register Write
void SR_write(int data) {
    digitalWrite(sr_latch, HIGH);
    shiftOut(sr_data, sr_clock, MSBFIRST, sr_display_array[data]);
    digitalWrite(sr_latch, LOW);
}


//======================================== On Request
void I2C_OnRequest() {
    i2c_sm_array[0] = mov_up;
    i2c_sm_array[1] = mov_dn;
    i2c_sm_array[2] = mov_onfloor;

    for (int i = 0; i < 3; i++) {
        Wire.write(i2c_sm_array[i]);
    }
}


//======================================== Master on Receive
//
void I2C_OnReceive() {
    while (Wire.available()) {
        mov_pos = Wire.read();
    }
}
