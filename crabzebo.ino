#include <AFMotor.h>

#define L_O_PIN 3
#define L_C_PIN 2
#define R_O_PIN 1
#define R_C_PIN 0
#define C_PIN 5

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2
#define MAGNET_LEFT 3
#define MAGNET_RIGHT 4

#define REST 0
#define OPENING 1
#define CLOSING 2

#define OPEN 1
#define CLOSE 2

#define HALL_T 150
#define SWITCH_T 30

AF_DCMotor motorleft(MOTOR_LEFT , MOTOR12_2KHZ); // create motor #2, 1KHz pwm
AF_DCMotor motorright(MOTOR_RIGHT , MOTOR12_2KHZ); // create motor #2, 1KHz pwm
AF_DCMotor magleft(MAGNET_LEFT, MOTOR34_64KHZ); // create motor #2, 1KHz pwm
AF_DCMotor magright(MAGNET_RIGHT, MOTOR34_64KHZ); // create motor #2, 1KHz pwm

int val_hall_L_O=0;
int val_hall_L_C=0;
int val_hall_R_O=0;
int val_hall_R_C=0;
int val_O=0;
int val_C=0;
int direc = 0;

bool hall_L_O=0;
bool hall_L_C=0;
bool hall_R_O=0;
bool hall_R_C=0;
bool T_O=0;
bool T_C=0;
bool T_M=0;
bool T_S=0;

int state[]={0,0};
bool left_closing = false;
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  //Serial.println("Motor test!");
  //pinMode(A4, OUTPUT);
  //pinMode(A4, OUTPUT);
  //digitalWrite(A0,HIGH);
  //digitalWrite(A4,HIGH);
  //Serial.begin(9600); 
  //delay(10000);
  //digitalWrite(A4, HIGH);
  //digitalWrite(A4, HIGH);
  delay(10000);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, T_S);
  
  motorleft.setSpeed(255);     // set the speed to 200/255
  motorright.setSpeed(255);
  magleft.setSpeed(255);
  magright.setSpeed(255);

}
 
void loop() {
  
  read_sensors();
  state_INPUT_PULLUP();
  state_sensors();
  move_levers();  
  //Serial.println("STATE 1: ");
  //Serial.println(state[1]);
  //Serial.println("\t");

}

void read_sensors(){

  hall_L_O=0;
  hall_L_C=0;
  hall_R_O=0;
  hall_R_C=0;
  T_O=0;
  T_C=0;

  bool T_before = T_M;
  val_C = analogRead(C_PIN);
  if(val_C<SWITCH_T) T_M=1;
  else T_M=0;

  if(T_before!=T_M){
    if(T_M) T_C=1;
    else T_O=1;
  }
  
  if(T_O) Serial.println("Open Command");
  if(T_C) Serial.println("Close Command");
  val_hall_L_O = analogRead(L_O_PIN);     // read the INPUT_PULLUP pin
  val_hall_L_C = analogRead(L_C_PIN);     // read the INPUT_PULLUP p
  val_hall_R_O = analogRead(R_O_PIN);     // read the INPUT_PULLUP pin
  val_hall_R_C = analogRead(R_C_PIN);     // read the INPUT_PULLUP p

  if(val_hall_L_O<HALL_T) hall_L_O=1;
  if(val_hall_L_C<HALL_T) hall_L_C=1;
  if(val_hall_R_O<HALL_T) hall_R_O=1;
  if(val_hall_R_C<HALL_T) hall_R_C=1;
  //Serial.println(val_hall_L_C);
  //Serial.println("\t");
  /*Serial.println("SWITCH_O: ");
  Serial.println(val_O);
  Serial.println("\t");
  Serial.println("SWITCH_C: ");
  Serial.println(val_C);
  Serial.println("\t"); */
  Serial.println("HALL_L_O: ");
  Serial.println(val_hall_L_O);
  Serial.println("\t");
  Serial.println("HALL_L_C: ");
  Serial.println(val_hall_L_C);
  Serial.println("\t");
  Serial.println("HALL_R_O: ");
  Serial.println(val_hall_R_O);
  Serial.println("\t");
  Serial.println("HALL_R_C: ");
  Serial.println(val_hall_R_C);
  Serial.println("\t"); 

}

void rest(){
  
  motorleft.run(RELEASE);
  motorright.run(RELEASE);
  magleft.run(RELEASE);
  magright.run(RELEASE);
  
}

void state_INPUT_PULLUP(){
  switch(state[0])
  {
    //START MOTIONS
    case REST:
      if(T_O){
        state[0]=OPENING;
        state[1]=OPEN;
      }

      if(T_C){
        state[0]=OPENING;
        state[1]=CLOSE;       
      }
      break;
      
    //OPENING MOTION  
    case OPENING:
      if(T_O){
        state[0]=OPENING;
        state[1]=OPEN;
      }

      if(T_C){
        state[0]=OPENING;
        state[1]=CLOSE;       
      }
      break;

    //CLOSE MOTION  
    case CLOSING:
      if(T_O){
        state[0]=OPENING;
        state[1]=OPEN;
      }

      if(T_C){
        state[0]=CLOSING;
        state[1]=CLOSE;       
      }
      break; 
       
      
  }
}

void state_sensors(){
  switch(state[0])
  {
    //START MOTIONS
    case REST:
      break;
      
    //OPENING MOTION  
    case OPENING:
      if(hall_L_O && hall_R_O){
        state[0]=CLOSING;
        magleft.run(FORWARD);
        magright.run(FORWARD);
        delay(1300);
        if(state[1]==CLOSE) left_closing=true;
      }      
      break;

    //CLOSE MOTION  
    case CLOSING:
      if(hall_L_C && hall_R_C){
        
      
        if(state[1]==OPEN){
        Serial.println("BLIND OPENED");
        motorleft.run(RELEASE);
        motorright.run(RELEASE);
        T_S = !(T_S);
        digitalWrite(A4, T_S);
        delay(100);
        state[0]=REST;
        state[1]=REST;
        }

        if(state[1]==CLOSE){
        Serial.println("BLIND CLOSED");
        motorleft.run(RELEASE);
        motorright.run(RELEASE);
        T_S = !(T_S);
        digitalWrite(A4, T_S);
        delay(100);
        state[0]=REST;
        state[1]=REST;
        }
      }     
      break; 
             
  }
}

void move_levers(){

  if(state[0]==REST){
    magleft.run(RELEASE);
    magright.run(RELEASE);
    motorleft.run(RELEASE);
    motorright.run(RELEASE);
    return;
    
  }
  
    if(state[1]==OPEN){
    magleft.run(RELEASE);
    magright.run(RELEASE);   
  }

  if(state[0]==OPENING)
  {
    
    Serial.println("State Opening");
    if(hall_L_O) motorleft.run(RELEASE);
    else motorleft.run(FORWARD);
    
    if(hall_R_O) motorright.run(RELEASE);
    else motorright.run(FORWARD);
    
    if(hall_L_O && hall_R_O) state[0]==CLOSING;
  }

  if(state[0]==CLOSING)
  {
    Serial.println("State Closing");
    if(hall_L_C) {
      motorleft.run(RELEASE);
      left_closing = false;
    }
    else motorleft.run(BACKWARD);
    
    if(hall_R_C || left_closing) motorright.run(RELEASE);
    else motorright.run(BACKWARD);
    
  }

    if(state[0]==CLOSING && state[1]==CLOSE && left_closing)
  {

    magleft.run(RELEASE);
    //magright.run(RELEASE);
    delay(35);
    magleft.run(FORWARD);
    //magright.run(FORWARD);

  }

  else if(state[0]==CLOSING && state[1]==CLOSE && (!left_closing)){
    magright.run(RELEASE);
    //magright.run(RELEASE);
    delay(25);
    magright.run(FORWARD);
    delay(45);
    //magright.run(FORWARD);
  }

  else
  {
    magleft.run(RELEASE);
    magright.run(RELEASE);
    
  }
  
  
}


