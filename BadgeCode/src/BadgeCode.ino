#include <Arduino.h>
/*
  06/03/2017
  Author: Badge Pirates (FG, RIXON, NETWORKGEEK)
  Platforms: ESP32
  Language: C++
  File: BadgeCode.ino
  ------------------------------------------------------------------------
  Description:
  SECKC DC25 Badge using ESP32 Thing and OLED display
  ------------------------------------------------------------------------
*/

#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>
#include <Bounce2.h>
#include <WiFi.h>
#include <Sprite.h>
#include <esp_wifi.h>

// Debug
const int debug = 0;  // set to 1 to turn on debugging
const int debugnav = 0; // set to 1 to turn on navigation debuging

// DEFINE BUTTONS
const int buttonOnBoard = 0;     // the number of the pushbutton pin
const int buttonUp = 14;     // the number of the pushbutton pin
const int buttonDown = 12;     // the number of the pushbutton pin
const int buttonRight = 13;     // the number of the pushbutton pin
const int buttonLeft = 27;     // the number of the pushbutton pin
const int ledPin =  5;      // the number of the LED pin

// variables will change:
int buttonUpState = 0;         // variable for reading the pushbutton status
int buttonDownState = 0;
int buttonRightState = 0;
int buttonLeftState = 0;
int buttonOnBoardState = 0;
int screenBrightness = 1;
int firstBoot = 0;
long buttonUpPressedTime;
long buttonPressTimeStamp = millis();
long buttonRoseTimeStamp = millis();

//wifi variables
int wifiStatus = 0;

// Menu Variables
//Set position variables
int currentMenuPos = 1;
int previousMenuPos = 0;
int menuPosSelected = 0;
int menuNumItems = 0;
int menuNumber = 0;
int previousMenuNumber = 0;
char menuIndicator[ ] = "*";

// Setup Debouncing for buttons
Bounce debounceButtonUp = Bounce();
Bounce debounceButtonDown = Bounce();
Bounce debounceButtonRight = Bounce();
Bounce debounceButtonLeft = Bounce();
Bounce debounceButtonOnBoard = Bounce();
int debounceInterval = 10;
bool scanning = false;

const unsigned char* imageArray[] = {SecKCSmall,SecKCFuzzy,DCP,DCXXV};
// Wifi Settings
char ssid[]="SecKC_Badge";  //SSID

void setup(){
  Serial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonUp, INPUT_PULLUP);
  debounceButtonUp.attach(buttonUp);
  debounceButtonUp.interval(debounceInterval);
  pinMode(buttonDown, INPUT_PULLUP);
  debounceButtonDown.attach(buttonDown);
  debounceButtonDown.interval(debounceInterval);
  pinMode(buttonRight, INPUT_PULLUP);
  debounceButtonRight.attach(buttonRight);
  debounceButtonRight.interval(debounceInterval);
  pinMode(buttonLeft, INPUT_PULLUP);
  debounceButtonLeft.attach(buttonLeft);
  debounceButtonLeft.interval(debounceInterval);
  pinMode(buttonOnBoard, INPUT);
  debounceButtonOnBoard.attach(buttonOnBoard);
  debounceButtonOnBoard.interval(debounceInterval);

  // Initialze SSD1306 OLED oled
  Wire.begin();
  oled.init();
  oled.setHorizontalScrollProperties(Scroll_Left,72,95,Scroll_2Frames);
  oled.clearDisplay();


  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void drawCredits(){
  oled.clearDisplay();
  oled.setTextXY(0,0);
  oled.putString ("****Credits****");
  oled.setTextXY(2,0);
  oled.setTextXY(3,0);
  oled.putString ("      fg");
  oled.setTextXY(4,0);
  oled.putString ("     rixon");
  oled.setTextXY(5,0);
  oled.putString ("  networkgeek");
  delay(4000);
  oled.clearDisplay();  // clear the display ahead of the menu
  returnToMenu(menuNumber);
}

void drawSponsors(){
  oled.clearDisplay();
  oled.setTextXY(0,0);
  oled.putString ("***************");
  oled.setTextXY(1,0);
  oled.putString ("  SecKC");
  oled.setTextXY(2,0);
  oled.putString ("Defcon 25 Badge");
  oled.setTextXY(5,0);
  oled.putString ("BadgePirates.com");
  delay(2000);
  oled.clearDisplay();  // clear the display ahead of the menu
  returnToMenu(menuNumber);
}

void drawTwitterCredits(){
  oled.clearDisplay();
  oled.setTextXY(0,0);
  oled.putString ("***************");
  oled.setTextXY(1,0);
  oled.putString ("#BadgeLife");
  oled.setTextXY(2,0);
  oled.putString ("#SecKC");
  oled.setTextXY(3,0);
  oled.putString ("#DefConParties");
  oled.setTextXY(5,0);
  oled.putString ("BadgePirates.com");
  delay(2000);
  oled.clearDisplay();  // clear the display ahead of the menu
  returnToMenu(menuNumber);
}

void drawBling(){
  int repeat = 1;
  long imageTimer = millis();
  digitalWrite(ledPin, LOW); //LED is OFF
  oled.clearDisplay();  // clear the display SecKC DefCon Bitmap
  while (repeat > 0){
    if(millis() - imageTimer <= 2000){
      oled.drawBitmap(SecKCFuzzy,1024);
    }
    if((millis() - imageTimer >= 2001) && (millis() - imageTimer <= 4000)){
      oled.drawBitmap(BobSmall,1024);
    }
    if((millis() - imageTimer >= 4001) && (millis() - imageTimer <= 6000)){
      oled.drawBitmap(DCXXV,1024);
    }
    if((millis() - imageTimer >= 6001) && (millis() - imageTimer <= 8000)){
      oled.drawBitmap(DCP,1024);
    }
    if(millis() - imageTimer >= 8001){
      imageTimer = millis();
    }
    setUpButtonTimers();
    if(checkButtonUpPressLength()){
      repeat = 0;
      menuNumber = 0;
      buttonUpPressedTime = 0;
      topMenu();
    };
  }
}

bool checkButtonUpPressLength(){
  if(buttonUpPressedTime >= 3000){return true;}
  else{return false;}
}

void returnToMenu(int menuNumber){
   // Menu 1 (Top)
   if (menuNumber == 1) {
     topMenu();
   }

  if (menuNumber == 2) {
    settingsMenu();
  }
  if (menuNumber == 3) {
    scanWifi();
  }
}

esp_err_t event_handler(void *ctx, system_event_t *event){
 if (event->event_id == SYSTEM_EVENT_SCAN_DONE) {
   //get the number of AP's found
   uint16_t apCount = event->event_info.scan_done.number;
   //setup oled for wifi menu
   oled.clearDisplay();
   currentMenuPos = 1;
   menuPosSelected = 0;
   menuNumItems = apCount;  // Allow for navigation to iterate through the correct number of positions
   menuNumber = 3;  // Each menu needs a unique ID for the cases to switch the correct function
   oled.setTextXY (0,5);
   oled.putString ("==APs==");

   wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount); //create list of type wifi_ap_record of the size of our results
   esp_wifi_scan_get_ap_records(&apCount, list);  //save the ap's to our list

   //loop of our list of ap's
   for (int i=0; i<apCount; i++) {
     String ssid = (char*)list[i].ssid; //convert ssid to string type
     if(ssid.length() > 13 ){ ssid.remove(13); } //trim anything over 13 chars for oled
     oled.setTextXY (i+1,2);  //move cursor
     oled.putString (ssid);  //write ssid
   }
   //cleanup
   oled.setTextXY (1,1);
   scanning = false;
   menuNavigation(menuNumber, menuNumItems);
 }
 return ESP_OK;
}


void topMenu(){
  oled.clearDisplay();  // Get screen ready for new menu
  oled.setTextXY (0,3);
  oled.putString ("==MENU==");
  oled.setTextXY (1,2);
  oled.putString ("1. Bling Mode");
  oled.setTextXY (2,2);
  oled.putString ("2. Credits");
  oled.setTextXY (3,2);
  oled.putString ("3. -----");
  oled.setTextXY (4,2);
  oled.putString ("4. Settings");
  oled.setTextXY (5,2);
  oled.putString ("5. Scan Wifi");
  oled.setTextXY (1,1);
  menuNumItems = 5;  // Allow for navigation to iterate through the correct number of positions
  menuNumber = 1;  // Each menu needs a unique ID for the cases to switch the correct function
  if (firstBoot == 0) {
  menuNavigation(menuNumber, menuNumItems);
  }
}

void scanWifi(){
  scanning = true;
  // Let us test a WiFi scan ...
  wifi_scan_config_t scanConf = {
  .ssid = NULL,
  .bssid = NULL,
  .channel = 0,
  .show_hidden = 1
  };
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, 0));
  oled.clearDisplay();  // Get screen ready for new menu
  oled.setTextXY (1,2);
  oled.putString ("Starting scan...");
}

void settingsMenu(){
  currentMenuPos = 1;
  menuPosSelected = 0;  // Reset flag from call to function
  oled.clearDisplay();  // Get screen ready for new menu
  oled.setTextXY (0,3);
  oled.putString ("==Settings==");
  oled.setTextXY (1,2);
  oled.putString ("1. LCD Low");
  // Set value indicator if brightness == low
  if (screenBrightness == 0) {
  oled.setTextXY (1,15);
  oled.putString ("*");
  }

  oled.setTextXY (2,2);
  oled.putString ("2. LCD High");
  // Set value indicator if brightness == high
  if (screenBrightness == 1) {
  oled.setTextXY (2,15);
  oled.putString ("*");
  }

  oled.setTextXY (3,2);
  oled.putString ("3. Wifi On");
  // Set value indicator if WIFI == high
  if (wifiStatus == 1) {
  oled.setTextXY (3,15);
  oled.putString ("*");
  }

  oled.setTextXY (4,2);
  oled.putString ("4. Wifi Off");
  // Set value indicator if brightness == low
  if (wifiStatus == 0) {
  oled.setTextXY (4,15);
  oled.putString ("*");
  }

  oled.setTextXY (1,1);
  menuNumItems = 4;  // Allow for navigation to iterate through the correct number of positions
  menuNumber = 2;  // Each menu needs a unique ID for the cases to switch the correct function
  menuNavigation(menuNumber, menuNumItems);
}


void menuNavigation(int menuNumber, int menuNumItems){
  // topMenu Navigation
  if (menuNumber == 1 && menuPosSelected == 1 && currentMenuPos == 1){
    drawBling();
  }

  if (menuNumber == 1 && menuPosSelected == 1 && currentMenuPos == 2){
    drawCredits();
  }

  if (menuNumber == 1 && menuPosSelected == 1 && currentMenuPos == 3){
    drawTwitterCredits();
  }

  if (menuNumber == 1 && menuPosSelected == 1 && currentMenuPos == 4){
    settingsMenu();
  }


  if (menuNumber == 1 && menuPosSelected == 1 && currentMenuPos == 5){
    scanWifi();
  }

  // Settings Navigation
  if (menuNumber == 2 && menuPosSelected == 1  && currentMenuPos == 1){
    oledBrightnessLow();
  }

  if (menuNumber == 2 && menuPosSelected == 1  && currentMenuPos == 2){
     oledBrightnessHigh();
  }

  if (menuNumber == 2 && menuPosSelected == 1  && currentMenuPos == 3){
    wifiOn();
  }

  if (menuNumber == 2 && menuPosSelected == 1  && currentMenuPos == 4){
    wifiOff();
  }

  // Track the Menu positions and update the cursor
  if (debugnav == 1){
    Serial.print ("Current Menu Position: ");
    Serial.println (currentMenuPos);
    Serial.print ("Previous Menu Position: ");
    Serial.println (previousMenuPos);
  }

  if (debounceButtonDown.fell() ){
    previousMenuPos = currentMenuPos;
    currentMenuPos++;
  }


  if (debounceButtonUp.fell() ){
      previousMenuPos = currentMenuPos;
      if (currentMenuPos > 1){
      currentMenuPos--;
    }
  }

  if(debounceButtonLeft.fell() ){
    topMenu();
  }

  if(scanning == false){
    // Clean up for button bouncing
    if (currentMenuPos > menuNumItems){
      currentMenuPos = 1;
      previousMenuPos = menuNumItems;
      oled.setTextXY (menuNumItems,1);
      oled.putString (" ");
    }

    // Remove indicator from previous location
    oled.setTextXY (previousMenuPos,1);
    oled.putString (" ");
    // Update Menu indicator
    oled.setTextXY (currentMenuPos,1);
    oled.putString (menuIndicator);
  }
  menuPosSelected = 0;  // reset the menu selection
}

void wifiOn(){
  // start broadcsting SSID AP On
  oled.clearDisplay ();
  oled.setTextXY (1,2);
  oled.putString ("Wifi ON");
  oled.setTextXY (2,2);
  oled.putString (ssid);
  WiFi.softAP(ssid, NULL, 1, 0);
  wifiStatus = 1;
  delay (2000);
  settingsMenu();
}

void wifiOff (){
  // stop broadcating SSID (AP off)
  oled.clearDisplay ();
  oled.setTextXY (1,2);
  oled.putString ("Wifi OFF");
  WiFi.softAPdisconnect(1);
  wifiStatus = 0;
  delay (2000);
  settingsMenu();
}

void oledBrightnessLow ()
{
  oled.setBrightness(0);
  Serial.println ("Brightness set to 0");
  Serial.print ("Menu Select flag: ");
  Serial.println (menuPosSelected);
  screenBrightness = 0;
  returnToMenu(menuNumber);
}

void oledBrightnessHigh()
{
  oled.setBrightness(255);
  Serial.println ("Brightness set to 255");
  Serial.print ("Menu Select flag: ");
  Serial.println (menuPosSelected);
  screenBrightness = 1;
  returnToMenu(menuNumber);
}

void setUpButtonTimers(){
  debounceButtonUp.update();
  buttonUpState = debounceButtonUp.read();
  if ( debounceButtonUp.rose()  ){
   buttonRoseTimeStamp = millis();
   buttonUpPressedTime = buttonRoseTimeStamp - buttonPressTimeStamp;
  }

  if ( debounceButtonUp.fell()  ){
   buttonPressTimeStamp = millis();
  }
}

void loop(){

  debounceButtonUp.update();
  debounceButtonDown.update();
  debounceButtonRight.update();
  debounceButtonLeft.update();
  debounceButtonOnBoard.update();

  buttonUpState = debounceButtonUp.read();
  buttonDownState = debounceButtonDown.read();
  buttonRightState = debounceButtonRight.read();
  buttonLeftState = debounceButtonLeft.read();
  buttonOnBoardState = debounceButtonOnBoard.read();

  if (debug == 1){
    Serial.println ("Button Status");
    Serial.println ("");
    Serial.print ("Up:");
    Serial.println (buttonUpState);
    Serial.print ("Down: ");
    Serial.println (buttonDownState);
    Serial.print ("Right: ");
    Serial.println (buttonRightState);
    Serial.print ("Left: ");
    Serial.println (buttonLeftState);
  }

  // Set select variable and pass it into the menu
  if (debounceButtonRight.fell() ){
    menuPosSelected = 1;
  }

  // Call the first menu
  if (firstBoot == 0){
    topMenu();
    firstBoot++;
  }
  menuNavigation(menuNumber, menuNumItems);
}
