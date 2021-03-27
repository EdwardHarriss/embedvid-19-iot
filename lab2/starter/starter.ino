#include <U8g2lib.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>
#include <math.h>
#include <string.h>
#include <vector>


class knob {
  private:
    int8_t knobposition;
    int8_t knobpreviousvalue;
    int8_t upper_limit;
    int8_t lower_limit;
    int8_t prevrot = 0;
    bool buttonState;
    bool toggleMode; //sets it to push/release default
    bool prevButtonPress; //to stop value from toggling non-stop when button is held down a little too long
  public:
    knob() {
      knobposition = 0;
      upper_limit = 16;
      lower_limit = -16;
      buttonState=0;
      toggleMode = 0;
    }
    bool get_buttonState(){
      return buttonState;
    }
    void setToggle(bool setter){ //called in the setup() function, sets buttons to toggle rather than push/release
      if (setter==1){
        toggleMode = 1; //sets it to toggle rather than push/release
      }
      if (setter ==0){
        toggleMode = 0;
      }
    }
    void update_buttonState(bool currButtonVal){
      bool pressed = !currButtonVal; //button values are normally 1 while not pressed, 0 while pressed, so this is just flipping it
      //toggleMode = 1; //for testing
      if (toggleMode ==1){
        if (pressed==1){
          if (prevButtonPress!= 1){ //stops value from non-stop toggling when held down
             buttonState= !buttonState;
          }
        }
        prevButtonPress = pressed;
      }
      else if (toggleMode == 0){
        buttonState = pressed; //if not toggle mode set, button value = whether it is currently pressed or not
      }
    }

    int8_t get_knob_position() {
      return knobposition;
    }
    void reset_knob_position(){
      knobposition =0;
      knobpreviousvalue = 0;
    }
    uint8_t get_previous_value() {
      return knobpreviousvalue;
    }
    void set_previous_position(uint8_t new_prev) {
      knobpreviousvalue = new_prev;
    }
    void set_upper_limit(int8_t newupper){
      upper_limit = newupper;
    }
    void set_lower_limit(int8_t newlower){
      lower_limit = newlower;
    }
    void knobdecoder(uint8_t localCurrentKnob) {
      int8_t rotation = 0;
      if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x1)) {
        rotation = 1;
        prevrot = 1;
      }
      else if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x2)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x0) && (localCurrentKnob == 0x3)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x0)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x1)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x1) && (localCurrentKnob == 0x3)) {
        rotation = 1;
        prevrot = 1;
      }
      else if ((knobpreviousvalue == 0x2) && (localCurrentKnob== 0x0)) {
        rotation = 1;
        prevrot = 1;
      }
      /*else if ((knobpreviousvalue == 0x2) && (localCurrentKnob == 0x1)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x2) && (localCurrentKnob == 0x3)) {
        rotation = -1;
        prevrot = -1;
      }
      /*else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x0)) {
        rotation = 2*prevrot;
      }*/
      else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x1)) {
        rotation = -1;
        prevrot = -1;
      }
      else if ((knobpreviousvalue == 0x3) && (localCurrentKnob == 0x2)) {
        rotation = 1;
        prevrot = 1;
      }
      if ((knobposition >= upper_limit) && (rotation == 1)) {
        knobposition = upper_limit;
        return;
      }
      if ((knobposition <= lower_limit) && (rotation == -1)) {
        knobposition = lower_limit;
        return;
      }
      knobposition += rotation;
    }
};

class LFO {//LOW FREQUENCY OSCILLATOR CLASS 
  //there are 2 of these objects implemented: one of these for controlling the incrementing/decrementing counter that is added to the output frequency for vibrato
  //and one for the counter added to amplitude for tremolo
  private:
    int max_val; //counter ranges from 0 to max_val when incrementing/decrementing
    int counter;
    int counterprev; //helps determine when the counter is increasing vs decreasing, amd whether to increment or decrement the counter value next
    int counter_incr; //integer that determines the value to add/subtract from counter when increasing/decreasing - essentially sets the speed for the fluctuation of the LFO
    
  public:
    LFO() {//default values for fluctuation
      max_val = 100;
      counter = 0;
      counterprev = 0;
      counter_incr = 1;
    }
    void set_max(int newsetmax){ //function only ever called from setup() 
      max_val = newsetmax; 
    }
    void change_max(int maxvalchange){ //max_val is only ever changed via adding or subtracting from the max value (by knob_1)
      int new_max = max_val + maxvalchange;
      if ((new_max >=0)&&(new_max<10000)){ //puts upper limit on fluctuation range
         max_val = new_max;
      }
    }
    void change_counterIncr(int incr_change){//changes speed at which counter counts up and down (therfore speed of fluctuation in freq)
      int new_counter_incr = counter_incr + incr_change;
      if ((!(new_counter_incr > max_val/4))&&(!(new_counter_incr<=0))){ //cant have the increment too large or small otherwise it doesn't work
        counter_incr = new_counter_incr;
      }
      
    }
    void reset_counter(){//used when the joystick button is pressed - resets vibrato settings
      counter = 0;
      counterprev =0;
      counter_incr = 1;
      max_val = 100;
    }
    int get_counter(){
      return counter;
    }
    int get_incr(){
      return counter_incr;
    }
    void update_counter(){ //increments/decrements counter
      if ((counter <= 0)||((counter>counterprev)&&(counter<max_val))){
        counterprev = counter; 
        counter+=counter_incr;
     }
      else if((counter >= max_val)||(counter < counterprev)){
        counterprev = counter;
        counter-=counter_incr;
     }
    }
    
    
};
//Pin definitions

//Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

//Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

//Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

//Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

//Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to set outputs via matrix
SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t keyPressedVolMutex;
SemaphoreHandle_t knobsMutex;
SemaphoreHandle_t octaveMutex;
SemaphoreHandle_t modesMutex;

QueueHandle_t msgOutQ;


void setOutMuxBit(const uint8_t bitIdx, const bool value) {
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, bitIdx & 0x01);
  digitalWrite(RA1_PIN, bitIdx & 0x02);
  digitalWrite(RA2_PIN, bitIdx & 0x04);
  digitalWrite(OUT_PIN, value);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(REN_PIN, LOW);
}

knob knob_0;
knob knob_1;
knob knob_2;
knob knob_3;

LFO lfoVib; //fully customizable
LFO lfoTrem; //no options, just fixed variation of volume

void setup() {
  // put your setup code here, to run once:

  //Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply
  //mycode

  keyArrayMutex = xSemaphoreCreateMutex();
  keyPressedVolMutex = xSemaphoreCreateMutex();
  knobsMutex = xSemaphoreCreateMutex();
  octaveMutex = xSemaphoreCreateMutex();
  modesMutex = xSemaphoreCreateMutex();

  //set limits and button toggle rules for knobs:
  knob_2.set_lower_limit(-8);
  knob_2.set_upper_limit(8);
  knob_0.setToggle(1); //VIBRATO MODE TOGGLES ON BUTTON 0
  knob_0.set_upper_limit(16);
  knob_0.set_lower_limit(-16);
  knob_2.setToggle(1); //TREMOLO MODE TOGGLES ON BUTTON 2
  knob_3.set_lower_limit(0); //setting min volume

  lfoTrem.set_max(50);//sets fixed range for tremelo mode
  
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(
    scanKeysTask, /* Function that implements the task */
    "scanKeys", /* Text name for the task */
    64, /* Stack size in words, not bytes */
    NULL, /* Parameter passed into the task */
    2, /* Task priority */
    &scanKeysHandle ); /* Pointer to store the task handle */

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
    displayUpdateTask,
    "displayUpdate",
    32,
    NULL,
    1,
    &displayUpdateHandle );

  TaskHandle_t msgOutHandle = NULL;
  xTaskCreate(
    msgOutTask,
    "msgOut",
    32,
    NULL,
    1,
    &msgOutHandle );

  TaskHandle_t msgInHandle = NULL;
  xTaskCreate(
    msgInTask,
    "msgIn",
    32,
    NULL,
    4,
    &msgInHandle );

    TaskHandle_t LFOTaskHandle = NULL;
  xTaskCreate(
    LFOTask,
    "LFOtask",
    32,
    NULL,
    4,
    &LFOTaskHandle );

    TaskHandle_t SustainCounterTaskHandle = NULL;
  xTaskCreate(
    SustainCounterTask,
    "SustainCounterTask",
    8,
    NULL,
    4,
    &SustainCounterTaskHandle );

  msgOutQ = xQueueCreate( 8, 4 );

  //Initialise UART
  Serial.begin(115200);
  Serial.println("Hello World");

  vTaskStartScheduler();

}

//MY CODE BEGINS HERE:
const uint32_t fs = 22000; //sampling rate
const double ones = pow(2, 32);
const uint32_t stepSizes [] = {calcPhaseStep(261.63), calcPhaseStep(277.18), calcPhaseStep(293.66), calcPhaseStep(311.13), calcPhaseStep(329.63), calcPhaseStep(349.23), calcPhaseStep(369.99), calcPhaseStep(392.00), calcPhaseStep(415.30), calcPhaseStep(440.00), calcPhaseStep(466.16), calcPhaseStep(493.88)};
volatile uint32_t currentStepSize;

volatile uint8_t keyArray[7];
volatile int32_t joyValues[3]; //{x, y, joystick pressed}

//modal functionalities, updated by updateKeyboardRules function
volatile bool receiveMode;
volatile bool vibratoMode;
volatile bool tremoloMode;
volatile bool sustainMode;
volatile int sustainCounter;

volatile int8_t octave; //distinguishing between current set octave and received octave from message
volatile int8_t octaveOwn; //this is the octave being played on the keys (set by knob_0)
volatile int8_t octaveRec; //this is the received octave from a message 

volatile char noteMessage[] = "   ";
std::string keysPressedVol = "";
const char intToHex[] = "0123456789ABCDEF";

int32_t calcPhaseStep(int32_t freq) { 
  return round((freq * ones) / fs);
}

void setOctave(){ //sets the octave depending on send or receive mode
  if (receiveMode == 0){ // send mode - uses own set octave
    __atomic_store_n(&octave, octaveOwn, __ATOMIC_RELAXED);
  }
  else{
    __atomic_store_n(&octave, octaveRec, __ATOMIC_RELAXED); //receive mode - uses received octave
  }
}

void readJoy(){//reads joystick x and y values and places them in the global variables in joyValues array
  int joyx = analogRead(JOYX_PIN);
  int joyy = analogRead(JOYY_PIN);
  //inputs are inverted
  int xMap = -1*(joyx - 545); //normally left numbers are positive, right numbers are negative so that inverts that
  int yMap = -1*(joyy - 440); //same concept as above 
  joyValues[0] = xMap/10; //makes the read values range from -38 -> 36 (left to right) rather than 10* that
  joyValues[1] = yMap/10; //ranges from -40 -> 34 (bottom to top) ^ same idea as above

  //those last 2 lines may need a semaphore to protect? maybe? or atomic store?
  
  //joystick button press is updated in checkKeyPresses, hence is protected by keyArrayMutex
}

uint8_t readCols(){
  int c0 = digitalRead(C0_PIN);
  int c1 = digitalRead(C1_PIN) << 1;
  int c2 = digitalRead(C2_PIN) << 2;
  int c3 = digitalRead(C3_PIN) << 3;
  uint8_t cols = c3+c2+c1+c0;
  return cols;
}

void setRow(uint8_t rowIdx) {
  digitalWrite(REN_PIN, 0); //disable row select (prevents glitches while address pins are changed)
  digitalWrite(RA0_PIN, rowIdx&1);
  digitalWrite(RA1_PIN, rowIdx&2);
  digitalWrite(RA2_PIN, rowIdx&4);
  digitalWrite(REN_PIN, 1); //enable row select
}

void updateKeyboardRules(bool b0, bool b1, bool b2, bool b3, bool bjoy){//update modes like vibrato, tremolo, FUTURE:chorus,etc
  __atomic_store_n(&vibratoMode, b0, __ATOMIC_RELAXED);
  __atomic_store_n(&tremoloMode, b2, __ATOMIC_RELAXED);
  __atomic_store_n(&sustainMode, b3, __ATOMIC_RELAXED);
  
  
  if (b3==0) {
    sustainCounter = 0;
  }
  
  
}

uint32_t checkKeyPress(uint16_t keyarray, uint8_t k3, uint8_t k4, uint8_t k5,uint8_t k6) {
  std::string keysPressed = "";
  uint32_t stepSizeReturn = currentStepSize; //default if none of the if conditions are met
  
  //get knob rotations
  //first get knobs AB values from the keyArray values passed into this function
  uint8_t localCurrentKnob_0 = k4 >> 2;
  uint8_t localCurrentKnob_1 = k4 & 0b11;
  uint8_t localCurrentKnob_2 = k3 >> 2;
  uint8_t localCurrentKnob_3 = k3 & 0b11;
  
  //then update rotations on the knob classes
  xSemaphoreTake(knobsMutex, portMAX_DELAY);
  knob_0.knobdecoder(localCurrentKnob_0);
  knob_0.set_previous_position(localCurrentKnob_0);
  knob_1.knobdecoder(localCurrentKnob_1);
  knob_1.set_previous_position(localCurrentKnob_1);
  knob_2.knobdecoder(localCurrentKnob_2);
  knob_2.set_previous_position(localCurrentKnob_2);
  knob_3.knobdecoder(localCurrentKnob_3);
  knob_3.set_previous_position(localCurrentKnob_3);
  xSemaphoreGive(knobsMutex);
  
  
  octaveOwn = round((knob_2.get_knob_position()/2)+4); //to get set keyboard octave, divide knob0 position by 2 and add 4
  

  //check button positions
  uint8_t button_0 = k6 & 0b1;
  uint8_t button_1 = k6 & 0b10;
  uint8_t button_2 = k5 & 0b1;
  uint8_t button_3 = k5 & 0b10;
  knob_0.update_buttonState(button_0);
  knob_1.update_buttonState(button_1);
  knob_2.update_buttonState(button_2);
  knob_3.update_buttonState(button_3);
  //check joystick button
  uint8_t button_joy = (k5 & 0b100) >> 2; //get button press value for joystick
  joyValues[2] = !button_joy; //sets the correct button value for joystick (is usually 1 when not pressed and 0 when pressed, this inverts that)

  //distinguish which octave to use, either knob-based one or received-message-based one
  xSemaphoreTake(octaveMutex, portMAX_DELAY);
  setOctave(); // sets either local octave or octave of received message
  char o [1];
  itoa(octave, o, 16);
  xSemaphoreGive(octaveMutex);

  //getting pressed keys
  //NEED TO ADD CASES FOR MULTIPLE KEYS
  if (receiveMode ==0){
    bool isKeyPressed;
  switch(keyarray){
     case 0xFFF:
       if (!sustainMode){ //SUSTAIN MODE IMPLEMENTATION - only udates note with 0 if sustain mode is not on
          stepSizeReturn = 0;
       }
       else{
          stepSizeReturn = currentStepSize;
       }
       noteMessage[0] = 'R';
       noteMessage[1] = noteMessage[1];
       noteMessage[2] = noteMessage[2];
       isKeyPressed = 0;
      break;
    case 0xEFF:
      keysPressed += 'C';
      stepSizeReturn = stepSizes[0];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[0];
      isKeyPressed = 1;
      break;
    case 0xDFF:
      keysPressed += 'C';
      keysPressed += '#';
      stepSizeReturn = stepSizes[1];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[1];
      isKeyPressed = 1;
      break;
    case 0xBFF:
      keysPressed += 'D';
      stepSizeReturn = stepSizes[2];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[2];
      isKeyPressed = 1;
      break;
    case 0x7FF:
      keysPressed += 'E';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[3];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[3];
      isKeyPressed = 1;
      break;
    case 0xFEF:
      keysPressed += 'E';
      stepSizeReturn = stepSizes[4];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[4];
      isKeyPressed = 1;
      break;
    case 0xFDF:
      keysPressed += 'F';
      stepSizeReturn = stepSizes[5];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[5];
      isKeyPressed = 1;
      break;
    case 0xFBF:
      keysPressed += 'F';
      keysPressed += '#';
      stepSizeReturn = stepSizes[6];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[6];
      isKeyPressed = 1;
      break;
    case 0xF7F:
      keysPressed += 'G';
      stepSizeReturn = stepSizes[7];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[7];
      isKeyPressed = 1;
      break;
    case 0xFFE:
      keysPressed += 'G';
      keysPressed += '#';
      stepSizeReturn = stepSizes[8];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[8];
      isKeyPressed = 1;
      break;
    case 0xFFD:
      keysPressed += 'A';
      stepSizeReturn = stepSizes[9];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[9];
      isKeyPressed = 1;
      break;
    case 0xFFB:
      keysPressed += 'B';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[10];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[10];
      isKeyPressed = 1;
      break;
    case 0xFF7:
      keysPressed += 'B';
      stepSizeReturn = stepSizes[11];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[11];
      isKeyPressed = 1;
      break;
    default:
      stepSizeReturn = stepSizeReturn;
      noteMessage[0] = noteMessage[0];
      noteMessage[1] = noteMessage[1];
      noteMessage[2] = noteMessage[2];
      //isKeyPressed = 0;
      break;

  }
  if (isKeyPressed==1){
        sustainCounter = 6;
      }

  xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);
  keysPressedVol = keysPressed;
  xSemaphoreGive(keyPressedVolMutex);
  
  }
  
  return stepSizeReturn;
 
  
}


//int n =0;

void sampleISR() {

  static uint32_t phaseAcc = 0;
  //setting local variables
  int8_t locknobsrot[4];
  locknobsrot[0] = knob_0.get_knob_position();
  locknobsrot[1] = knob_1.get_knob_position();
  locknobsrot[2] = knob_2.get_knob_position();
  locknobsrot[3] = knob_3.get_knob_position();
  uint32_t loccurrentStepSize = currentStepSize;
  int32_t locjoyX = joyValues[0];
  int8_t locOctave = octave;
  bool locVibMode = vibratoMode;
  bool locTremMode = tremoloMode; 
  int locVibCounter = lfoVib.get_counter();
  int locTremCounter = lfoTrem.get_counter();
  
  
  //adjusts frequency by counter gotten from vibrato lfo 
  if (locVibMode ==1){
    if (loccurrentStepSize !=0){//stops clicking when no note is pressed and vibrato is on
      loccurrentStepSize+= locVibCounter*100000; //100000 could be a parameter we change to get a wider/smaller range - shouldn't need to though as this can be done by protected variables in LFO class
    }
  }
  
  
  //OCTAVES IMPLEMENTED ON KNOB 0 | also implements changing of octave based on incoming message
  //shifts octave up or down based on if octave is above or below 4 (the default octave)
  if (locOctave>=4){
    loccurrentStepSize = loccurrentStepSize << (uint32_t) (locOctave-4);
  }
  else{
    loccurrentStepSize = loccurrentStepSize >> (uint32_t) (-1*(locOctave-4));
  }

  //NOTE DISTORTION "WHAMMY BAR" ON X-AXIS JOYSTICK
  uint8_t distortResolution = 5; //how many frequency levels within the distortion there are (higher val -> fewer levels)
  uint8_t distortRange = 4; //how far joystick distorts the note
  double freqAdjust = pow(2,1/12) * (locjoyX/distortResolution) * distortRange ; //frequency change between current note and next/prev 2 notes
  if (loccurrentStepSize!=0){ //gets rid of clicking when using joystick without pressing key
    loccurrentStepSize += calcPhaseStep(freqAdjust);
  }
  
  phaseAcc += loccurrentStepSize;
  uint8_t outValue;

  int volAdjust = 8-locknobsrot[3]/2;
  //implementation of tremolo adjusting volume from counter gotten from tremolo lfo
  if (locTremMode ==1){
    if(loccurrentStepSize!=0){
     volAdjust += (locTremCounter/25); 
    }
  }

  //writes phase step to output pin
  outValue = (phaseAcc >> 24)>> volAdjust;
  int sustainAtten =  6 -(int)round(sustainCounter);
  if (sustainMode&&(currentStepSize!=0)){
    outValue = outValue >> sustainAtten;
  }
  analogWrite(OUTR_PIN, outValue);
 
  
}

void msgOutTask(void *pvParameters) {
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  char outMsg[4];
    while (1) {
     vTaskDelayUntil(&xLastWakeTime, xFrequency);
     xQueueReceive(msgOutQ, outMsg, portMAX_DELAY);
     Serial.println(outMsg);
   }
  
}

void msgInTask(void *pvParameters) {
  const TickType_t xFrequency = 5 / portTICK_PERIOD_MS; //initiation interval of the task (converted from time in ticks to 5ms)
  TickType_t xLastWakeTime = xTaskGetTickCount(); //stores time of last initiation
  char inMsg[] = "xxx";
  uint8_t placement = 0;
  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    while (Serial.available() > 0) {
      char message_in = Serial.read();
      if (message_in != '\n') {
        inMsg[placement++] = message_in;
      }
      else {
        placement = 0;
        if (inMsg[0] == 'R') {
          __atomic_store_n(&receiveMode, 0, __ATOMIC_RELAXED);
          __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
          
        }
        else if (inMsg[0] == 'P') {
          __atomic_store_n(&receiveMode, 1, __ATOMIC_RELAXED);
          std::string key_in = "";
          //write something here that changes the octave of incoming stuff.
          octaveRec = ((uint8_t) inMsg[1]) - 48; //takes value from ASCII int to int
          key_in.push_back(inMsg[2]);
          __atomic_store_n(&currentStepSize, stepSizes[std::stoi(key_in, 0, 16)], __ATOMIC_RELAXED);
        }
      }
    }
  }
}


//THREADS
void scanKeysTask(void * pvParameters) {
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; //initiation interval of the task (converted from time in ticks to 50ms)
  TickType_t xLastWakeTime = xTaskGetTickCount(); //stores time of last initiation
  //subsequent iterations this variable will be updated by the call to vTaskDelayUntil().

  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    // NEED TO REINCLUDE
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    for (int i = 0; i <= 6; i++) {
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols();
    }
    readJoy(); //get joystick x and y values - also protected by keyArrayMutex
    
    //getting notes:
    uint16_t k0 = keyArray[0] << 8;
    uint8_t k1 = keyArray[1] << 4;
    uint8_t k2 = keyArray[2];
    uint16_t keysConcatenated = k0+k1+k2;
    //checking key presses, knob rotations, etc performed inside checkKeyPress()
    uint32_t localCurrentStepSize = checkKeyPress(keysConcatenated, keyArray[3], keyArray[4], keyArray[5], keyArray[6]);

    //update required functional values based on all keyboard element changes detected

    //update vibrato LFO:
    lfoVib.change_counterIncr(round(joyValues[1]/30));//updates vibrato speed based on joystick Y input 
    lfoVib.change_max(knob_0.get_knob_position()/3);//updates vibrato range based on knob_1 rotational position
    if (joyValues[2] == 1){ //reset lfo by clicking joystick button if vibrato goes too far, and you don't want to spend all the time winding it back
      lfoVib.reset_counter();
      knob_0.reset_knob_position();
    }

    

    //update keyboard rules based on button values e.g. send/receive mode, vibrato, etc.
    updateKeyboardRules(knob_0.get_buttonState(), knob_1.get_buttonState(), knob_2.get_buttonState(), knob_3.get_buttonState(), joyValues[2]);
  
    xQueueSend( msgOutQ, (char*) noteMessage, portMAX_DELAY);
    
   
    __atomic_store_n(&currentStepSize, localCurrentStepSize, __ATOMIC_RELAXED); // ensures atomic operation
    xSemaphoreGive(keyArrayMutex);
  }
}

void displayUpdateTask(void * pvParameters) {
  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS; //initiation interval of the task (converted from time in ticks to 50ms)
  TickType_t xLastWakeTime = xTaskGetTickCount(); //stores time of last initiation

  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    u8g2.clearBuffer();
    
    //VIBRATO AND TREMOLO:
    u8g2.setFont(u8g2_font_ncenR08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Vib: ");
    u8g2.drawStr(40,10,"Trem: ");
    std::string vibrato;
    std::string tremolo;
    xSemaphoreTake(modesMutex, portMAX_DELAY);
    if (vibratoMode==1){
      u8g2.drawDisc(26, 6, 3, U8G2_DRAW_ALL);
    }
    else{
      u8g2.drawCircle(26, 6, 3, U8G2_DRAW_ALL);
    }
    if (tremoloMode==1){
      u8g2.drawDisc(75, 6, 3, U8G2_DRAW_ALL);
    }
    else{
      u8g2.drawCircle(75, 6, 3, U8G2_DRAW_ALL);
    }
    xSemaphoreGive(modesMutex);

    //VIBRATO OPTIONS - SPEED AND RANGE
     u8g2.setFont(u8g2_font_blipfest_07_tr);
     u8g2.drawStr(2,20,"Speed: ");
     u8g2.setCursor(25, 20);
     u8g2.print(lfoVib.get_incr(),DEC); //////////////////MAY NEED SEMAPHORE
     u8g2.drawStr(2,30,"Range: ");
     std::string rangeChange;
     int change = (int)round(knob_0.get_knob_position()/5); 
     switch(change){
        case 1:
          rangeChange = "+";
          break;
        case 2: 
          rangeChange = "++";
          break;
        case 3:
          rangeChange = "+++";
          break;
        case -1:
          rangeChange = "-";
          break;
        case -2: 
          rangeChange = "--";
          break;
        case -3:
          rangeChange = "---";
          break;
        case 0:
          rangeChange = "0";
          break;
     }
     u8g2.drawStr(25, 30, rangeChange.c_str());
       

    //VOLUME:
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(97,10,"Vol: ");
    u8g2.setCursor(117, 10);
    u8g2.print(knob_3.get_knob_position(),DEC);

    //OCTAVE:
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(97,20,"Oct: ");
    xSemaphoreTake(octaveMutex, portMAX_DELAY);
    std::string octaveString = std::to_string(octave);
    xSemaphoreGive(octaveMutex);
    u8g2.drawStr(117, 20, octaveString.c_str()); // write something to the internal memory
    
    
    
    //KEY PRESSED:
    xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);       
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(40,20,"Key: ");
    u8g2.drawStr(66, 20, keysPressedVol.c_str()); // write note
    xSemaphoreGive(keyPressedVolMutex);

    //SERIAL MESSAGE SENT (could take out)
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(40,30,"Msg: ");
    u8g2.setCursor(65, 30);
    u8g2.print((char*) noteMessage);

    //SEND VS RECEIVE MODE (if it is between receiving a Pxx and Rxx over serial)
    u8g2.drawStr(95,30,"S/R: ");
    std::string sendReceive;
    xSemaphoreTake(modesMutex, portMAX_DELAY);
    if (receiveMode==1){
      sendReceive = "R";
    }
    else{
      sendReceive = "S";
    }
    xSemaphoreGive(modesMutex);
    u8g2.drawStr(117, 30, sendReceive.c_str());

    
    
    //ROTATION LEFTOVERS (could replace message being sent with these)
    xSemaphoreTake(knobsMutex, portMAX_DELAY);
    u8g2.setCursor(80,20);       // set coordinates to print knob values
    u8g2.print(sustainCounter,DEC);
    xSemaphoreGive(knobsMutex);

    //BUTTON LEFTOVERS
    //u8g2.print(knob_2.get_buttonState());
    //u8g2.print(knob_3.get_buttonState());
   
    

    u8g2.sendBuffer();
    //Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
  }
}

void LFOTask(void *pvParameters) {
  const TickType_t xFrequency = 5 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
     vTaskDelayUntil(&xLastWakeTime, xFrequency);
     lfoVib.update_counter();
     lfoTrem.update_counter();
     //Serial.println(lfoVib.get_counter());
   } 
}

void SustainCounterTask(void *pvParameters) {
  const TickType_t xFrequency = 600 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
     vTaskDelayUntil(&xLastWakeTime, xFrequency);
     if ((sustainCounter>0)&&(sustainMode)){
      sustainCounter--;
     }
   } 
}

void loop() {}
