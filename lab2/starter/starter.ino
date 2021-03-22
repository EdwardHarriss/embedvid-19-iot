#include <U8g2lib.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>
#include <math.h>
#include <string.h>


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

    void setToggle(bool setter){
      if (setter==1){
        toggleMode = 1; //sets it to toggle rather than push/release
      }
      if (setter ==0){
        toggleMode = 0;
      }
    }
    void update_buttonState(bool currButtonVal){
      bool pressed = !currButtonVal; //button values are 1 while not pressed, 0 while pressed, so just flipping it
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

  //set limits and button toggle rules for knobs:
  knob_0.set_lower_limit(-8);
  knob_0.set_upper_limit(8);
  knob_3.setToggle(1); //SEND/RECEIVE MODE TOGGLES ON BUTTON 3
  
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
volatile int32_t joyValues[3]; //{x, y, pressed}

volatile bool receiveMode;
volatile int8_t octave; //distinguishing between current set octave and received octave from message
volatile int8_t octaveOwn;
volatile int8_t octaveRec;

volatile char noteMessage[] = "   ";
std::string keysPressedVol = "";
const char intToHex[] = "0123456789ABCDEF";

int32_t calcPhaseStep(int32_t freq) { //TODO: implement octaves
  return round((freq * ones) / fs);
}

void setOctave(){ //sets the octave dependi ng on send or receive mode
  if (receiveMode == 0){ // send mode - uses own set octave
    octave = octaveOwn;
  }
  else{
    octave = octaveRec; //receive mode - uses received octave
  }
}

void readJoy(){
  int joyx = analogRead(JOYX_PIN);
  int joyy = analogRead(JOYY_PIN);
  //inputs are inverted
  int xMap = -1*(joyx - 545); 
  int yMap = -1*(joyy - 440);
  joyValues[0] = xMap/10; //ranges from -38 -> 36 (left to right)
  joyValues[1] = yMap/10; //ranges from -40 -> 34 (bottom to top)
  //joystick press is updated in checkKeyPresses
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

void updateKeyboardRules(bool b0, bool b1, bool b2, bool b3, bool bjoy){//update modes like send/receive, ect
  receiveMode = b3;
}

uint32_t checkKeyPress(uint16_t keyarray, uint8_t k3, uint8_t k4, uint8_t k5,uint8_t k6) {
  std::string keysPressed = "";
  uint32_t stepSizeReturn = currentStepSize; //default if none of the if conditions are met
  
  //get knob rotations
  //first get knobs AB
  uint8_t localCurrentKnob_0 = k4 >> 2;
  uint8_t localCurrentKnob_1 = k4 & 0b11;
  uint8_t localCurrentKnob_2 = k3 >> 2;
  uint8_t localCurrentKnob_3 = k3 & 0b11;
  //then update rotations
  knob_0.knobdecoder(localCurrentKnob_0);
  knob_0.set_previous_position(localCurrentKnob_0);
  knob_1.knobdecoder(localCurrentKnob_1);
  knob_1.set_previous_position(localCurrentKnob_1);
  knob_2.knobdecoder(localCurrentKnob_2);
  knob_2.set_previous_position(localCurrentKnob_2);
  knob_3.knobdecoder(localCurrentKnob_3);
  knob_3.set_previous_position(localCurrentKnob_3);
  if (receiveMode ==0){
    octaveOwn = round((knob_0.get_knob_position()/2)+4); //to get set keyboard octave, divide knob0 position by 2 and add 4
  }
  

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
  uint8_t button_joy = (k5 & 0b100) >> 2;
  joyValues[2] = !button_joy;

  //update keyboard rules e.g. send/receive mode, vibrato, etc.
  updateKeyboardRules(knob_0.get_buttonState(), knob_1.get_buttonState(), knob_2.get_buttonState(), knob_3.get_buttonState(), joyValues[2]);
  
  //Serial.println(button_0);
  
  //octave = (octave/2) + 8;
  
  setOctave(); // sets either local octave or octave of received message
  char o [1];
  itoa(octave, o, 16);

  //getting pressed keys
  if (receiveMode ==0){
  switch(keyarray){
     case 0xFFF:
       stepSizeReturn = 0;
       noteMessage[0] = 'R';
       noteMessage[1] = noteMessage[1];
       noteMessage[2] = noteMessage[2];
      break;
    case 0xEFF:
      keysPressed += 'C';
      stepSizeReturn = stepSizes[0];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[0];
      break;
    case 0xDFF:
      keysPressed += 'C';
      keysPressed += '#';
      stepSizeReturn = stepSizes[1];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[1];
      break;
    case 0xBFF:
      keysPressed += 'D';
      stepSizeReturn = stepSizes[2];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[2];
      break;
    case 0x7FF:
      keysPressed += 'E';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[3];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[3];
      break;
    case 0xFEF:
      keysPressed += 'E';
      stepSizeReturn = stepSizes[4];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[4];
      break;
    case 0xFDF:
      keysPressed += 'F';
      stepSizeReturn = stepSizes[5];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[5];
      break;
    case 0xFBF:
      keysPressed += 'F';
      keysPressed += '#';
      stepSizeReturn = stepSizes[6];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[6];
      break;
    case 0xF7F:
      keysPressed += 'G';
      stepSizeReturn = stepSizes[7];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[7];
      break;
    case 0xFFE:
      keysPressed += 'G';
      keysPressed += '#';
      stepSizeReturn = stepSizes[8];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[8];
      break;
    case 0xFFD:
      keysPressed += 'A';
      stepSizeReturn = stepSizes[9];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[9];
      break;
    case 0xFFB:
      keysPressed += 'B';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[10];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[10];
      break;
    case 0xFF7:
      keysPressed += 'B';
      stepSizeReturn = stepSizes[11];
      noteMessage[0] = 'P';
      noteMessage[1] = o[0];
      noteMessage[2] = intToHex[11];
      break;
    default:
      stepSizeReturn = stepSizeReturn;
      noteMessage[0] = noteMessage[0];
      noteMessage[1] = noteMessage[1];
      noteMessage[2] = noteMessage[2];
      break;
  }
  

  xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);
  keysPressedVol = keysPressed;
  xSemaphoreGive(keyPressedVolMutex);
  
  }
  return stepSizeReturn;
}
int8_t knob0 = 0; ///THESE 4 ONLY FOR DEBUGGING KNOB VALUES
int8_t knob1 = 0; ///THESE 4 ONLY FOR DEBUGGING KNOB VALUES
int8_t knob2 = 0; ///THESE 4 ONLY FOR DEBUGGING KNOB VALUES
int8_t knob3 = 0; ///THESE 4 ONLY FOR DEBUGGING KNOB VALUES

int counter = 0;

void sampleISR() {
  int8_t knobsrot[4];
  knobsrot[0] = knob_0.get_knob_position();
  knobsrot[1] = knob_1.get_knob_position();
  knobsrot[2] = knob_2.get_knob_position();
  knobsrot[3] = knob_3.get_knob_position();
  knob0 = knobsrot[0]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob1 = knobsrot[1]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob2 = knobsrot[2]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob3 = knobsrot[3]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  
  static uint32_t phaseAcc = 0;
  uint32_t loccurrentStepSize = currentStepSize;

  setOctave(); // sets either local octave or octave of received message
  //OCTAVES IMPLEMENTED ON KNOB 0 | also implements changing of octave based on incoming message
  if (octave>=4){
    loccurrentStepSize = loccurrentStepSize << (uint32_t) (octave-4);
  }
  else{
    loccurrentStepSize = loccurrentStepSize >> (uint32_t) (-1*(octave-4));
  }

  //NOTE DISTORTION "WHAMMY BAR" ON X-AXIS JOYSTICK
  uint8_t distortResolution = 5; //how many frequency levels within the distortion there are (higher val -> fewer levels)
  uint8_t distortRange = 4; //how far joystick distorts the note

  double freqAdjust = pow(2,1/12) * (joyValues[0]/distortResolution) * distortRange ; //frequency change between current note and next/prev 2 notes
  if (loccurrentStepSize!=0){ //gets rid of clicking when using joystick without pressing key
    loccurrentStepSize += calcPhaseStep(freqAdjust);
  }
  
  phaseAcc += loccurrentStepSize;
  uint8_t outValue;
  
  /*uint8_t outValue2;

  //for (int i=0; i<2; i++){
   // outValues[i] = (phaseAccArr[i] >> 24) >> (8-knobsrot[3]/2);
  }*/
  
  outValue = (phaseAcc >> 24)>> (8 - knobsrot[3]/2);
  analogWrite(OUTR_PIN, outValue);
  //TYLER - trying to implement multiple freqs/varying freqs over time
  /*outValue2 = ((phaseAcc+10000000) >> 24)>> (8 - knobsrot[3]/2);
  
  if (counter ==0){
    analogWrite(OUTR_PIN, outValue);
    //Serial.println(phaseAcc);
    counter++;
  }
  else{
    analogWrite(OUTR_PIN, outValue2);
    //Serial.println("alternative");
    counter = 0;
  }
  
  //analogWrite(OUTL_PIN, outValue);


  
  //uint8_t outvaluevec[1] = {calcPhaseStep(261.63)};
  //, calcPhaseStep(277.18), calcPhaseStep(293.66), calcPhaseStep(311.13), calcPhaseStep(329.63), calcPhaseStep(349.23), calcPhaseStep(369.99), calcPhaseStep(392.00), calcPhaseStep(415.30), calcPhaseStep(440.00), calcPhaseStep(466.16), calcPhaseStep(493.88)};
  //for (int i=0; i<1; i++){
  //  outValue = (outvaluevec[i] >> 24) >> (8 - knobsrot[3]/2);
  //  analogWrite(OUTR_PIN, outValue);
  //}
  //outValue = calcPhaseStep(261.63) >> 24;
  
  //for (int j=0; j<2; j++){
  //analogWrite(OUTR_PIN, outValue);
  //}
  */
  
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
          __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
        }
        else if (inMsg[0] == 'P') {
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
    readJoy();
    
    //getting notes:
    uint16_t k0 = keyArray[0] << 8;
    uint8_t k1 = keyArray[1] << 4;
    uint8_t k2 = keyArray[2];
    uint16_t keysConcatenated = k0+k1+k2;
    //checking key presses, knob rotations, etc
    uint32_t localCurrentStepSize = checkKeyPress(keysConcatenated, keyArray[3], keyArray[4], keyArray[5], keyArray[6]);

    xQueueSend( msgOutQ, (char*) noteMessage, portMAX_DELAY);
    
    //uint8_t localCurrentKnob_3 = keyArray[3] & 0x3;
    //knob_3.knobdecoder(localCurrentKnob_3);
    //knob_3.set_previous_position(localCurrentKnob_3);
    __atomic_store_n(&currentStepSize, localCurrentStepSize, __ATOMIC_RELAXED); // ensures atomic operation
    xSemaphoreGive(keyArrayMutex);
  }
}

void displayUpdateTask(void * pvParameters) {
  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS; //initiation interval of the task (converted from time in ticks to 50ms)
  TickType_t xLastWakeTime = xTaskGetTickCount(); //stores time of last initiation

  while (1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    std::string myString = "";
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2, 10, keysPressedVol.c_str()); // write something to the internal memory

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    
    u8g2.setCursor(2, 30);      // set coordinates to print result
    u8g2.print(keyArray[0], HEX);
    u8g2.print(keyArray[1], HEX);
    u8g2.print(keyArray[2], HEX);
    xSemaphoreGive(keyArrayMutex);
    xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);


    std::string octaveString = std::to_string(octave);
    u8g2.drawStr(90, 10, octaveString.c_str()); // write something to the internal memory
    
    u8g2.setCursor(90,30);       // set coordinates to print knob values
    
    u8g2.print((int)round(knob0/2)+4,DEC);
    u8g2.print(knob1,DEC);
    u8g2.print(knob2,DEC);
    u8g2.print(knob3,DEC);

    u8g2.setCursor(70,20);
    u8g2.print(joyValues[2]);

    u8g2.setCursor(90,20); //print button values
    u8g2.print(knob_0.get_buttonState());
    u8g2.print(knob_1.get_buttonState());
    u8g2.print(knob_2.get_buttonState());
    std::string sendReceive;
    if (receiveMode==1){
      sendReceive = "R";
    }
    else{
      sendReceive = "S";
    }
    u8g2.drawStr(108, 20, sendReceive.c_str());
    //u8g2.print(sendReceive);

    u8g2.setCursor(64, 30);
    u8g2.print((char*) noteMessage);
    u8g2.sendBuffer();
    //Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    xSemaphoreGive(keyPressedVolMutex);
  }
}

void loop() {}
