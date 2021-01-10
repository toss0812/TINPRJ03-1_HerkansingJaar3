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
int b_mag_state = 0;


// button related 
const int b_up = 5;
const int b_up_led = 6;
int b_up_state = 0;
const int b_dn = 7;
const int b_dn_led = 8;
int b_dn_state = 0;


// movement related
boolean mov_up = false;
boolean mov_dn = false; 
boolean mov_onfloor = false;
int mov_pos = 0;


// shift register related
const int sr_latch = 12;
const int sr_clock = 10;
const int sr_data = 11;
int sr_display_array[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111}; // dependant on if 7-segment is Common Annode / - Cathode


// pin arrays used in setup()
int pins_input[] = {pin_reed, b_up, b_dn};
int pins_output[] = {pin_story_led, pin_door_led, b_up_led, b_dn_led, sr_latch, sr_data, sr_clock};


// I2C related
byte i2c_sm_array[2] = {0, 0};
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
      pinMode(pins_input[i], INPUT); // input pins
    }
    for (int i = 0; i < 6; i++) {
      pinMode(pins_output[i], OUTPUT); // output pins
    }

    SR_write(mov_pos); // write cab postition to 7-segement

    // check buttons on init
    b_mag_state = digitalRead(pin_reed);
    b_up_state = digitalRead(b_up);
    b_dn_state = digitalRead(b_dn);
}


//================================================== MAIN LOOP
// > check button states and save to temps
// > change states according to temps
// > change lights according to states
// > write cabs' current position to 7-segement
// > print all info for debugging
void loop(){
    // check buttons
    b_mag_state = digitalRead(pin_reed);
    b_up_state = digitalRead(b_up);
    b_dn_state = digitalRead(b_dn);


    // change movement state based on button states
    if (b_up_state == HIGH) { // set movement state to up
        mov_up = true;
        mov_dn = false;
    }

    if (b_dn_state == HIGH) { // set movement state to down
        mov_up = false;
        mov_dn = true;
    }

    if (b_mag_state == HIGH) { // reset momevent state if the carrige passed the story
        mov_onfloor = true;
        mov_pos = 0; // quick override of global cab position, could otherwise not updaate in time

        if(mov_up == true || mov_dn == true){
            EL_DoorSequence(2500); // cheaty way of holding the door open 
            mov_up = false; // reset movement requests
            mov_dn = false; // |
        }
    } else {
        mov_onfloor = false; // reset local cab indicator
    }

    // change led states based on movement states
    if (mov_up == true) { // up light
        digitalWrite(b_up_led, HIGH);
    } 
    else {
        digitalWrite(b_up_led, LOW);
    }

    if (mov_dn == true) { // down light
        digitalWrite(b_dn_led, HIGH);
    } 
    else {
        digitalWrite(b_dn_led, LOW);
    }

    if (mov_onfloor == true) { // floor indicator light
        digitalWrite(pin_story_led, HIGH);

        while (mov_up == true || mov_dn == true) { // door indicator light
            digitalWrite(pin_door_led, HIGH);
        }
    } 
    else { // reset indiccator lights
        digitalWrite(pin_story_led, LOW); 
        digitalWrite(pin_door_led, LOW);
    }
    
    SR_write(mov_pos); // write current carrige postition to 7-segment display

    // Data prints
    Serial.print("move up: ");
    Serial.println(mov_up);
    Serial.print("move down: ");
    Serial.println(mov_dn);
    Serial.print("position: ");
    Serial.println(mov_pos);
    Serial.println("----------");
    delay(50);
}


//================================================== Custom Functions
//======================================== Shift Register Write
// > write number to 7-segment disply
void SR_write(int data) {
    digitalWrite(sr_latch, HIGH);
    shiftOut(sr_data, sr_clock, MSBFIRST, sr_display_array[data]); // data is the indicator used in an array
    digitalWrite(sr_latch, LOW);
}


//======================================== On Request
// > send movement data to master
void I2C_OnRequest() {
    if (mov_up == true || mov_dn == true) { // user requests movement?
        i2c_sm_array[0] = 1; 
    }

    if (mov_onfloor == true) { // cab on floor?
        i2c_sm_array[1] = 1; 
    } else {
        i2c_sm_array[1] = 0; // reset just to be sure
    }

    for (int i = 0; i < 2; i++) { 
        Wire.write(i2c_sm_array[i]); // send data to master
    }
}


//======================================== on Receive
// > receive data from master
void I2C_OnReceive(int a) {
    while (Wire.available()) {
        mov_pos = Wire.read(); // read postition data
    }
}

//======================================== Door open sequence
// Opens and holds the door open for x millis
void EL_DoorSequence(int time) {
    digitalWrite(pin_story_led, HIGH);
    digitalWrite(pin_door_led, HIGH);
    delay(time);
    digitalWrite(pin_door_led, LOW);
}
