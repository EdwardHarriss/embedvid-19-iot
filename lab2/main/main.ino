#include <U8g2lib.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <knob.h>
#include <LFO.h>

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
SemaphoreHandle_t keysPressedVolMutex;
SemaphoreHandle_t knobsMutex;
SemaphoreHandle_t octaveMutex;
SemaphoreHandle_t lfoMutex;
SemaphoreHandle_t stepSizeMutex;
SemaphoreHandle_t joyMutex;
SemaphoreHandle_t modeMutex;

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
  keysPressedVolMutex = xSemaphoreCreateMutex();
  knobsMutex = xSemaphoreCreateMutex();
  octaveMutex = xSemaphoreCreateMutex();
  lfoMutex = xSemaphoreCreateMutex();
  stepSizeMutex = xSemaphoreCreateMutex();
  joyMutex = xSemaphoreCreateMutex();
  modeMutex = xSemaphoreCreateMutex();

  //set limits and button toggle rules for knobs:
  knob_2.set_lower_limit(-8);
  knob_2.set_upper_limit(8);
  knob_0.setToggle(1); //VIBRATO MODE TOGGLES ON BUTTON 0
  knob_0.set_upper_limit(16);
  knob_0.set_lower_limit(-16);
  knob_2.setToggle(1); //TREMOLO MODE TOGGLES ON BUTTON 2
  knob_3.set_lower_limit(0); //setting min volume
  knob_1.set_upper_limit(4);
  knob_1.set_lower_limit(0);

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
    1, /* Task priority */
    &scanKeysHandle ); /* Pointer to store the task handle */

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(
    displayUpdateTask,
    "displayUpdate",
    32,
    NULL,
    4,
    &displayUpdateHandle );

  TaskHandle_t msgOutHandle = NULL;
  xTaskCreate(
    msgOutTask,
    "msgOut",
    32,
    NULL,
    5,
    &msgOutHandle );

  TaskHandle_t msgInHandle = NULL;
  xTaskCreate(
    msgInTask,
    "msgIn",
    32,
    NULL,
    3,
    &msgInHandle );

    TaskHandle_t LFOTaskHandle = NULL;
  xTaskCreate(
    LFOTask,
    "LFOtask",
    32,
    NULL,
    2,
    &LFOTaskHandle );

    TaskHandle_t SustainCounterTaskHandle = NULL;
  xTaskCreate(
    SustainCounterTask,
    "SustainCounterTask",
    8,
    NULL,
    6,
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
const std::string waveform_selected [] = {"Saw", "Sin", "Sqr"};
const uint32_t SLUT [] = { 32768, 32868, 32969, 33069, 33170, 33270, 33371, 33471, 33572, 33673, 33773, 33874, 33974, 34075, 34175, 34276, 34376, 34477, 34577, 34677, 34778, 34878, 34979, 35079, 35179, 35279, 35380, 35480, 35580, 35680, 35781, 35881, 35981, 36081, 36181, 36281, 36381, 36481, 36581, 36681, 36781, 36880, 36980, 37080, 37180, 37279, 37379, 37478, 37578, 37677, 37777, 37876, 37975, 38075, 38174, 38273, 38372, 38471, 38570, 38669, 38768, 38867, 38966, 39065, 39163, 39262, 39360, 39459, 39557, 39656, 39754, 39852, 39950, 40048, 40146, 40244, 40342, 40440, 40538, 40636, 40733, 40831, 40928, 41026, 41123, 41220, 41317, 41414, 41511, 41608, 41705, 41802, 41898, 41995, 42091, 42188, 42284, 42380, 42476, 42572, 42668, 42764, 42860, 42955, 43051, 43146, 43242, 43337, 43432, 43527, 43622, 43717, 43812, 43906, 44001, 44095, 44190, 44284, 44378, 44472, 44566, 44660, 44753, 44847, 44940, 45034, 45127, 45220, 45313, 45406, 45498, 45591, 45684, 45776, 45868, 45960, 46052, 46144, 46236, 46328, 46419, 46510, 46602, 46693, 46784, 46875, 46965, 47056, 47146, 47237, 47327, 47417, 47507, 47597, 47686, 47776, 47865, 47954, 48043, 48132, 48221, 48310, 48398, 48486, 48575, 48663, 48750, 48838, 48926, 49013, 49100, 49188, 49274, 49361, 49448, 49534, 49621, 49707, 49793, 49879, 49965, 50050, 50135, 50221, 50306, 50391, 50475, 50560, 50644, 50728, 50812, 50896, 50980, 51064, 51147, 51230, 51313, 51396, 51479, 51561, 51643, 51725, 51807, 51889, 51971, 52052, 52133, 52214, 52295, 52376, 52456, 52537, 52617, 52697, 52777, 52856, 52935, 53015, 53094, 53172, 53251, 53329, 53408, 53486, 53563, 53641, 53719, 53796, 53873, 53950, 54026, 54103, 54179, 54255, 54331, 54406, 54482, 54557, 54632, 54707, 54782, 54856, 54930, 55004, 55078, 55151, 55225, 55298, 55371, 55444, 55516, 55588, 55660, 55732, 55804, 55875, 55947, 56017, 56088, 56159, 56229, 56299, 56369, 56439, 56508, 56577, 56646, 56715, 56784, 56852, 56920, 56988, 57056, 57123, 57190, 57257, 57324, 57390, 57457, 57523, 57588, 57654, 57719, 57784, 57849, 57914, 57978, 58042, 58106, 58170, 58233, 58296, 58359, 58422, 58484, 58547, 58609, 58670, 58732, 58793, 58854, 58915, 58975, 59035, 59095, 59155, 59215, 59274, 59333, 59392, 59450, 59509, 59567, 59624, 59682, 59739, 59796, 59853, 59909, 59965, 60021, 60077, 60133, 60188, 60243, 60297, 60352, 60406, 60460, 60513, 60567, 60620, 60673, 60725, 60778, 60830, 60882, 60933, 60984, 61035, 61086, 61137, 61187, 61237, 61286, 61336, 61385, 61434, 61482, 61531, 61579, 61626, 61674, 61721, 61768, 61815, 61861, 61907, 61953, 61999, 62044, 62089, 62134, 62178, 62223, 62267, 62310, 62354, 62397, 62440, 62482, 62524, 62566, 62608, 62649, 62690, 62731, 62772, 62812, 62852, 62892, 62931, 62970, 63009, 63048, 63086, 63124, 63162, 63199, 63237, 63273, 63310, 63346, 63382, 63418, 63453, 63489, 63523, 63558, 63592, 63626, 63660, 63693, 63726, 63759, 63792, 63824, 63856, 63887, 63919, 63950, 63981, 64011, 64041, 64071, 64101, 64130, 64159, 64188, 64216, 64244, 64272, 64299, 64327, 64354, 64380, 64407, 64433, 64458, 64484, 64509, 64534, 64558, 64582, 64606, 64630, 64653, 64676, 64699, 64721, 64744, 64765, 64787, 64808, 64829, 64850, 64870, 64890, 64910, 64929, 64948, 64967, 64985, 65004, 65022, 65039, 65056, 65073, 65090, 65106, 65122, 65138, 65154, 65169, 65184, 65198, 65212, 65226, 65240, 65253, 65266, 65279, 65291, 65303, 65315, 65327, 65338, 65349, 65359, 65370, 65380, 65389, 65398, 65407, 65416, 65425, 65433, 65440, 65448, 65455, 65462, 65469, 65475, 65481, 65486, 65492, 65497, 65501, 65506, 65510, 65514, 65517, 65520, 65523, 65526, 65528, 65530, 65532, 65533, 65534, 65535, 65535, 65535, 65535, 65534, 65533, 65532, 65531, 65529, 65527, 65524, 65522, 65519, 65515, 65512, 65508, 65504, 65499, 65494, 65489, 65484, 65478, 65472, 65465, 65459, 65452, 65444, 65437, 65429, 65420, 65412, 65403, 65394, 65384, 65375, 65364, 65354, 65343, 65332, 65321, 65309, 65297, 65285, 65273, 65260, 65247, 65233, 65219, 65205, 65191, 65176, 65161, 65146, 65130, 65114, 65098, 65082, 65065, 65048, 65030, 65013, 64995, 64976, 64958, 64939, 64919, 64900, 64880, 64860, 64839, 64819, 64798, 64776, 64754, 64733, 64710, 64688, 64665, 64642, 64618, 64594, 64570, 64546, 64521, 64496, 64471, 64445, 64420, 64393, 64367, 64340, 64313, 64286, 64258, 64230, 64202, 64173, 64144, 64115, 64086, 64056, 64026, 63996, 63965, 63934, 63903, 63872, 63840, 63808, 63775, 63743, 63710, 63677, 63643, 63609, 63575, 63541, 63506, 63471, 63436, 63400, 63364, 63328, 63292, 63255, 63218, 63181, 63143, 63105, 63067, 63029, 62990, 62951, 62912, 62872, 62832, 62792, 62752, 62711, 62670, 62629, 62587, 62545, 62503, 62461, 62418, 62375, 62332, 62288, 62245, 62201, 62156, 62112, 62067, 62022, 61976, 61930, 61884, 61838, 61792, 61745, 61698, 61650, 61603, 61555, 61507, 61458, 61409, 61360, 61311, 61262, 61212, 61162, 61111, 61061, 61010, 60959, 60907, 60856, 60804, 60752, 60699, 60646, 60593, 60540, 60487, 60433, 60379, 60325, 60270, 60215, 60160, 60105, 60049, 59993, 59937, 59881, 59824, 59768, 59710, 59653, 59595, 59538, 59479, 59421, 59362, 59304, 59244, 59185, 59125, 59066, 59005, 58945, 58884, 58824, 58762, 58701, 58639, 58578, 58516, 58453, 58391, 58328, 58265, 58201, 58138, 58074, 58010, 57946, 57881, 57817, 57752, 57687, 57621, 57555, 57490, 57423, 57357, 57290, 57224, 57157, 57089, 57022, 56954, 56886, 56818, 56749, 56681, 56612, 56543, 56474, 56404, 56334, 56264, 56194, 56124, 56053, 55982, 55911, 55840, 55768, 55696, 55624, 55552, 55480, 55407, 55334, 55261, 55188, 55115, 55041, 54967, 54893, 54819, 54744, 54670, 54595, 54520, 54444, 54369, 54293, 54217, 54141, 54065, 53988, 53911, 53834, 53757, 53680, 53602, 53525, 53447, 53369, 53290, 53212, 53133, 53054, 52975, 52896, 52816, 52737, 52657, 52577, 52497, 52416, 52336, 52255, 52174, 52093, 52011, 51930, 51848, 51766, 51684, 51602, 51520, 51437, 51354, 51272, 51188, 51105, 51022, 50938, 50854, 50770, 50686, 50602, 50518, 50433, 50348, 50263, 50178, 50093, 50007, 49922, 49836, 49750, 49664, 49578, 49491, 49405, 49318, 49231, 49144, 49057, 48969, 48882, 48794, 48706, 48619, 48530, 48442, 48354, 48265, 48177, 48088, 47999, 47910, 47820, 47731, 47641, 47552, 47462, 47372, 47282, 47192, 47101, 47011, 46920, 46829, 46738, 46647, 46556, 46465, 46373, 46282, 46190, 46098, 46006, 45914, 45822, 45730, 45637, 45545, 45452, 45359, 45266, 45173, 45080, 44987, 44894, 44800, 44706, 44613, 44519, 44425, 44331, 44237, 44143, 44048, 43954, 43859, 43764, 43670, 43575, 43480, 43385, 43289, 43194, 43099, 43003, 42908, 42812, 42716, 42620, 42524, 42428, 42332, 42236, 42139, 42043, 41947, 41850, 41753, 41656, 41560, 41463, 41366, 41269, 41171, 41074, 40977, 40879, 40782, 40684, 40587, 40489, 40391, 40293, 40195, 40097, 39999, 39901, 39803, 39705, 39606, 39508, 39410, 39311, 39213, 39114, 39015, 38916, 38818, 38719, 38620, 38521, 38422, 38323, 38223, 38124, 38025, 37926, 37826, 37727, 37628, 37528, 37429, 37329, 37229, 37130, 37030, 36930, 36830, 36731, 36631, 36531, 36431, 36331, 36231, 36131, 36031, 35931, 35831, 35731, 35630, 35530, 35430, 35330, 35229, 35129, 35029, 34928, 34828, 34728, 34627, 34527, 34426, 34326, 34225, 34125, 34024, 33924, 33823, 33723, 33622, 33522, 33421, 33321, 33220, 33120, 33019, 32918, 32818, 32717, 32617, 32516, 32415, 32315, 32214, 32114, 32013, 31913, 31812, 31712, 31611, 31511, 31410, 31310, 31209, 31109, 31008, 30908, 30807, 30707, 30607, 30506, 30406, 30306, 30205, 30105, 30005, 29905, 29804, 29704, 29604, 29504, 29404, 29304, 29204, 29104, 29004, 28904, 28804, 28705, 28605, 28505, 28405, 28306, 28206, 28106, 28007, 27907, 27808, 27709, 27609, 27510, 27411, 27312, 27212, 27113, 27014, 26915, 26816, 26717, 26619, 26520, 26421, 26322, 26224, 26125, 26027, 25929, 25830, 25732, 25634, 25536, 25438, 25340, 25242, 25144, 25046, 24948, 24851, 24753, 24656, 24558, 24461, 24364, 24266, 24169, 24072, 23975, 23879, 23782, 23685, 23588, 23492, 23396, 23299, 23203, 23107, 23011, 22915, 22819, 22723, 22627, 22532, 22436, 22341, 22246, 22150, 22055, 21960, 21865, 21771, 21676, 21581, 21487, 21392, 21298, 21204, 21110, 21016, 20922, 20829, 20735, 20641, 20548, 20455, 20362, 20269, 20176, 20083, 19990, 19898, 19805, 19713, 19621, 19529, 19437, 19345, 19253, 19162, 19070, 18979, 18888, 18797, 18706, 18615, 18524, 18434, 18343, 18253, 18163, 18073, 17983, 17894, 17804, 17715, 17625, 17536, 17447, 17358, 17270, 17181, 17093, 17005, 16916, 16829, 16741, 16653, 16566, 16478, 16391, 16304, 16217, 16130, 16044, 15957, 15871, 15785, 15699, 15613, 15528, 15442, 15357, 15272, 15187, 15102, 15017, 14933, 14849, 14765, 14681, 14597, 14513, 14430, 14347, 14263, 14181, 14098, 14015, 13933, 13851, 13769, 13687, 13605, 13524, 13442, 13361, 13280, 13199, 13119, 13038, 12958, 12878, 12798, 12719, 12639, 12560, 12481, 12402, 12323, 12245, 12166, 12088, 12010, 11933, 11855, 11778, 11701, 11624, 11547, 11470, 11394, 11318, 11242, 11166, 11091, 11015, 10940, 10865, 10791, 10716, 10642, 10568, 10494, 10420, 10347, 10274, 10201, 10128, 10055, 9983, 9911, 9839, 9767, 9695, 9624, 9553, 9482, 9411, 9341, 9271, 9201, 9131, 9061, 8992, 8923, 8854, 8786, 8717, 8649, 8581, 8513, 8446, 8378, 8311, 8245, 8178, 8112, 8045, 7980, 7914, 7848, 7783, 7718, 7654, 7589, 7525, 7461, 7397, 7334, 7270, 7207, 7144, 7082, 7019, 6957, 6896, 6834, 6773, 6711, 6651, 6590, 6530, 6469, 6410, 6350, 6291, 6231, 6173, 6114, 6056, 5997, 5940, 5882, 5825, 5767, 5711, 5654, 5598, 5542, 5486, 5430, 5375, 5320, 5265, 5210, 5156, 5102, 5048, 4995, 4942, 4889, 4836, 4783, 4731, 4679, 4628, 4576, 4525, 4474, 4424, 4373, 4323, 4273, 4224, 4175, 4126, 4077, 4028, 3980, 3932, 3885, 3837, 3790, 3743, 3697, 3651, 3605, 3559, 3513, 3468, 3423, 3379, 3334, 3290, 3247, 3203, 3160, 3117, 3074, 3032, 2990, 2948, 2906, 2865, 2824, 2783, 2743, 2703, 2663, 2623, 2584, 2545, 2506, 2468, 2430, 2392, 2354, 2317, 2280, 2243, 2207, 2171, 2135, 2099, 2064, 2029, 1994, 1960, 1926, 1892, 1858, 1825, 1792, 1760, 1727, 1695, 1663, 1632, 1601, 1570, 1539, 1509, 1479, 1449, 1420, 1391, 1362, 1333, 1305, 1277, 1249, 1222, 1195, 1168, 1142, 1115, 1090, 1064, 1039, 1014, 989, 965, 941, 917, 893, 870, 847, 825, 802, 781, 759, 737, 716, 696, 675, 655, 635, 616, 596, 577, 559, 540, 522, 505, 487, 470, 453, 437, 421, 405, 389, 374, 359, 344, 330, 316, 302, 288, 275, 262, 250, 238, 226, 214, 203, 192, 181, 171, 160, 151, 141, 132, 123, 115, 106, 98, 91, 83, 76, 70, 63, 57, 51, 46, 41, 36, 31, 27, 23, 20, 16, 13, 11, 8, 6, 4, 3, 2, 1, 0, 0, 0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 29, 34, 38, 43, 49, 54, 60, 66, 73, 80, 87, 95, 102, 110, 119, 128, 137, 146, 155, 165, 176, 186, 197, 208, 220, 232, 244, 256, 269, 282, 295, 309, 323, 337, 351, 366, 381, 397, 413, 429, 445, 462, 479, 496, 513, 531, 550, 568, 587, 606, 625, 645, 665, 685, 706, 727, 748, 770, 791, 814, 836, 859, 882, 905, 929, 953, 977, 1001, 1026, 1051, 1077, 1102, 1128, 1155, 1181, 1208, 1236, 1263, 1291, 1319, 1347, 1376, 1405, 1434, 1464, 1494, 1524, 1554, 1585, 1616, 1648, 1679, 1711, 1743, 1776, 1809, 1842, 1875, 1909, 1943, 1977, 2012, 2046, 2082, 2117, 2153, 2189, 2225, 2262, 2298, 2336, 2373, 2411, 2449, 2487, 2526, 2565, 2604, 2643, 2683, 2723, 2763, 2804, 2845, 2886, 2927, 2969, 3011, 3053, 3095, 3138, 3181, 3225, 3268, 3312, 3357, 3401, 3446, 3491, 3536, 3582, 3628, 3674, 3720, 3767, 3814, 3861, 3909, 3956, 4004, 4053, 4101, 4150, 4199, 4249, 4298, 4348, 4398, 4449, 4500, 4551, 4602, 4653, 4705, 4757, 4810, 4862, 4915, 4968, 5022, 5075, 5129, 5183, 5238, 5292, 5347, 5402, 5458, 5514, 5570, 5626, 5682, 5739, 5796, 5853, 5911, 5968, 6026, 6085, 6143, 6202, 6261, 6320, 6380, 6440, 6500, 6560, 6620, 6681, 6742, 6803, 6865, 6926, 6988, 7051, 7113, 7176, 7239, 7302, 7365, 7429, 7493, 7557, 7621, 7686, 7751, 7816, 7881, 7947, 8012, 8078, 8145, 8211, 8278, 8345, 8412, 8479, 8547, 8615, 8683, 8751, 8820, 8889, 8958, 9027, 9096, 9166, 9236, 9306, 9376, 9447, 9518, 9588, 9660, 9731, 9803, 9875, 9947, 10019, 10091, 10164, 10237, 10310, 10384, 10457, 10531, 10605, 10679, 10753, 10828, 10903, 10978, 11053, 11129, 11204, 11280, 11356, 11432, 11509, 11585, 11662, 11739, 11816, 11894, 11972, 12049, 12127, 12206, 12284, 12363, 12441, 12520, 12600, 12679, 12758, 12838, 12918, 12998, 13079, 13159, 13240, 13321, 13402, 13483, 13564, 13646, 13728, 13810, 13892, 13974, 14056, 14139, 14222, 14305, 14388, 14471, 14555, 14639, 14723, 14807, 14891, 14975, 15060, 15144, 15229, 15314, 15400, 15485, 15570, 15656, 15742, 15828, 15914, 16001, 16087, 16174, 16261, 16347, 16435, 16522, 16609, 16697, 16785, 16872, 16960, 17049, 17137, 17225, 17314, 17403, 17492, 17581, 17670, 17759, 17849, 17938, 18028, 18118, 18208, 18298, 18389, 18479, 18570, 18660, 18751, 18842, 18933, 19025, 19116, 19207, 19299, 19391, 19483, 19575, 19667, 19759, 19851, 19944, 20037, 20129, 20222, 20315, 20408, 20501, 20595, 20688, 20782, 20875, 20969, 21063, 21157, 21251, 21345, 21440, 21534, 21629, 21723, 21818, 21913, 22008, 22103, 22198, 22293, 22389, 22484, 22580, 22675, 22771, 22867, 22963, 23059, 23155, 23251, 23347, 23444, 23540, 23637, 23733, 23830, 23927, 24024, 24121, 24218, 24315, 24412, 24509, 24607, 24704, 24802, 24899, 24997, 25095, 25193, 25291, 25389, 25487, 25585, 25683, 25781, 25879, 25978, 26076, 26175, 26273, 26372, 26470, 26569, 26668, 26767, 26866, 26965, 27064, 27163, 27262, 27361, 27460, 27560, 27659, 27758, 27858, 27957, 28057, 28156, 28256, 28355, 28455, 28555, 28655, 28754, 28854, 28954, 29054, 29154, 29254, 29354, 29454, 29554, 29654, 29754, 29855, 29955, 30055, 30155, 30256, 30356, 30456, 30556, 30657, 30757, 30858, 30958, 31058, 31159, 31259, 31360, 31460, 31561, 31661, 31762, 31862, 31963, 32064, 32164, 32265, 32365, 32466, 32566, 32667 };

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
  bool recModeloc;
  __atomic_store_n(&recModeloc, receiveMode, __ATOMIC_RELAXED);
  if (recModeloc == 0){ // send mode - uses own set octave
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
  uint32_t valX = xMap/10;
  uint32_t valY = yMap/10;
  __atomic_store_n(&joyValues[0], valX, __ATOMIC_RELAXED);
  __atomic_store_n(&joyValues[1], valY, __ATOMIC_RELAXED);
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
  uint32_t stepSizeReturn;
  __atomic_store_n(&stepSizeReturn, currentStepSize, __ATOMIC_RELAXED);//default if none of the if conditions are met

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

  xSemaphoreTake(octaveMutex, portMAX_DELAY);
  octaveOwn = round((knob_2.get_knob_position()/2)+4); //to get set keyboard octave, divide knob0 position by 2 and add 4
  xSemaphoreGive(octaveMutex);

  //check button positions
  uint8_t button_0 = k6 & 0b1;
  uint8_t button_1 = k6 & 0b10;
  uint8_t button_2 = k5 & 0b1;
  uint8_t button_3 = k5 & 0b10;
  xSemaphoreTake(knobsMutex, portMAX_DELAY);
  knob_0.update_buttonState(button_0);
  knob_1.update_buttonState(button_1);
  knob_2.update_buttonState(button_2);
  knob_3.update_buttonState(button_3);
  xSemaphoreGive(knobsMutex);
  //check joystick button
  uint8_t button_joy = (k5 & 0b100) >> 2; //get button press value for joystick
  xSemaphoreTake(joyMutex, portMAX_DELAY);
  joyValues[2] = !button_joy; //sets the correct button value for joystick (is usually 1 when not pressed and 0 when pressed, this inverts that)
  xSemaphoreGive(joyMutex);

  //distinguish which octave to use, either knob-based one or received-message-based one
  xSemaphoreTake(octaveMutex, portMAX_DELAY);
  setOctave(); // sets either local octave or octave of received message
  char o [1];
  itoa(octave, o, 16);
  xSemaphoreGive(octaveMutex);

  //getting pressed keys
  //NEED TO ADD CASES FOR MULTIPLE KEYS
  xSemaphoreTake(modeMutex, portMAX_DELAY);
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
  xSemaphoreGive(modeMutex);
  if (isKeyPressed==1){
        sustainCounter = 6;
      }

  xSemaphoreTake(keysPressedVolMutex, portMAX_DELAY);
  keysPressedVol = keysPressed;
  xSemaphoreGive(keysPressedVolMutex);
  }
  return stepSizeReturn;
}

void sampleISR() {
  static uint32_t phaseAcc = 0;
  static uint32_t wave_value = 0;
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

  if(locknobsrot[1]/2 < 1){ //sawtooth
    wave_value = phaseAcc >> 24;
  }
  else if(locknobsrot[1]/2 < 2){ //sine
    wave_value = SLUT[phaseAcc >> 22] >> 6;
  }
  else if(locknobsrot[1]/2 < 3){  //square
    if(phaseAcc < pow(2,31)){
      wave_value = 0;
    }
    else{
      wave_value = 255;
    }
  }

  //writes phase step to output pin
  outValue = wave_value >> volAdjust;
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
          xSemaphoreTake(octaveMutex, portMAX_DELAY);
          octaveRec = ((uint8_t) inMsg[1]) - 48; //takes value from ASCII int to int
          xSemaphoreGive(octaveMutex);
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
    //int32_t locJoyX, locJoyY;
    //bool locJoyButton;
    //__atomic_store_n(&locJoyX, joyValues[0], __ATOMIC_RELAXED);
    //__atomic_store_n(&locJoyY, joyValues[1], __ATOMIC_RELAXED);
    //__atomic_store_n(&locJoyButton, joyValues[2], __ATOMIC_RELAXED);
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    // NEED TO REINCLUDE
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    for (int i = 0; i <= 6; i++) {
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols();
    }
    readJoy(); //get joystick x and y values

    //getting notes:
    uint16_t k0 = keyArray[0] << 8;
    uint8_t k1 = keyArray[1] << 4;
    uint8_t k2 = keyArray[2];
    uint16_t keysConcatenated = k0+k1+k2;
    //checking key presses, knob rotations, etc performed inside checkKeyPress()
    xSemaphoreTake(stepSizeMutex, portMAX_DELAY);
    uint32_t localCurrentStepSize = checkKeyPress(keysConcatenated, keyArray[3], keyArray[4], keyArray[5], keyArray[6]);
    xSemaphoreGive(stepSizeMutex);
    //update required functional values based on all keyboard element changes detected

    //update vibrato LFO:
    xSemaphoreTake(lfoMutex, portMAX_DELAY);

    lfoVib.change_counterIncr(round(joyValues[1]/30));//updates vibrato speed based on joystick Y input
    xSemaphoreTake(knobsMutex, portMAX_DELAY);
    lfoVib.change_max(knob_0.get_knob_position()/3);//updates vibrato range based on knob_1 rotational position
    xSemaphoreGive(knobsMutex);
    if (joyValues[2] == 1){ //reset lfo by clicking joystick button if vibrato goes too far, and you don't want to spend all the time winding it back
      lfoVib.reset_counter();

      xSemaphoreTake(knobsMutex, portMAX_DELAY);
      knob_0.reset_knob_position();
      xSemaphoreGive(knobsMutex);
    }

    xSemaphoreGive(lfoMutex);

    //update keyboard rules based on button values e.g. send/receive mode, vibrato, etc.
    xSemaphoreTake(knobsMutex, portMAX_DELAY);
    updateKeyboardRules(knob_0.get_buttonState(), knob_1.get_buttonState(), knob_2.get_buttonState(), knob_3.get_buttonState(), joyValues[2]);
    xSemaphoreGive(knobsMutex);

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
    xSemaphoreTake(modeMutex, portMAX_DELAY);
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
    xSemaphoreGive(modeMutex);

    //VIBRATO OPTIONS - SPEED AND RANGE
     u8g2.setFont(u8g2_font_blipfest_07_tr);
     u8g2.drawStr(2,20,"Speed: ");
     u8g2.setCursor(25, 20);
     xSemaphoreTake(lfoMutex, portMAX_DELAY);
     u8g2.print(lfoVib.get_incr(),DEC); //////////////////MAY NEED SEMAPHORE
     xSemaphoreGive(lfoMutex);
     u8g2.drawStr(2,30,"Range: ");
     std::string rangeChange;
     xSemaphoreTake(knobsMutex, portMAX_DELAY);
     int change = (int)round(knob_0.get_knob_position()/5);
     xSemaphoreGive(knobsMutex);
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
    xSemaphoreTake(knobsMutex, portMAX_DELAY);
    u8g2.print(knob_3.get_knob_position(),DEC);
    xSemaphoreGive(knobsMutex);

    //OCTAVE:
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(97,20,"Oct: ");
    xSemaphoreTake(octaveMutex, portMAX_DELAY);
    std::string octaveString = std::to_string(octave);
    xSemaphoreGive(octaveMutex);
    u8g2.drawStr(117, 20, octaveString.c_str()); // write something to the internal memory

    //KEY PRESSED:
    xSemaphoreTake(keysPressedVolMutex, portMAX_DELAY);
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(40,20,"Key: ");
    u8g2.drawStr(66, 20, keysPressedVol.c_str()); // write note
    xSemaphoreGive(keysPressedVolMutex);

    //SERIAL MESSAGE SENT (could take out)
    u8g2.setFont(u8g2_font_ncenR08_tr);
    u8g2.drawStr(40,30,"Msg: ");
    u8g2.setCursor(65, 30);
    u8g2.print((char*) noteMessage);

    //SEND VS RECEIVE MODE (if it is between receiving a Pxx and Rxx over serial)
    u8g2.drawStr(95,30,"S/R: ");
    std::string sendReceive;
    xSemaphoreTake(modeMutex, portMAX_DELAY);
    if (receiveMode==1){
      sendReceive = "R";
    }
    else{
      sendReceive = "S";
    }
    xSemaphoreGive(modeMutex);
    u8g2.drawStr(117, 30, sendReceive.c_str());

    //ROTATION LEFTOVERS (could replace message being sent with these)

    std::string waveform;
    xSemaphoreTake(knobsMutex, portMAX_DELAY);
    int waveformMode = knob_1.get_knob_position();
    xSemaphoreGive(knobsMutex);
    switch(waveformMode){
      case(0):
        waveform = waveform_selected[0];
        break;
      case(2):
        waveform = waveform_selected[1];
        break;
      case(4):
        waveform = waveform_selected[2];
        break;
    }
    u8g2.setFont(u8g2_font_blipfest_07_tr);      // set coordinates to print knob values
    u8g2.drawStr(80, 20, waveform.c_str());

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
     xSemaphoreTake(lfoMutex, portMAX_DELAY);
     lfoVib.update_counter();
     lfoTrem.update_counter();
     xSemaphoreGive(lfoMutex);
     //Serial.println(lfoVib.get_counter());
   }
}

void SustainCounterTask(void *pvParameters) {
  const TickType_t xFrequency = 600 / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
     vTaskDelayUntil(&xLastWakeTime, xFrequency);
     xSemaphoreTake(modeMutex, portMAX_DELAY);
     if ((sustainCounter>0)&&(sustainMode)){
      sustainCounter--;
     }
     xSemaphoreGive(modeMutex);
   }
}

void loop() {}
