// #######
// # tinprj03-1 : Herkansing jaar 3
// # Tom Dingenouts
// #######

#include <Wire.h>


// Movement related 
int mov_pos = 0;

// I2C related
const int i2c_adress = 8; // change according to position in stack


//================================================== SETUP
//
void setup(){
    Wire.begin(i2c_adress);
    Wire.onRequest(I2C_OnRequest);
    Wire.onReceive(I2C_OnReceive);
}

//================================================== MAIN LOOP
//
void main(){
    
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