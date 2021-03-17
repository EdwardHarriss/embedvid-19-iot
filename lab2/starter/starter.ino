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
  public:
    knob() {
      knobposition = 0;
      upper_limit = 16;
      lower_limit = -16;
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
      if ((knobposition > upper_limit) && (rotation == 1)) {
        knobposition = upper_limit;
        return;
      }
      if ((knobposition < lower_limit) && (rotation == -1)) {
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

  knob_0.set_lower_limit(-8);
  knob_0.set_upper_limit(8);

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

volatile int32_t joyValues[2]; //{x, y}

volatile char noteMessage[] = "   ";
std::string keysPressedVol = "";
const char intToHex[] = "0123456789ABCDEF";

int32_t calcPhaseStep(int32_t freq) { //TODO: implement octaves
  return round((freq * ones) / fs);
}

void readJoy(){
  int joyx = analogRead(JOYX_PIN);
  int joyy = analogRead(JOYY_PIN);
  //inputs are inverted
  int xMap = -1*(joyx - 545); 
  int yMap = -1*(joyy - 440);
  joyValues[0] = xMap/10; //ranges from -38 -> 36 (left to right)
  joyValues[1] = yMap/10; //ranges from -40 -> 34 (bottom to top)
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

uint32_t checkKeyPress(uint16_t keyarray, uint8_t k3, uint8_t k4) {
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
  int8_t octave = (knob_0.get_knob_position()/2)+4; //to get octave, divide knob0 position by 2 and add 4
  //octave = (octave/2) + 8;
  char o [1];
  itoa(octave, o, 16);
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
  
  
  return stepSizeReturn;
}
int8_t knob0 = 0; ///THESE 4 ONLY FOR DEBUGGING KNOB VALUES
int8_t knob1 = 0;
int8_t knob2 = 0;
int8_t knob3 = 0;

void sampleISR() {
  int8_t knobsrot[4];
  knobsrot[0] = knob_0.get_knob_position();
  //knobsrot[1] = knob_1.get_knob_position();
  //knobsrot[2] = knob_2.get_knob_position();
  knobsrot[3] = knob_3.get_knob_position();
  knob0 = knobsrot[0]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob1 = knobsrot[1]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob2 = knobsrot[2]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  knob3 = knobsrot[3]; ///                DEBUGINGGGGGGGGGGGGGGGGGGGGGGGGG
  
  static uint32_t phaseAcc = 0;
  uint32_t loccurrentStepSize = currentStepSize;
  
  //OCTAVES IMPLEMENTED ON KNOB 0
  if (knobsrot[0]>=0){
    loccurrentStepSize = loccurrentStepSize << (uint32_t) round((knobsrot[0])/2);
  }
  else{
    loccurrentStepSize = loccurrentStepSize >> (uint32_t) round(-1*(knobsrot[0])/2);
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
  
  outValue = (phaseAcc >> 24) >> (8 - knobsrot[3]/2);
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
          __atomic_store_n(&currentStepSize, currentStepSize, __ATOMIC_RELAXED);
        }
        else if (inMsg[0] == 'P') {
          std::string key_in = "";
          //write something here that changes the octave of incoming stuff.
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
    for (int i = 0; i <= 4; i++) {
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols();
    }
    readJoy();
    
    uint16_t k0 = keyArray[0] << 8;
    uint8_t k1 = keyArray[1] << 4;
    uint8_t k2 = keyArray[2];
    uint16_t keysConcatenated = k0+k1+k2;
    uint32_t localCurrentStepSize = checkKeyPress(keysConcatenated, keyArray[3], keyArray[4]);

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
    //u8g2.sendBuffer();          // transfer internal memory to the display

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    //u8g2.clearBuffer();         // clear the internal memory
    u8g2.setCursor(2, 30);      // set coordinates to print result
    u8g2.print(keyArray[0], HEX);
    u8g2.print(keyArray[1], HEX);
    u8g2.print(keyArray[2], HEX);
    xSemaphoreGive(keyArrayMutex);
    xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);
    
    u8g2.setCursor(90,30);       // set coordinates to print knob values
    u8g2.print((int)round(knob0/2)+4,DEC);
    u8g2.print(knob1,DEC);
    u8g2.print(knob2,DEC);
    u8g2.print(knob3,DEC);
    //u8g2.sendBuffer();          // transfer internal memory to the display

    //u8g2.clearBuffer();
    u8g2.setCursor(64, 30);
    u8g2.print((char*) noteMessage);
    u8g2.sendBuffer();
    //Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    xSemaphoreGive(keyPressedVolMutex);
  }
}

void loop() {}
