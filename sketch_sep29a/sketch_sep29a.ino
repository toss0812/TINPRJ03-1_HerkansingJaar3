/*####
# Project 1: Lift
# Tom Dingenouts

######*/

#define pin 2



void setup() {
    // put your setup code here, to run once:
    pinMode(pin, OUTPUT);

}

void loop() {
    // put your main code here, to run repeatedly:
    digitalWrite(pin,HIGH);
    delay(1000);
    digitalWrite(pin,LOW);
    delay(1000);
}

void shiftOut(int data) {

}
