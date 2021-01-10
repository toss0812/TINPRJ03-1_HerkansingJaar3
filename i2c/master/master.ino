// #######
// # tinprj03-1 : Herkansing jaar 3
// # Tom Dingenouts
// #######



#include <Wire.h> // library for I2C communitation
#include <ArduinoQueue.h> // from the interwebs: https://github.com/EinarArnason/ArduinoQueue , essentially an linked list implementation
#include <Keypad.h> // library to read a keypad used for cab inputs 
#include <Stepper.h> // library to control stepper motor used to move the cab


// Movement Related
int mov_lastSeen = 0; // index of where the cab was last seen. dependant on i2c data from slaves
int mov_target = -1; // index of the floor the cab is currently moving to
ArduinoQueue<int> queue(10); // queue used for movement planning


// Semi movement related
const int stepsPerRevolution = 50; // No. steps in a full revolution
Stepper stepper(stepsPerRevolution, 9, 10, 11, 12); // Initialize a steppermotor object and define the pins


// I2C Related
const int i2c_slaveCount = 3; // amount of slaves
const int i2c_slaveAdress[i2c_slaveCount] = {8,9,10}; // array of possible slave adresses
byte i2c_slaveInfo[i2c_slaveCount][2] = { // array of info from slaves, 1) was there a floor call? 2) is the cab at this floor
    {0,0},
    {0,0},
    {0,0},
};


// Keypad related
const byte rows = 4;
const byte cols = 3;
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'0', '0', '0'}
};
byte rowPins [rows] = {5,4,3,2}; // keypad pins
byte colPins [cols] = {8,7,6};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
char kp_current = '#';


//============================================================ SETUP
void setup(){
    Wire.begin(); // join I2C bus as master
    stepper.setSpeed(60); // set stepper rpm to 60
}


//============================================================ MAIN LOOP
void loop(){
    for(int i = 0; i < 2; i++){
        Wire.requestFrom(i2c_slaveAdress[i], 2); // request info from slave @adress, of 2 byte length 

        // temp values for later refrencing
        int temp1 = Wire.read();
        int temp2 = Wire.read();

        // if slave has new movement request, write in storage and add to queue
        if(i2c_slaveInfo[i][0] == 0 && temp1 == 1 ){
            i2c_slaveInfo[i][0] = 1;
            queue.enqueue(i);
        }

        // if carrige has passed slave update current position
        if(i2c_slaveInfo[i][1] == 0 && temp2 == 1){
            i2c_slaveInfo[i][1] = 1;
            mov_lastSeen = i;
        } else {
            i2c_slaveInfo[i][1] = 0;
        }
    }

    // write current carrige position to slaves
    for(int i = 0; i < 2; i++){
        Wire.begin(i2c_slaveAdress[i]);
        Wire.write(mov_lastSeen);
        Wire.endTransmission();
    }

    // look for cab calls and add to queue
    kp_current = keypad.getKey(); // get info from keypad

    if(kp_current != NO_KEY){ // if anything from keypad, add to queue 
        queue.enqueue(atoi(kp_current)); // convert char to int
    }

    // when there are new floor calls and the carrige is idle, get new destination, -1 as target is considered idle
    if(!queue.isEmpty() && mov_target == -1){
        mov_target = queue.dequeue();
    }

    move(mov_lastSeen, mov_target); // will keep moving until cab has reached target
}


//============================================================ MOVE
// > check if the cab is idle , just to be sure
// > check if cab has reached the destination
// > check what direction the cab has to move
// > move cab to given direction
void move(int current, int target){
    if (current == -1){ // cab is idle
        return;
    }
    
    if (current == target){ // cab has reached target
        mov_lastSeen = -1; // set cab to idle
        return;
    }
    
    if (current < target){ // cab has to go up
        stepper.step(stepsPerRevolution);
        blink(9);
        return;
    }

    if (current > target){ // cab has to go down
        stepper.step(-stepsPerRevolution);
        blink(11);
        return;
    }
}

void blink(int pin){
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
}