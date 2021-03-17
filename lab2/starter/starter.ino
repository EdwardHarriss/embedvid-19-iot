#include <U8g2lib.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>
#include <math.h>
#include <string.h>

class knob {
  private:
    uint8_t knobposition;
    uint8_t knobpreviousvalue;
    uint8_t upper_limit;
    uint8_t lower_limit;
  public:
    knob() {
      knobposition = 0;
      upper_limit = 16;
      lower_limit = 0;
    }

    uint8_t get_knob_position() {
      return knobposition;
    }

    uint8_t get_previous_value() {
      return knobpreviousvalue;
    }

    void set_previous_position(uint8_t new_prev) {
      knobpreviousvalue = new_prev;
    }

    void knobdecoder(uint8_t localCurrentKnob_3) {
      int8_t rotation = 0;
      if ((knobpreviousvalue == 0x0) && (localCurrentKnob_3 == 0x1)) {
        rotation = 1;
      }
      if ((knobpreviousvalue == 0x0) && (localCurrentKnob_3 == 0x2)) {
        rotation = -1;
      }
      if ((knobpreviousvalue == 0x1) && (localCurrentKnob_3 == 0x0)) {
        rotation = -1;
      }
      if ((knobpreviousvalue == 0x1) && (localCurrentKnob_3 == 0x3)) {
        rotation = 1;
      }
      if ((knobpreviousvalue == 0x2) && (localCurrentKnob_3 == 0x0)) {
        rotation = 1;
      }
      if ((knobpreviousvalue == 0x2) && (localCurrentKnob_3 == 0x3)) {
        rotation = -1;
      }
      if ((knobpreviousvalue == 0x3) && (localCurrentKnob_3 == 0x1)) {
        rotation = -1;
      }
      if ((knobpreviousvalue == 0x3) && (localCurrentKnob_3 == 0x2)) {
        rotation = 1;
      }
      if ((knobposition == 16) && (rotation == 1)) {
        return;
      }
      if ((knobposition == 0) && (rotation == -1)) {
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
knob knob_3;
volatile uint8_t keyArray[7];
volatile char noteMessage[] = "   ";
std::string keysPressedVol = "";
const char intToHex[] = "0123456789ABCDEF";

uint32_t calcPhaseStep(uint32_t freq) { //TODO: implement octaves
  return round((freq * ones) / fs);
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

uint32_t checkKeyPress(uint16_t keyarray) {
  std::string keysPressed = "";
  uint32_t stepSizeReturn = currentStepSize; //default if none of the if conditions are met
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
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[0];
      break;
    case 0xDFF:
      keysPressed += 'C';
      keysPressed += '#';
      stepSizeReturn = stepSizes[1];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[1];
      break;
    case 0xBFF:
      keysPressed += 'D';
      stepSizeReturn = stepSizes[2];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[2];
      break;
    case 0x7FF:
      keysPressed += 'E';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[3];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[3];
      break;
    case 0xFEF:
      keysPressed += 'E';
      stepSizeReturn = stepSizes[4];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[4];
      break;
    case 0xFDF:
      keysPressed += 'F';
      stepSizeReturn = stepSizes[5];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[5];
      break;
    case 0xFBF:
      keysPressed += 'F';
      keysPressed += '#';
      stepSizeReturn = stepSizes[6];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[6];
      break;
    case 0xF7F:
      keysPressed += 'G';
      stepSizeReturn = stepSizes[7];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[7];
      break;
    case 0xFFE:
      keysPressed += 'G';
      keysPressed += '#';
      stepSizeReturn = stepSizes[8];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[8];
      break;
    case 0xFFD:
      keysPressed += 'A';
      stepSizeReturn = stepSizes[9];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[9];
      break;
    case 0xFFB:
      keysPressed += 'B';
      keysPressed += 'b';
      stepSizeReturn = stepSizes[10];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[10];
      break;
    case 0xFF7:
      keysPressed += 'B';
      stepSizeReturn = stepSizes[11];
      noteMessage[0] = 'P';
      noteMessage[1] = '4';
      noteMessage[2] = intToHex[11];
      break;
    default:
      stepSizeReturn = stepSizeReturn;
      noteMessage[0] = noteMessage[0];
      noteMessage[1] = noteMessage[1];
      noteMessage[2] = noteMessage[2];
  }
  
  xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);
  keysPressedVol = keysPressed;
  xSemaphoreGive(keyPressedVolMutex);
  return stepSizeReturn;
}

void sampleISR() {
  static uint32_t phaseAcc = 0;
  phaseAcc += currentStepSize;
  uint8_t outValue = (phaseAcc >> 24) >> (8 - knob_3.get_knob_position()/2);
  analogWrite(OUTR_PIN, outValue);
}

void msgOutTask(void *pvParameters) {
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  char outMsg[4];
  while (1) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    xQueueReceive(msgOutQ, outMsg, portMAX_DELAY);
    //Serial.println(outMsg);
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
    for (int i = 0; i <= 3; i++) {
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols();
    }
    uint16_t k0 = keyArray[0] << 8;
    uint8_t k1 = keyArray[1] << 4;
    uint8_t k2 = keyArray[2];
    uint16_t keysConcatenated = k0+k1+k2;
    uint32_t localCurrentStepSize = checkKeyPress(keysConcatenated);
    xQueueSend( msgOutQ, (char*) noteMessage, portMAX_DELAY);
    uint8_t localCurrentKnob_3 = keyArray[3] & 0x3;
    knob_3.knobdecoder(localCurrentKnob_3);
    knob_3.set_previous_position(localCurrentKnob_3);
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
    u8g2.sendBuffer();          // transfer internal memory to the display

    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setCursor(2, 30);      // set coordinates to print result
    u8g2.print(keyArray[0], HEX);
    u8g2.print(keyArray[1], HEX);
    u8g2.print(keyArray[2], HEX);
    xSemaphoreGive(keyArrayMutex);
    xSemaphoreTake(keyPressedVolMutex, portMAX_DELAY);
    u8g2.print(knob_3.get_previous_value(), HEX);
    u8g2.print(knob_3.get_knob_position(), HEX);
    u8g2.sendBuffer();          // transfer internal memory to the display

    u8g2.clearBuffer();
    u8g2.setCursor(64, 30);
    u8g2.print((char*) noteMessage);
    u8g2.sendBuffer();
    //Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    xSemaphoreGive(keyPressedVolMutex);
  }
}

void loop() {}
