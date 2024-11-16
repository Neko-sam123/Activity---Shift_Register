#include "images.h"
// OLED display
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//Input pins
#define LDR A0
#define Potentio A1
int Potval;
int Pot_state;
//IC pins
const int dataPin = 10;  // DS (Data Input)
const int clockPin = 9; // SH_CP (Shift Clock)
const int latchPin = 8;   // ST_CP (Latch Clock)
//timing
unsigned long currentT;
unsigned long LDRT = 0;
unsigned long PatternTime = 100;
//first pattern for enable display
unsigned long pattern_display1T = 0;
bool LDR_state = true ;
bool MSB_LSB = true;
int i = 0;
int LDRval;
const int LDR_threshold = 30;
//pattern variables
  int LDR_pattern_num = 0;
void centerText(const char *text ,int y) {
  int TEXT_SIZE = 1; 
  display.setTextSize(TEXT_SIZE);
  display.setTextColor(SSD1306_WHITE);
  int textWidth = strlen(text) * 6 * TEXT_SIZE; 
  int textHeight = 8 * TEXT_SIZE;            
  int x = (SCREEN_WIDTH - textWidth) / 2;
  //int y = (SCREEN_HEIGHT - textHeight) / 2;
  y -= textHeight;
  display.setCursor(x, y);
  display.println(text);
}

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(Potentio,INPUT);
  pinMode(LDR, INPUT);
  pinMode(dataPin,OUTPUT);
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  currentT = millis();
  Activate();
  Potentiometer();
}


void Activate(){
  if(LDR_state == true){
  display.clearDisplay();
  centerText("(OFF)",12);
  centerText("Shift Register",22);
  centerText("74HC595",32);
  display.display();
  while(LDR_state == true){
  currentT = millis();
  LDRval = analogRead(LDR);
  LDR_state = Light_Dependent_Resistor(LDRval);
    clear();
  }
  for(int fill = 0; fill <= 128; fill+= 1){
    display.fillRect(fill, 40, 1, 20, SSD1306_WHITE);
    centerText("Loading",54);
    display.display();
    }
  }
}
bool Light_Dependent_Resistor(int value){
  if (value <= LDR_threshold){
    return false;
  }else
    return true;
}
void shift_out_data_MSB(int pattern){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
}
void shift_out_data_LSB(int pattern){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
}
void clear(){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, 0b00000000);
  digitalWrite(latchPin, HIGH);
} 
void Potentiometer(){
  Potval = analogRead(Potentio);
  Pot_state = map(Potval, 0, 1023, 0, 5);
  switch(Pot_state){
    case 0:
      LDR_pattern();
      break;
    case 1:
      Display_dog();
      break;
    case 2:
      Display_cat();
      break;
    case 3:
      Display_image();
      break;
    case 4:
      Display_reset();
    default:
      break;
  }
}
void LDR_pattern(){
  LDRval = analogRead(LDR);

  if(LDRval <= LDR_threshold && LDR_pattern_num < 10 && currentT - LDRT >= 300){
    LDR_pattern_num++;
    LDRT = currentT;
  }
  if(LDR_pattern_num >= 10){
    LDR_pattern_num = 0;
  }
  switch(LDR_pattern_num){
    case 0:
      Pattern_1();
      break;
    case 1:
      Pattern_2();
      break;
    case 2:
      Pattern_3();
      break;
    case 3:
      Pattern_4();
      break;
    case 4:
      binaryCounterPattern();
      break;
    case 5:
      expandContractPattern();
      break;
    case 6:
      alternatingPattern();
      break;
    case 7:
      fillClearPattern();
      break;
    case 8:
      randomPattern();
      break;
    case 9:
      BlinkPattern();
      break;
    default:
      break;
  }
}
void BlinkPattern() {
  if(currentT - pattern_display1T >= PatternTime){
    static int pattern;
    static bool blink = true;
    if(blink == true){
    pattern = 0b00000000;
    }else{
      pattern = ~0b00000000;
    }
  shift_out_data_MSB(pattern);
  display.clearDisplay();
  draw_patterns(pattern);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Blink Pattern",32);
  display.display();
  blink = !blink;
  pattern_display1T = currentT;
  }
}
void randomPattern() {
  if(currentT - pattern_display1T >= PatternTime){
  int pattern = random(0, 256); 
  shiftOut(dataPin, clockPin, MSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  display.clearDisplay();
  draw_patterns(pattern);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Random Pattern",32);
  display.display();
  pattern_display1T = currentT;
  }
}
void fillClearPattern() {
   if(currentT - pattern_display1T >= PatternTime){
  static int step = 0;
  static bool filling = true;
  int pattern = filling ? (1 << step) - 1 : ~(1 << step) & 0xFF;
  shiftOut(dataPin, clockPin, MSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  step++;
  if (step > 8) {
    step = 0;
    filling = !filling; 
  }
  display.clearDisplay();
  draw_patterns(pattern);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Fill Clear Pattern",32);
  display.display();
  pattern_display1T = currentT;
  }
}
void binaryCounterPattern() {
  if(currentT - pattern_display1T >= PatternTime){
  static int counter = 0;
  shiftOut(dataPin, clockPin, MSBFIRST, counter);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  counter++;
  if (counter > 255) { 
    counter = 0;
  }
  display.clearDisplay();
  draw_patterns(counter);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Binary Counter",32);
  display.display();
  pattern_display1T = currentT;
  }
}
void alternatingPattern() {
  if(currentT - pattern_display1T >= PatternTime){
  static bool toggle = false;
  int pattern = toggle ? 0b10101010 : 0b01010101;
  shiftOut(dataPin, clockPin, MSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  toggle = !toggle;
  display.clearDisplay();
  draw_patterns(pattern);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Alternating Pattern",32);
  display.display();
  pattern_display1T = currentT;
  }
}
void expandContractPattern() {
  if(currentT - pattern_display1T >= PatternTime){
  static int step = 0;
  static bool expanding = true;
  int pattern;
  if (expanding) {
    pattern = (1 << step) | (1 << (7 - step)); 
  } else {
    pattern = ~((1 << step) | (1 << (7 - step))) & 0xFF; 
  }
  shiftOut(dataPin, clockPin, MSBFIRST, pattern);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  step++;
  if (step > 3) {
    step = 0;
    expanding = !expanding; 
  }
  display.clearDisplay();
  draw_patterns(pattern);
  centerText("(ON)",12);
  centerText("Shift Register",22);
  centerText("Expand and Contract",32);
  display.display();
  pattern_display1T = currentT;
  }
}
void Pattern_1(){
  if(currentT - pattern_display1T >= PatternTime){
    int pattern_display1 = 1 << i;
    display.clearDisplay();
    draw_patterns(pattern_display1);
    centerText("(ON)",12);
    centerText("Shift Register",22);
    centerText("Running light",32);
    display.display();
    if(MSB_LSB == true){
      i++;
    }else{
      i--;
    }
    if(i > 7 ){
      MSB_LSB = !MSB_LSB;
      i = 6;
    }
    if(i < 0){
      MSB_LSB = !MSB_LSB;
      i = 1;
    }
    shift_out_data_MSB(pattern_display1);
  pattern_display1T = currentT;
  }
}
void Pattern_2(){
  if(currentT - pattern_display1T >= PatternTime){
    int pattern_display2 = 1 << i;
    display.clearDisplay();
    draw_inverted_patterns(pattern_display2);
    centerText("(ON)",12);
    centerText("Shift Register",22);
    centerText("Inverted RunningLight",32);
    display.display();
    if(MSB_LSB == true){
      i++;
    }else{
      i--;
    }
    if(i > 7 ){
      MSB_LSB = !MSB_LSB;
      i = 6;
    }
    if(i < 0){
      MSB_LSB = !MSB_LSB;
      i = 1;
    }
    pattern_display2 = ~pattern_display2;
    shift_out_data_MSB(pattern_display2);
  pattern_display1T = currentT;
  }
}
void Pattern_3(){
  if(currentT - pattern_display1T >= PatternTime){
    int pattern_display_first =  1 << i ;
    int pattern_display_second =  1 << i + 2;
    int pattern_display_last = pattern_display_first | pattern_display_second;
    display.clearDisplay();
    draw_patterns(pattern_display_first);
    draw_patterns(pattern_display_second);
    centerText("(ON)",12);
    centerText("Shift Register",22);
    centerText("2 RunningLed",32);
    display.display();
    if(MSB_LSB == true){
      i++;
    }else{
      i--;
    }
    if(i > 7 ){
      MSB_LSB = !MSB_LSB;
      i = 6;
    }
    if(i <= -2){
      MSB_LSB = !MSB_LSB;
      i = -1;
    }
    if(i <= -2){
      shift_out_data_MSB(pattern_display_second);
    }else{
    shift_out_data_MSB(pattern_display_last);
    }
    pattern_display1T = currentT;
  }
}
void Pattern_4(){
  if(currentT - pattern_display1T >= PatternTime){
    int pattern_display_first =  1 << i ;
    int pattern_display_second =  1 << i + 2;
    int pattern_display_last = pattern_display_first | pattern_display_second;
    display.clearDisplay();
    draw_inverted_patterns(pattern_display_first);
    draw_inverted_patterns_1(pattern_display_second);
    centerText("(ON)",12);
    centerText("Shift Register",22);
    centerText("Inverted 2RunningLed",32);
    display.display();
    if(MSB_LSB == true){
      i++;
    }else{
      i--;
    }
    if(i > 7 ){
      MSB_LSB = !MSB_LSB;
      i = 6;
    }
    if(i <= -2){
      MSB_LSB = !MSB_LSB;
      i = -1;  
    }
    pattern_display_second = ~pattern_display_second;
    pattern_display_last = ~pattern_display_last;
    if(i <= -2){
      shift_out_data_MSB(pattern_display_second);
    }else{
    shift_out_data_MSB(pattern_display_last);
    }
    pattern_display1T = currentT;
  }
}
void draw_patterns(int num_box) {
  for (int i = 0; i < 8; i++) { 
    int x = i * 16; 
    display.drawRect(x, 40, 15, 20, SSD1306_WHITE); 

    if (num_box & (1 << i)) { 
      display.fillRect(x, 40, 15, 20, SSD1306_WHITE); 
    }
  }
}
void draw_inverted_patterns(int num_box) {
  for (int i = 0; i < 8; i++) {
    int x = i * 16; 
    display.fillRect(x, 40, 15, 20, SSD1306_WHITE); 
    if (num_box & (1 << i)) {
      display.fillRect(x, 40, 15, 20, SSD1306_BLACK); 
    }
  }
}
void draw_inverted_patterns_1(int num_box){
  for (int i = 0; i < 8; i++) {
    int x = i * 16; 
    if (num_box & (1 << i)) {
      display.fillRect(x, 40, 15, 20, SSD1306_BLACK); 
    }
  }
}
void Display_image(){
  shift_out_data_MSB(16);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_Image_to_code_russell2, 128, 64, SSD1306_WHITE);
  display.display();
}
void Display_cat(){
  shift_out_data_MSB(4);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_neko, 128, 64, SSD1306_WHITE);
  display.display();
}
void Display_dog(){
  shift_out_data_MSB(1);
  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_sam4, 128, 64, SSD1306_WHITE);
  display.display();
}
void Display_reset(){
  shift_out_data_MSB(128);
  display.clearDisplay();
  centerText("Block the LDR",22);
  centerText("To Turn Off",32);
  display.display();
  LDRval = analogRead(LDR);
  if(LDRval <= LDR_threshold){
    LDR_state = true;
    delay(200);
    }
}



