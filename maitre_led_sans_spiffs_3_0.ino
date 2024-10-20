#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Nextion.h"
#include "Adafruit_SHT4x.h"
#include <Adafruit_SCD30.h>
#include <Adafruit_MCP23X17.h>
#include "RTClib.h"
#include "ThingSpeak.h"
//#include "SPIFFS.h"//
#include "ESP32_NOW.h"
#include <esp_mac.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <ElegantOTA.h>

#define mcp_LIGHT 13
#define mcp_HYDRO 1
#define mcp_CYCLE 2
#define mcp_CHAUFF 14
#define mcp_EXV1 12
#define mcp_EXV2 11
#define mcp_EXV3 10
#define mcp_EXV4 9
#define mcp_EXV5 8
#define mcp_INV1 7
#define mcp_INV2 6
#define mcp_INV3 5
#define mcp_INV4 4
#define mcp_INV5 3
#define PIC_EXV0 50
#define PIC_EXV1 51
#define PIC_EXV2 52
#define PIC_EXV3 53
#define PIC_EXV4 54
#define PIC_EXV5 55
#define PIC_INV0 56
#define PIC_INV1 57
#define PIC_INV2 58
#define PIC_INV3 59
#define PIC_INV4 60
#define PIC_INV5 61

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_SCD30 scd30;
Adafruit_MCP23X17 mcp;
RTC_DS3231 rtc;
DateTime now;
WiFiClient client;
Preferences preferences;
AsyncWebServer server(80);
//AsyncWebSocket ws("/ws");//
#define ESPNOW_WIFI_CHANNEL 6

/***************************************
    Nextion component for page: HOME
***************************************/
NexText date_home = NexText(0, 63, "date_home");
NexText heure_home = NexText(0, 64, "heure_home");
NexText temperature = NexText(0, 1, "temperature");
NexText hydrometrie = NexText(0, 3, "hydrometrie");
NexText tmax = NexText(0, 8, "tmax");
NexText tmin = NexText(0, 10, "tmin");
NexText hmax = NexText(0, 12, "hmax");
NexText hmin = NexText(0, 14, "hmin");
NexWaveform s0 = NexWaveform(0, 2, "s0");
NexWaveform s1 = NexWaveform(0, 33, "s1");
NexWaveform s2 = NexWaveform(0, 45, "s2");
NexPicture home_extra = NexPicture(0, 28, "extra");
NexText home_extra1 = NexText(0, 30, "extra1");
NexPicture home_intra = NexPicture(0, 29, "intra");
NexText home_intra1 = NexText(0, 31, "intra1");
NexPicture home_lightpic = NexPicture(0, 19, "p2");
NexPicture home_hydropic = NexPicture(0, 20, "p3");
NexPicture home_cyclepic = NexPicture(0, 21, "p6");
NexPicture home_chauffpic = NexPicture(0, 22, "p7");
NexText home_nbrejour = NexText(0, 58, "nbrejour");
NexButton settime = NexButton(0, 15, "b6");
NexNumber home_led = NexNumber(0, 78, "n0");
NexNumber tempe = NexNumber(0, 59, "temp");
NexNumber hysteresis = NexNumber(0, 60, "hysterisis");
NexNumber hydrom = NexNumber(0, 61, "hydro");
NexNumber CO22 = NexNumber(0, 62, "co2");
NexSlider h0 = NexSlider(0, 32, "h0");
/****************************************
    Nextion component for page: LIGHT
****************************************/
NexButton light_back_to_set = NexButton(1, 2, "backhome");
NexDSButton manulight = NexDSButton(1, 14, "manul");
NexDSButton autolight = NexDSButton(1, 15, "autol");
NexNumber setlight_hour_on = NexNumber(1, 10, "hour11");
NexNumber setlight_hour_off = NexNumber(1, 11, "hour12");
NexNumber setlight_min_on = NexNumber(1, 12, "min11");
NexNumber setlight_min_off = NexNumber(1, 13, "min12");
NexNumber setlight_led = NexNumber(1, 16, "led");
/***************************************
    Nextion component for page: TEMP
***************************************/
NexButton temp_back_to_set = NexButton(2, 2, "backhome");
NexNumber settemp_tempjour = NexNumber(2, 18, "tempjour");
NexNumber settemp_hystjour = NexNumber(2, 19, "hystjour");
NexNumber settemp_tempnuit = NexNumber(2, 20, "tempnuit");
NexNumber settemp_hystnuit = NexNumber(2, 21, "hystnuit");
/****************************************
    Nextion component for page: HYDRO
****************************************/
NexButton hydro_back_to_set = NexButton(3, 2, "backhome");
NexDSButton manuhydro = NexDSButton(3, 24, "manuh");
NexDSButton autohydro = NexDSButton(3, 25, "autoh");
NexNumber sethydro_hydromaxjour = NexNumber(3, 20, "hydromaxjour");
NexNumber sethydro_hydrominjour = NexNumber(3, 21, "hydrominjour");
NexNumber sethydro_hydromaxnuit = NexNumber(3, 22, "hydromaxnuit");
NexNumber sethydro_hydrominnuit = NexNumber(3, 23, "hydrominnuit");
/*****************************************
    Nextion component for page: MOTEUR
*****************************************/
NexButton moteur_back_to_set = NexButton(4, 2, "backhome");
NexDSButton setmoteur_extrajouroff = NexDSButton(4, 4, "extrajouroff");
NexDSButton setmoteur_extrajouron = NexDSButton(4, 6, "extrajouron");
NexDSButton setmoteur_extrajourtemp = NexDSButton(4, 8, "extrajourtemp");
NexDSButton setmoteur_extrajourhydro = NexDSButton(4, 10, "extrajourhydro");
NexDSButton setmoteur_extranuitoff = NexDSButton(4, 5, "extranuitoff");
NexDSButton setmoteur_extranuiton = NexDSButton(4, 7, "extranuiton");
NexDSButton setmoteur_extranuittemp = NexDSButton(4, 9, "extranuittemp");
NexDSButton setmoteur_extranuithydro = NexDSButton(4, 11, "extranuithydro");
NexNumber setmoteur_extrajour = NexNumber(4, 18, "extra_jour");
NexNumber setmoteur_extranuit = NexNumber(4, 19, "extra_nuit");
NexNumber setmoteur_intrajour = NexNumber(4, 20, "intra_jour");
NexNumber setmoteur_intranuit = NexNumber(4, 21, "intra_nuit");
/****************************************
    Nextion component for page: CYCLE
****************************************/
NexButton cycle_back_to_set = NexButton(5, 2, "backhome");
NexDSButton manucycle = NexDSButton(5, 16, "manuc");
NexDSButton autocycle = NexDSButton(5, 17, "autoc");
NexNumber setcycle_jouron = NexNumber(5, 12, "jouron");
NexNumber setcycle_jouroff = NexNumber(5, 13, "jouroff");
NexNumber setcycle_nuiton = NexNumber(5, 14, "nuiton");
NexNumber setcycle_nuitoff = NexNumber(5, 15, "nuitoff");
/********************************************
    Nextion component for page: CHAUFFAGE
********************************************/
NexButton chauffage_back_to_set = NexButton(6, 2, "backhome");
NexDSButton manuchauff = NexDSButton(6, 14, "manuch");
NexDSButton autochauff = NexDSButton(6, 15, "autoch");
NexNumber setchauffmax = NexNumber(6, 12, "chauffmax");
NexNumber setchauffmin = NexNumber(6, 13, "chauffmin");
/***************************************
    Nextion component for page: TIME
***************************************/
NexButton backhome = NexButton(7, 2, "backhome");
NexButton settime_ok = NexButton(7, 4, "settimeok");
NexNumber sethour = NexNumber(7, 16, "sethour1");
NexNumber setmin = NexNumber(7, 17, "setmin1");
NexNumber setday = NexNumber(7, 13, "setday1");
NexNumber setmon = NexNumber(7, 14, "setmon1");
NexNumber setyear = NexNumber(7, 15, "setyear2");
/*********************************
    Nextion component all PAGE
*********************************/
NexPage page0 = NexPage(0, 0, "home");
NexPage page1 = NexPage(1, 0, "light");
NexPage page2 = NexPage(2, 0, "temps");
NexPage page3 = NexPage(3, 0, "hydro");
NexPage page4 = NexPage(4, 0, "moteur");
NexPage page5 = NexPage(5, 0, "cycle");
NexPage page6 = NexPage(6, 0, "chauffage");
NexPage page7 = NexPage(7, 0, "time");

NexTouch *nex_Listen_List[] = {
  &temperature, &hydrometrie, &home_lightpic, &home_hydropic, &tmax, &tmin,
  &hmax, &hmin, &home_extra, &home_extra1, &home_intra, &home_intra1,
  &home_cyclepic, &home_chauffpic, &home_nbrejour, &settime, &h0,

  &manulight, &autolight, &setlight_hour_on, &setlight_min_off,
  &setlight_hour_off, &setlight_min_on, &light_back_to_set, &setlight_led,

  &settemp_tempjour, &settemp_tempnuit, &settemp_hystjour,
  &settemp_hystnuit, &temp_back_to_set,

  &manuhydro, &autohydro, &sethydro_hydromaxjour, &sethydro_hydrominjour,
  &sethydro_hydromaxnuit, &sethydro_hydrominnuit, &hydrom, &hydro_back_to_set,

  &setmoteur_extrajouron, &setmoteur_extrajouroff, &setmoteur_extrajourtemp,
  &setmoteur_extrajourhydro, &setmoteur_extrajour, &setmoteur_extranuiton,
  &setmoteur_extranuitoff, &setmoteur_extranuittemp, &setmoteur_extranuithydro,
  &setmoteur_extranuit, &setmoteur_intrajour, &setmoteur_intranuit, &moteur_back_to_set,

  &manucycle, &autocycle, &setcycle_jouron, &setcycle_jouroff, &setcycle_nuiton,
  &setcycle_nuitoff, &cycle_back_to_set,

  &manuchauff, &autochauff, &chauffage_back_to_set, &setchauffmax, &setchauffmin,

  &sethour, &setmin, &setday, &setmon, &setyear, &settime_ok, &backhome,
  NULL
};
/***********************
    partial variable
***********************/
const int intervalGraph = 18000;
unsigned long previousGraph = 0;
const int intervalEcran = 8000;
unsigned long previousEcran = 0;
unsigned long ota_progress_millis = 0;
int previousDay = -1;
int previousCycle = -1;
int previousWifi = -1;
uint8_t cptejour = 0;
unsigned long delta = 0;
float t = 0.0;
float h = 0.0;
float sht = 0.0;
float CO2 = 0;
char tbuffer[10];
char hbuffer[10];
char sht31buffer[10];
char CO2buffer[10];
float previous_t = 0.0;
float previous_h = 0.0;
float previous_CO2 = 0;
float tempmin = 100;
float tempmax = 0;
float hydromin = 100;
float hydromax = 0;
char tempminbuffer[10];
char tempmaxbuffer[10];
char hydrominbuffer[10];
char hydromaxbuffer[10];
uint8_t currentPage = 0;
uint32_t lumPrev = 9;
uint32_t icolum = 9;
uint32_t icoled = 9;
uint32_t hydroPrev = 14;
uint32_t icohydro = 14;
uint32_t home_extra11Prev = 0;
uint32_t home_intra11Prev = 0;
uint32_t home_extra11 = 0;
uint32_t home_intra11 = 0;
uint32_t icoextra = 50;
uint32_t icoextraPrev = 50;
uint32_t icointra = 50;
uint32_t icointraPrev = 50;
uint32_t cyclePrev = 20;
uint32_t icocycle = 20;
uint32_t chauffPrev = 25;
uint32_t icochauff = 25;
uint32_t tempePrev = 0;
uint32_t hysteresisPrev = 0;
uint32_t hydromPrev = 0;
char cptebuffer[10];
// REMPLACEZ PAR VOTRE Adresse MAC DU RECEVEUR
// uint8_t myMacAddress[] = { 0x84, 0x0D, 0x8E, 0xE6, 0x59, 0x08 };

//esp_now_peer_info_t peerInfo;
/****************************
    TIME partial variable
****************************/
char daysOfTheMonth[13][10] = {
  "", "janv", "fev", "mars", "avril", "mai",
  "juin", "juil", "aout", "sept", "oct", "nov", "dec"
};
uint32_t number_settime_year = 2022;
uint32_t number_settime_month = 7;
uint32_t number_settime_day = 11;
uint32_t number_settime_hour = 22;
uint32_t number_settime_minute = 0;
uint32_t previousSecond = -1;
/*****************************
    LIGHT partial variable
*****************************/
bool light = 0;
bool ledState1 = 0;
bool ledState2 = 0;
uint32_t light_Manual = 0;
uint32_t light_Timer = 0;
uint32_t number_setlight_hour_on = 19;
uint32_t number_setlight_min_on = 0;
uint32_t number_setlight_hour_off = 13;
uint32_t number_setlight_min_off = 0;
uint32_t valueLed = 100;
int request_value = 1;  // Ajout de la déclaration de la variable request_value
bool isConnected = false;
bool sendRequested = false;   // Variable pour indiquer si l'esclave a demandé l'envoi
bool sentValueStock = false;  // Variable pour indiquer si la valeur valueStock a été envoyée
/****************************
    TEMP partial variable
****************************/
uint32_t number_settemp_tempjour = 26;
uint32_t number_settemp_hystjour = 2;
uint32_t number_settemp_tempnuit = 22;
uint32_t number_settemp_hystnuit = 2;
/****************************
    HYDRO partial variable
****************************/
bool hydro = 0;
bool ledState3 = 0;
bool ledState4 = 0;
uint32_t hydro_Manual = 0;
uint32_t hydro_Timer = 0;
uint32_t number_sethydro_hydromaxjour = 57;
uint32_t number_sethydro_hydrominjour = 55;
uint32_t number_sethydro_hydromaxnuit = 57;
uint32_t number_sethydro_hydrominnuit = 55;
/******************************
    MOTEUR partial variable
******************************/
uint32_t moteurExtrajouroff = 0;
uint32_t moteurExtrajouron = 0;
uint32_t moteurExtrajourtemp = 0;
uint32_t moteurExtrajourhydro = 0;
uint32_t moteurExtranuitoff = 0;
uint32_t moteurExtranuiton = 0;
uint32_t moteurExtranuittemp = 0;
uint32_t moteurExtranuithydro = 0;
uint32_t number_setmoteur_extrajour = 4;
uint32_t number_setmoteur_intrajour = 3;
uint32_t number_setmoteur_extranuit = 3;
uint32_t number_setmoteur_intranuit = 2;
int16_t previousSpeedEx = 0;
uint8_t speedEx[6];
uint8_t icoEx[6];
int16_t previousSpeedIn = 0;
uint8_t speedIn[6];
uint8_t icoIn[6];
unsigned long previousMotExj = 0;
unsigned long previousMotInj = 0;
unsigned long previousMotExn = 0;
unsigned long previousMotInn = 0;
/*****************************
    CYCLE partial variable
*****************************/
bool cycle = 0;
bool ledState = 0;
bool ledState9 = 0;
bool ledState10 = 0;
uint32_t cycle_Manual = 0;
uint32_t cycle_Timer = 0;
uint32_t number_setcycle_jouron = 60;
uint32_t number_setcycle_jouroff = 0;
uint32_t number_setcycle_nuiton = 3;
uint32_t number_setcycle_nuitoff = 1;
/*********************************
    CHAUFFAGE partial variable
*********************************/
bool chauff = 0;
bool ledState11 = 0;
bool ledState12 = 0;
uint32_t chauff_Manual = 0;
uint32_t chauff_Timer = 0;
uint32_t number_setchauffmax = 20;
uint32_t number_setchauffmin = 18;
/****************************
    WIFI partial variable
****************************/
const char *ssid = "********";
const char *pass = "********";
const char *soft_ap_ssid = "ESP32_BOITIER";   //--> access point name
const char *soft_ap_password = "helloesp32";  //--> access point password
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned long myChannelNumber = ******;          
const char *myWriteAPIKey = "*************";
bool moteur = 0;
/*************
    CORE 1
*************/
TaskHandle_t Task1;
void Core1(void *pvParameters) {
  for (;;) {
    nexLoop(nex_Listen_List);
    delay(10);
  }
}
/****************
    HTML WIFI
****************/
/*void notifyClients() {
  if (ledState1 == 1) {
    light = 1;
    icolum = 11;
    light_Manual = 1;
    ledState1 = 1;
    preferences.putUInt("lightmanual", light_Manual);
    preferences.putBool("ledState1", ledState1);
    ws.textAll(String(ledState1) + "led1");
  } else {
    light = 0;
    icolum = 9;
    light_Manual = 0;
    ledState1 = 0;
    preferences.putUInt("lightmanual", light_Manual);
    preferences.putBool("ledState1", ledState1);
    ws.textAll(String(ledState1) + "led1");
  }
  if (ledState2 == 1) {
    light_Timer = 0;
    ledState2 = 0;
    preferences.putUInt("lighttimer", light_Timer);
    preferences.putBool("ledState2", ledState2);
    ws.textAll(String(ledState2) + "led2");
  }
}

void notifyClients2() {
  if (ledState2 == 1) {
    icolum = 12;
    light_Timer = 1;
    ledState2 = 1;
    preferences.putUInt("lighttimer", light_Timer);
    preferences.putBool("ledState2", ledState2);
    ws.textAll(String(ledState2) + "led2");
  } else {
    light = 0;
    icolum = 9;
    light_Timer = 0;
    ledState2 = 0;
    preferences.putUInt("lighttimer", light_Timer);
    preferences.putBool("ledState2", ledState2);
    ws.textAll(String(ledState2) + "led2");
  }
  if (ledState1 == 1) {
    light_Timer = 0;
    ledState2 = 0;
    preferences.putUInt("lighttimer", light_Timer);
    preferences.putBool("ledState2", ledState2);
    ws.textAll(String(ledState2) + "led2");
  }
}

void broadcastBrightness(int brightnessValue) {
  String message = "BRIGHTNESS:" + String(brightnessValue);
  ws.textAll(message);
}

void notifyClients3() {
  if (ledState3 == 1) {
    hydro = 1;
    icohydro = 16;
    hydro_Manual = 1;
    ledState3 = 1;
    preferences.putUInt("hydromanual", hydro_Manual);
    preferences.putBool("ledState3", ledState3);
    ws.textAll(String(ledState3) + "led3");
  } else {
    hydro = 0;
    icohydro = 14;
    hydro_Manual = 0;
    ledState3 = 0;
    preferences.putUInt("hydromanual", hydro_Manual);
    preferences.putBool("ledState3", ledState3);
    ws.textAll(String(ledState3) + "led3");
  }
  if (ledState4 == 1) {
    hydro_Timer = 0;
    ledState4 = 0;
    preferences.putUInt("hydrotimer", hydro_Timer);
    preferences.putBool("ledState4", ledState4);
    ws.textAll(String(ledState4) + "led4");
  }
}

void notifyClients4() {
  if (ledState4 == 1) {
    icohydro = 17;
    hydro_Timer = 1;
    ledState4 = 1;
    preferences.putUInt("hydrotimer", hydro_Timer);
    preferences.putBool("ledState4", ledState4);
    ws.textAll(String(ledState4) + "led4");
  } else {
    hydro = 0;
    icohydro = 14;
    hydro_Timer = 0;
    ledState4 = 0;
    preferences.putUInt("hydrotimer", hydro_Timer);
    preferences.putBool("ledState4", ledState4);
    ws.textAll(String(ledState4) + "led4");
  }
  if (ledState3 == 1) {
    hydro_Timer = 0;
    ledState4 = 0;
    preferences.putUInt("hydrotimer", hydro_Timer);
    preferences.putBool("ledState4", ledState4);
    ws.textAll(String(ledState4) + "led4");
  }
}

void notifyClients9() {
  if (ledState9 == 1) {
    cycle = 1;
    icocycle = 22;
    cycle_Manual = 1;
    ledState9 = 1;
    preferences.putUInt("cyclemanual", cycle_Manual);
    preferences.putBool("ledState9", ledState9);
    ws.textAll(String(ledState9) + "led9");
  } else {
    cycle = 0;
    icocycle = 20;
    cycle_Manual = 0;
    ledState9 = 0;
    preferences.putUInt("cyclemanual", cycle_Manual);
    preferences.putBool("ledState9", ledState9);
    ws.textAll(String(ledState9) + "led9");
  }
  if (ledState10 == 1) {
    cycle_Timer = 0;
    ledState10 = 0;
    preferences.putUInt("cycletimer", cycle_Timer);
    preferences.putBool("ledState10", ledState10);
    ws.textAll(String(ledState10) + "led10");
  }
}

void notifyClients10() {
  if (ledState10 == 1) {
    icocycle = 23;
    cycle_Timer = 1;
    ledState10 = 1;
    preferences.putUInt("cycletimer", cycle_Timer);
    preferences.putBool("ledState10", ledState10);
    ws.textAll(String(ledState10) + "led10");
  } else {
    cycle = 0;
    icocycle = 20;
    cycle_Timer = 0;
    ledState10 = 0;
    preferences.putUInt("cycletimer", cycle_Timer);
    preferences.putBool("ledState10", ledState10);
    ws.textAll(String(ledState10) + "led10");
  }
  if (ledState9 == 1) {
    cycle_Timer = 0;
    ledState10 = 0;
    preferences.putUInt("cycletimer", cycle_Timer);
    preferences.putBool("ledState10", ledState10);
    ws.textAll(String(ledState10) + "led10");
  }
}

void notifyClients11() {
  if (ledState11 == 1) {
    chauff = 1;
    icochauff = 27;
    chauff_Manual = 1;
    ledState11 = 1;
    preferences.putUInt("chauffmanual", chauff_Manual);
    preferences.putBool("ledState11", ledState11);
    ws.textAll(String(ledState11) + "led11");
  } else {
    chauff = 0;
    icochauff = 25;
    chauff_Manual = 0;
    ledState11 = 0;
    preferences.putUInt("chauffmanual", chauff_Manual);
    preferences.putBool("ledState11", ledState11);
    ws.textAll(String(ledState11) + "led11");
  }
  if (ledState12 == 1) {
    chauff_Timer = 0;
    ledState12 = 0;
    preferences.putUInt("chaufftimer", chauff_Timer);
    preferences.putBool("ledState12", ledState12);
    ws.textAll(String(ledState12) + "led12");
  }
}

void notifyClients12() {
  if (ledState12 == 1) {
    icochauff = 28;
    chauff_Timer = 1;
    ledState12 = 1;
    preferences.putUInt("chaufftimer", chauff_Timer);
    preferences.putBool("ledState12", ledState12);
    ws.textAll(String(ledState12) + "led12");
  } else {
    chauff = 0;
    icochauff = 25;
    chauff_Timer = 0;
    ledState12 = 0;
    preferences.putUInt("chaufftimer", chauff_Timer);
    preferences.putBool("ledState12", ledState12);
    ws.textAll(String(ledState12) + "led12");
  }
  if (ledState11 == 1) {
    chauff_Timer = 0;
    ledState12 = 0;
    preferences.putUInt("chaufftimer", chauff_Timer);
    preferences.putBool("ledState12", ledState12);
    ws.textAll(String(ledState12) + "led12");
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->opcode == WS_TEXT) {
    String command = String((char *)data);
    // Serial.println("command WebSocket reçu : " + command);  // Ajoutez cette ligne

    if (command.startsWith("MOTOR_SPEED:")) {
      String motorSpeedCommand = command.substring(12);     // Extrait la commande complète après "MOTOR_SPEED:"
      String buttonId = motorSpeedCommand.substring(0, 7);  // Extrait le bouton ID (par exemple, "button5")
      String speedValue = motorSpeedCommand.substring(7);   // Extrait la valeur de vitesse
      // Mettez à jour la valeur de vitesse pour le bouton ID5 en temps réel
      if (buttonId.equals("button5")) {
        number_setmoteur_extrajour = speedValue.toInt();
        // Envoie la nouvelle valeur de vitesse à tous les clients WebSocket
        String message = "MOTOR_SPEED:" + speedValue;
        ws.textAll(message);
        // Affichez la valeur de vitesse reçue dans le moniteur série
        Serial.println("Valeur setmoteur_extrajour reçue : " + message);
      }
    }*/
/*if (command.startsWith("setmoteur_intrajour:")) {
      number_setmoteur_intrajour = command.substring(21).toInt();                                    // Mettez à jour la valeur
      Serial.println("Valeur setmoteur_intrajour envoyer : " + String(number_setmoteur_intrajour));  // Ajoutez cette ligne
      String message = "MOTOR_SPEED:" + String(number_setmoteur_intrajour);
      ws.textAll(message);
    }
    if (command.startsWith("setmoteur_extranuit:")) {
      number_setmoteur_extranuit = command.substring(21).toInt();                                    // Mettez à jour la valeur
      Serial.println("Valeur setmoteur_extranuit envoyer : " + String(number_setmoteur_extranuit));  // Ajoutez cette ligne
      String message = "MOTOR_SPEED:" + String(number_setmoteur_extranuit);
      ws.textAll(message);
    }
    if (command.startsWith("setmoteur_intranuit:")) {
      number_setmoteur_intranuit = command.substring(21).toInt();                                    // Mettez à jour la valeur
      Serial.println("Valeur setmoteur_intranuit envoyer : " + String(number_setmoteur_intranuit));  // Ajoutez cette ligne
      String message = "MOTOR_SPEED:" + String(number_setmoteur_intranuit);
      ws.textAll(message);
    }*/
/*if (command.startsWith("brightness:")) {
      int brightnessValue = command.substring(11).toInt();
      // Mettez à jour la valeur de luminosité
      valueLed = brightnessValue;

      // Envoie la nouvelle valeur de luminosité aux clients WebSocket
      String brightnessUpdate = "BRIGHTNESS:" + String(valueLed);
      ws.textAll(brightnessUpdate);
      esp_err_t result = esp_now_send(myMacAddress, (uint8_t *)&valueLed, sizeof(valueLed));

      // Affichez la valeur de luminosité reçue dans le moniteur série
      Serial.print("Luminosité reçu : ");
      Serial.println(valueLed);
    }
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;
      if (strcmp((char *)data, "toggle") == 0) {
        ledState1 = !ledState1;
        notifyClients();
      }
      if (strcmp((char *)data, "toggle2") == 0) {
        ledState2 = !ledState2;
        notifyClients2();
      }
      if (strcmp((char *)data, "toggle3") == 0) {
        ledState3 = !ledState3;
        notifyClients3();
      }
      if (strcmp((char *)data, "toggle4") == 0) {
        ledState4 = !ledState4;
        notifyClients4();
      }
      if (strcmp((char *)data, "toggle9") == 0) {
        ledState9 = !ledState9;
        notifyClients9();
      }
      if (strcmp((char *)data, "toggle10") == 0) {
        ledState10 = !ledState10;
        notifyClients10();
      }
      if (strcmp((char *)data, "toggle11") == 0) {
        ledState11 = !ledState11;
        notifyClients11();
      }
      if (strcmp((char *)data, "toggle12") == 0) {
        ledState12 = !ledState12;
        notifyClients12();
      }
    }
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.println("Client WebSocket connecté.");
      break;
    case WS_EVT_DISCONNECT:
      Serial.println("Client WebSocket déconnecté.");
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  String currentValueMessage;  // Déplacez la déclaration ici

  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // Envoyer la valeur actuelle de la luminosité au client WebSocket
      currentValueMessage = "BRIGHTNESS:" + String(valueLed);
      server->text(client->id(), currentValueMessage);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      {
        String message = (char *)data;
        if (message == "getInitialValue") {
          // Envoyer la valeur actuelle de la luminosité au client WebSocket
          String currentValueMessage = "BRIGHTNESS:" + String(valueLed);
          server->textAll(currentValueMessage);
        }
        handleWebSocketMessage(arg, data, len);
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE1") {
    if (ledState1) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE2") {
    if (ledState2) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE3") {
    if (ledState3) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE4") {
    if (ledState4) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE5") {
    if (number_setmoteur_extrajour == 0) {
      return "0";
    } else if (number_setmoteur_extrajour == 1) {
      return "1";
    } else if (number_setmoteur_extrajour == 2) {
      return "2";
    } else if (number_setmoteur_extrajour == 3) {
      return "3";
    } else if (number_setmoteur_extrajour == 4) {
      return "4";
    } else if (number_setmoteur_extrajour == 5) {
      return "5";
    }
  }
  if (var == "STATE6") {
    if (number_setmoteur_intrajour == 0) {
      return "0";
    } else if (number_setmoteur_intrajour == 1) {
      return "1";
    } else if (number_setmoteur_intrajour == 2) {
      return "2";
    } else if (number_setmoteur_intrajour == 3) {
      return "3";
    } else if (number_setmoteur_intrajour == 4) {
      return "4";
    } else if (number_setmoteur_intrajour == 5) {
      return "5";
    }
  }
  if (var == "STATE7") {
    if (number_setmoteur_extranuit == 0) {
      return "0";
    } else if (number_setmoteur_extranuit == 1) {
      return "1";
    } else if (number_setmoteur_extranuit == 2) {
      return "2";
    } else if (number_setmoteur_extranuit == 3) {
      return "3";
    } else if (number_setmoteur_extranuit == 4) {
      return "4";
    } else if (number_setmoteur_extranuit == 5) {
      return "5";
    }
  }
  if (var == "STATE8") {
    if (number_setmoteur_intranuit == 0) {
      return "0";
    } else if (number_setmoteur_intranuit == 1) {
      return "1";
    } else if (number_setmoteur_intranuit == 2) {
      return "2";
    } else if (number_setmoteur_intranuit == 3) {
      return "3";
    } else if (number_setmoteur_intranuit == 4) {
      return "4";
    } else if (number_setmoteur_intranuit == 5) {
      return "5";
    }
  }
  if (var == "STATE9") {
    if (ledState9) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE10") {
    if (ledState10) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE11") {
    if (ledState11) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "STATE12") {
    if (ledState12) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  if (var == "BRIGHTNESS") {
    return String(valueLed);
  }
  return String();
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
*/
// Fonction de rappel lorsqu'une donnée est envoyée
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Statut d'envoi du dernier paquet : Succès de la livraison");
    Serial.println("Envoyé avec succès");
  } else {
    Serial.println("Statut d'envoi du dernier paquet : Échec de la livraison");
  }
}

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (data_len == sizeof(request_value) && memcmp(data, &request_value, data_len) == 0) {
    sentValueStock = false;  // Réinitialiser la marque de valeur envoyée
    sendRequested = true;    // Activer la demande d'envoi
  }
}

/* Classes */

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk)
    : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

/*************************
    AFFICHAGE 1ER PAGE
*************************/
void displayText(NexText *ptr, String value) {
  char tempBuf[50] = { 0 };
  int lenght = value.length();
  value.toCharArray(tempBuf, lenght + 1);
  ptr->setText(tempBuf);
}
String itos(long value, int8_t nbDigits) {
  String returnVal = "0000000000" + String(value);
  return returnVal.substring(returnVal.length() - nbDigits, returnVal.length());
}
String createClockString(void) {
  return itos(now.day(), 2) + "-" + (daysOfTheMonth[now.month()]) + "-" + itos(now.year(), 4);
}
String createClockString2(void) {
  return itos(now.hour(), 2) + ":" + itos(now.minute(), 2) + ":" + itos(now.second(), 2);
}

void pageprinc() {
  page0.show();
  currentPage = 0;

  String tempString = createClockString();
  displayText(&date_home, tempString);
  String tempString2 = createClockString2();
  displayText(&heure_home, tempString2);
  delay(10);

  if (light == 1) {
    if (t > number_settemp_tempjour) {
      dtostrf(t, 4, 1, tbuffer);
      temperature.Set_font_color_pco(63488);
      temperature.setText(tbuffer);
    } else {
      dtostrf(t, 4, 1, tbuffer);
      temperature.Set_font_color_pco(2016);
      temperature.setText(tbuffer);
    }
    if (h > number_sethydro_hydromaxjour || h < number_sethydro_hydrominjour) {
      dtostrf(h, 4, 1, hbuffer);
      hydrometrie.Set_font_color_pco(63488);
      hydrometrie.setText(hbuffer);
    } else {
      dtostrf(h, 4, 1, hbuffer);
      hydrometrie.Set_font_color_pco(2016);
      hydrometrie.setText(hbuffer);
    }
    tempe.setValue(number_settemp_tempjour);
    hysteresis.setValue(number_settemp_hystjour);
    hydrom.setValue(number_sethydro_hydromaxjour);
  }

  else {
    if (t > number_settemp_tempnuit) {
      dtostrf(t, 4, 1, tbuffer);
      temperature.Set_font_color_pco(63488);
      temperature.setText(tbuffer);
    } else {
      dtostrf(t, 4, 1, tbuffer);
      temperature.Set_font_color_pco(2016);
      temperature.setText(tbuffer);
    }
    if (h > number_sethydro_hydromaxnuit || h < number_sethydro_hydrominnuit) {
      dtostrf(h, 4, 1, hbuffer);
      hydrometrie.Set_font_color_pco(63488);
      hydrometrie.setText(hbuffer);
    } else {
      dtostrf(h, 4, 1, hbuffer);
      hydrometrie.Set_font_color_pco(2016);
      hydrometrie.setText(hbuffer);
    }
    tempe.setValue(number_settemp_tempnuit);
    hysteresis.setValue(number_settemp_hystnuit);
    hydrom.setValue(number_sethydro_hydromaxnuit);
  }

  dtostrf(tempmax, 4, 1, tempmaxbuffer);
  tmax.setText(tempmaxbuffer);

  dtostrf(tempmin, 4, 1, tempminbuffer);
  tmin.setText(tempminbuffer);

  dtostrf(hydromax, 4, 1, hydromaxbuffer);
  hmax.setText(hydromaxbuffer);

  dtostrf(hydromin, 4, 1, hydrominbuffer);
  hmin.setText(hydrominbuffer);

  CO22.setValue(CO2);

  home_led.setValue(valueLed);
  home_lightpic.setPic(icolum);
  home_hydropic.setPic(icohydro);
  home_cyclepic.setPic(icocycle);
  home_chauffpic.setPic(icochauff);
  home_extra.setPic(icoextra);
  home_intra.setPic(icointra);
  home_extra1.Set_font_color_pco(home_extra11);
  home_intra1.Set_font_color_pco(home_intra11);
}

void temphydroDisplay() {
  if (currentPage == 0) {
    String tempString = createClockString();
    displayText(&date_home, tempString);
    String tempString2 = createClockString2();
    displayText(&heure_home, tempString2);

    if ((millis() - previousEcran) >= intervalEcran) {
      if (light == 1) {
        if (previous_t != t) {
          if (t > number_settemp_tempjour) {
            dtostrf(t, 4, 1, tbuffer);
            temperature.Set_font_color_pco(63488);
            temperature.setText(tbuffer);
          } else {
            dtostrf(t, 4, 1, tbuffer);
            temperature.Set_font_color_pco(2016);
            temperature.setText(tbuffer);
          }
          previous_t = t;
        }
        if (previous_h != h) {
          if (h > number_sethydro_hydromaxjour || h < number_sethydro_hydrominjour) {
            dtostrf(h, 4, 1, hbuffer);
            hydrometrie.Set_font_color_pco(63488);
            hydrometrie.setText(hbuffer);
          } else {
            dtostrf(h, 4, 1, hbuffer);
            hydrometrie.Set_font_color_pco(2016);
            hydrometrie.setText(hbuffer);
          }
          previous_h = h;
        }
      }

      else {
        if (previous_t != t) {
          if (t > number_settemp_tempnuit) {
            dtostrf(t, 4, 1, tbuffer);
            temperature.Set_font_color_pco(63488);
            temperature.setText(tbuffer);
          } else {
            dtostrf(t, 4, 1, tbuffer);
            temperature.Set_font_color_pco(2016);
            temperature.setText(tbuffer);
          }
          previous_t = t;
        }
        if (previous_h != h) {
          if (h > number_sethydro_hydromaxnuit || h < number_sethydro_hydrominnuit) {
            dtostrf(h, 4, 1, hbuffer);
            hydrometrie.Set_font_color_pco(63488);
            hydrometrie.setText(hbuffer);
          } else {
            dtostrf(h, 4, 1, hbuffer);
            hydrometrie.Set_font_color_pco(2016);
            hydrometrie.setText(hbuffer);
          }
          previous_h = h;
        }
      }

      if (t > tempmax) {
        tempmax = t;
        dtostrf(tempmax, 4, 1, tempmaxbuffer);
        tmax.setText(tempmaxbuffer);
      }
      if (t < tempmin) {
        tempmin = t;
        dtostrf(tempmin, 4, 1, tempminbuffer);
        tmin.setText(tempminbuffer);
      }
      if (h > hydromax) {
        hydromax = h;
        dtostrf(hydromax, 4, 1, hydromaxbuffer);
        hmax.setText(hydromaxbuffer);
      }
      if (h < hydromin) {
        hydromin = h;
        dtostrf(hydromin, 4, 1, hydrominbuffer);
        hmin.setText(hydrominbuffer);
      }
      if (previous_CO2 != CO2) {
        CO22.setValue(CO2);
        previous_CO2 = CO2;
      }
      previousEcran = millis();
    }
    if ((millis() - previousGraph) >= intervalGraph) {
      if (t > 32) {
        t = 32;
      }
      if (t < 12) {
        t = 12;
      }
      if (sht > 32) {
        sht = 32;
      }
      if (sht < 12) {
        sht = 12;
      }
      if (h > 100) {
        h = 100;
      }
      if (h < 0) {
        h = 0;
      }
      if (CO2 > 2000) {
        CO2 = 2000;
      }
      if (CO2 < 0) {
        CO2 = 0;
      }
      s0.addValue(0, (t - 12) * 10);
      s0.addValue(1, (sht - 12) * 10);
      s1.addValue(0, h * 2);
      s2.addValue(0, CO2 / 10);
      previousGraph = millis();
    }
    if (light == 1) {
      if (tempePrev != number_settemp_tempjour) {
        tempe.setValue(number_settemp_tempjour);
        tempePrev = number_settemp_tempjour;
      }
      if (hysteresisPrev != number_settemp_hystjour) {
        hysteresis.setValue(number_settemp_hystjour);
        hysteresisPrev = number_settemp_hystjour;
      }
      if (hydromPrev != number_sethydro_hydromaxjour) {
        hydrom.setValue(number_sethydro_hydromaxjour);
        hydromPrev = number_sethydro_hydromaxjour;
      }
    } else {
      if (tempePrev != number_settemp_tempnuit) {
        tempe.setValue(number_settemp_tempnuit);
        tempePrev = number_settemp_tempnuit;
      }
      if (hysteresisPrev != number_settemp_hystnuit) {
        hysteresis.setValue(number_settemp_hystnuit);
        hysteresisPrev = number_settemp_hystnuit;
      }
      if (hydromPrev != number_sethydro_hydromaxnuit) {
        hydrom.setValue(number_sethydro_hydromaxnuit);
        hydromPrev = number_sethydro_hydromaxnuit;
      }
    }
    if (lumPrev != icolum) {
      home_lightpic.setPic(icolum);
      lumPrev = icolum;
    }
    if (hydroPrev != icohydro) {
      home_hydropic.setPic(icohydro);
      hydroPrev = icohydro;
    }
    if (cyclePrev != icocycle) {
      home_cyclepic.setPic(icocycle);
      cyclePrev = icocycle;
    }
    if (chauffPrev != icochauff) {
      home_chauffpic.setPic(icochauff);
      chauffPrev = icochauff;
    }
    if (icoextraPrev != icoextra) {
      home_extra.setPic(icoextra);
      icoextraPrev = icoextra;
    }
    if (icointraPrev != icointra) {
      home_intra.setPic(icointra);
      icointraPrev = icointra;
    }
    if (home_extra11Prev != home_extra11) {
      home_extra1.Set_font_color_pco(home_extra11);
      home_extra11Prev = home_extra11;
    }
    if (home_intra11Prev != home_intra11) {
      home_intra1.Set_font_color_pco(home_intra11);
      home_intra11Prev = home_intra11;
    }
    if (previousDay != now.day()) {
      cptejour += 1;
      previousDay = now.day();
      // preferences.putUInt("compteur_jour", cptejour);
    }
    memset(cptebuffer, 0, sizeof(cptebuffer));
    itoa(cptejour, cptebuffer, 10);
    home_nbrejour.setText(cptebuffer);
  }
}

void moytmaxPopCallback(void *ptr) {
  tempmax = 0;
}

void moytminPopCallback(void *ptr) {
  tempmin = 100;
}

void moyhmaxPopCallback(void *ptr) {
  hydromax = 0;
}

void moyhminPopCallback(void *ptr) {
  hydromin = 100;
}

void nbrejourPopCallback(void *ptr) {
  cptejour = 0;
  preferences.putUInt("compteur_jour", cptejour);
}
/**************************
    LIGHT partial start
**************************/
void lightPopCallback(void *ptr) {
  page1.show();
  currentPage = 1;

  setlight_hour_on.setValue(number_setlight_hour_on);
  setlight_min_on.setValue(number_setlight_min_on);
  setlight_hour_off.setValue(number_setlight_hour_off);
  setlight_min_off.setValue(number_setlight_min_off);
  setlight_led.setValue(valueLed);
  if (light_Manual == 1) {
    manulight.setValue(1);
  } else {
    manulight.setValue(0);
  }
  if (light_Timer == 1) {
    autolight.setValue(1);
  } else {
    autolight.setValue(0);
  }
}

void lightManualPopCallback(void *ptr) {
  manulight.getValue(&light_Manual);
  // Allumer ou éteindre manuellement la lampe en fonction de la valeur de light_Manual
  light = light_Manual ? 1 : 0;
  icolum = light_Manual ? 11 : 9;
  ledState1 = light_Manual ? 1 : 0;
  ledState2 = 0;

  // Vérifier si light_Timer est HIGH, auquel cas le désactiver
  if (light_Timer) {
    light_Timer = false;
    manulight.setValue(1);
    autolight.setValue(0);
    icolum = 11;
    ledState1 = 1;
  }
  mcp.digitalWrite(mcp_LIGHT, light_Manual ? HIGH : LOW);  // Allumer ou éteindre la lampe
  // ws.textAll(String(ledState1) + "led1");
}

void lightTimerPopCallback(void *ptr) {
  autolight.getValue(&light_Timer);
  // Mettre à jour l'état de la lampe en fonction de la valeur de light_Timer
  light = light_Timer ? 1 : (light_Manual ? 1 : 0);
  icolum = light_Timer ? 12 : (light_Manual ? 11 : 9);
  ledState1 = light_Timer ? 0 : (light_Manual ? 1 : 0);
  ledState2 = light_Timer ? 1 : 0;

  // Vérifier si light_Manual est HIGH, auquel cas light_Manual reste HIGH et light_Timer devient LOW
  if (light_Manual) {
    manulight.setValue(1);
    autolight.setValue(0);
    icolum = 11;
    ledState1 = 1;
    light_Timer = false;
  } else {
    lightauto();  // Utiliser la fonction lightauto() pour gérer l'état de la lampe en mode automatique
  }

  if (!light_Timer && !light_Manual) {
    manulight.setValue(0);
    autolight.setValue(0);
    light_Manual = 0;
    light_Timer = 0;
    light = 0;
    icolum = 9;
    ledState1 = 0;
    ledState2 = 0;
    mcp.digitalWrite(mcp_LIGHT, LOW);  // Éteindre la lampe en mode manuel
  }
  // ws.textAll(String(ledState2) + "led2");
}

void lightauto() {
  if (light_Timer == 1) {
    if (number_setlight_hour_on < number_setlight_hour_off) {
      // Les heures d'allumage et d'extinction sont dans la même journée
      if (now.hour() > number_setlight_hour_on && now.hour() < number_setlight_hour_off) {
        light = true;
      } else if (now.hour() == number_setlight_hour_on && now.minute() >= number_setlight_min_on) {
        light = true;
      } else if (now.hour() == number_setlight_hour_off && now.minute() < number_setlight_min_off) {
        light = true;
      } else {
        light = false;
      }
    } else if (number_setlight_hour_on > number_setlight_hour_off) {
      // Les heures d'allumage et d'extinction sont sur deux jours différents
      if ((now.hour() > number_setlight_hour_on || now.hour() < number_setlight_hour_off) || (now.hour() == number_setlight_hour_on && now.minute() >= number_setlight_min_on) || (now.hour() == number_setlight_hour_off && now.minute() < number_setlight_min_off)) {
        light = true;
      } else {
        light = false;
      }
    } else {
      // Les heures d'allumage et d'extinction sont identiques
      if (now.minute() >= number_setlight_min_on && now.minute() < number_setlight_min_off) {
        light = true;
      } else {
        light = false;
      }
    }
    mcp.digitalWrite(mcp_LIGHT, light ? HIGH : LOW);
    icolum = (light == 1) ? 10 : 12;
  }
}

void HomeLightPopCallback(void *ptr) {
  setlight_hour_on.getValue(&number_setlight_hour_on);
  setlight_min_on.getValue(&number_setlight_min_on);
  setlight_hour_off.getValue(&number_setlight_hour_off);
  setlight_min_off.getValue(&number_setlight_min_off);
  setlight_led.getValue(&valueLed);
  preferences.putUInt("lighthon", number_setlight_hour_on);
  preferences.putUInt("lighthoff", number_setlight_hour_off);
  preferences.putUInt("lightmon", number_setlight_min_on);
  preferences.putUInt("lightmoff", number_setlight_min_off);
  preferences.putUInt("lightled", valueLed);
  preferences.putUInt("lightmanual", light_Manual);
  preferences.putUInt("lighttimer", light_Timer);
  preferences.putUInt("icolum", icolum);
  preferences.putBool("light", light);
  preferences.putBool("ledState1", ledState1);
  preferences.putBool("ledState2", ledState2);
  /* esp_err_t result = esp_now_send(myMacAddress, (uint8_t *)&valueLed, sizeof(valueLed));
  if (result == ESP_OK) {
    Serial.println("Données envoyées avec succès via ESP-NOW");
  } else {
    Serial.println("Erreur lors de l'envoi des données via ESP-NOW");
  }*/
  pageprinc();
}
/*************************
    TEMP partial start
*************************/
void temp(void *ptr) {
  page2.show();
  currentPage = 2;

  settemp_tempjour.setValue(number_settemp_tempjour);
  settemp_tempnuit.setValue(number_settemp_tempnuit);
  settemp_hystjour.setValue(number_settemp_hystjour);
  settemp_hystnuit.setValue(number_settemp_hystnuit);
}

void HomeTempPopCallback(void *ptr) {
  settemp_tempjour.getValue(&number_settemp_tempjour);
  settemp_tempnuit.getValue(&number_settemp_tempnuit);
  settemp_hystjour.getValue(&number_settemp_hystjour);
  settemp_hystnuit.getValue(&number_settemp_hystnuit);
  preferences.putUInt("tempjour", number_settemp_tempjour);
  preferences.putUInt("tempnuit", number_settemp_tempnuit);
  preferences.putUInt("hystjour", number_settemp_hystjour);
  preferences.putUInt("hystnuit", number_settemp_hystnuit);
  pageprinc();
}
/**************************
    HYDRO partial start
**************************/
void hydroPopCallback(void *ptr) {
  page3.show();
  currentPage = 3;

  sethydro_hydromaxjour.setValue(number_sethydro_hydromaxjour);
  sethydro_hydrominjour.setValue(number_sethydro_hydrominjour);
  sethydro_hydromaxnuit.setValue(number_sethydro_hydromaxnuit);
  sethydro_hydrominnuit.setValue(number_sethydro_hydrominnuit);

  if (hydro_Manual == 1) {
    manuhydro.setValue(1);
  } else {
    manuhydro.setValue(0);
  }
  if (hydro_Timer == 1) {
    autohydro.setValue(1);
  } else {
    autohydro.setValue(0);
  }
}

void hydroManualPopCallback(void *ptr) {
  manuhydro.getValue(&hydro_Manual);
  // Allumer ou éteindre manuellement l hydro en fonction de la valeur de hydro_Manual
  hydro = hydro_Manual ? 1 : 0;
  icohydro = hydro_Manual ? 16 : 14;
  ledState3 = hydro_Manual ? 1 : 0;
  ledState4 = 0;

  // Vérifier si hydro_Timer est HIGH, auquel cas le désactiver
  if (hydro_Timer) {
    hydro_Timer = false;
    manuhydro.setValue(1);
    autohydro.setValue(0);
    icohydro = 16;
    ledState3 = 1;
  }
  mcp.digitalWrite(mcp_HYDRO, hydro_Manual ? HIGH : LOW);  // Allumer ou éteindre l hydro
  // ws.textAll(String(ledState3) + "led3");
}

void hydroTimerPopCallback(void *ptr) {
  autohydro.getValue(&hydro_Timer);
  // Mettre à jour l'état de l hydro en fonction de la valeur de hydro_Timer
  hydro = hydro_Timer ? 1 : (hydro_Manual ? 1 : 0);
  icohydro = hydro_Timer ? 17 : (hydro_Manual ? 16 : 14);
  ledState3 = hydro_Timer ? 0 : (hydro_Manual ? 1 : 0);
  ledState4 = hydro_Timer ? 1 : 0;

  // Vérifier si hydro_Manual est HIGH, auquel cas hydro_Manual reste HIGH et hydro_Timer devient LOW
  if (hydro_Manual) {
    manuhydro.setValue(1);
    autohydro.setValue(0);
    icohydro = 16;
    ledState3 = 1;
    hydro_Timer = false;
  } else {
    hydroauto();  // Utiliser la fonction hydroauto() pour gérer l'état de hydro en mode automatique
  }

  if (!hydro_Timer && !hydro_Manual) {
    manuhydro.setValue(0);
    autohydro.setValue(0);
    hydro_Manual = 0;
    hydro_Timer = 0;
    hydro = 0;
    icohydro = 14;
    ledState3 = 0;
    ledState4 = 0;
    mcp.digitalWrite(mcp_HYDRO, LOW);  // Éteindre la lampe en mode manuel
  }
  // ws.textAll(String(ledState4) + "led4");
}

void hydroauto() {
  if (hydro_Timer == 1) {
    hydro = (light == 1)
              ? (h > number_sethydro_hydromaxjour)   ? (icohydro = 17, mcp.digitalWrite(mcp_HYDRO, LOW), 0)
                : (h < number_sethydro_hydrominjour) ? (icohydro = 15, mcp.digitalWrite(mcp_HYDRO, HIGH), 1)
                                                     : hydro
            : (h > number_sethydro_hydromaxnuit) ? (icohydro = 17, mcp.digitalWrite(mcp_HYDRO, LOW), 0)
            : (h < number_sethydro_hydrominnuit) ? (icohydro = 15, mcp.digitalWrite(mcp_HYDRO, HIGH), 1)
                                                 : hydro;
  }
}

void HomeHydroPopCallback(void *ptr) {
  sethydro_hydromaxjour.getValue(&number_sethydro_hydromaxjour);
  sethydro_hydrominjour.getValue(&number_sethydro_hydrominjour);
  sethydro_hydromaxnuit.getValue(&number_sethydro_hydromaxnuit);
  sethydro_hydrominnuit.getValue(&number_sethydro_hydrominnuit);
  preferences.putUInt("hydromaxj", number_sethydro_hydromaxjour);
  preferences.putUInt("hydrominj", number_sethydro_hydrominjour);
  preferences.putUInt("hydromaxn", number_sethydro_hydromaxnuit);
  preferences.putUInt("hydrominn", number_sethydro_hydrominnuit);
  preferences.putUInt("hydromanual", hydro_Manual);
  preferences.putUInt("hydrotimer", hydro_Timer);
  preferences.putUInt("icohydro", icohydro);
  preferences.putBool("hydro", hydro);
  pageprinc();
}
/***************************
    MOTEUR partial start
***************************/
void moteurPopCallback(void *ptr) {
  page4.show();
  currentPage = 4;

  setmoteur_extrajouroff.setValue(moteurExtrajouroff);
  setmoteur_extrajouron.setValue(moteurExtrajouron);
  setmoteur_extrajourtemp.setValue(moteurExtrajourtemp);
  setmoteur_extrajourhydro.setValue(moteurExtrajourhydro);
  setmoteur_extranuitoff.setValue(moteurExtranuitoff);
  setmoteur_extranuiton.setValue(moteurExtranuiton);
  setmoteur_extranuittemp.setValue(moteurExtranuittemp);
  setmoteur_extranuithydro.setValue(moteurExtranuithydro);
  setmoteur_extrajour.setValue(number_setmoteur_extrajour);
  setmoteur_extranuit.setValue(number_setmoteur_extranuit);
  setmoteur_intrajour.setValue(number_setmoteur_intrajour);
  setmoteur_intranuit.setValue(number_setmoteur_intranuit);
}

void moteurExtrajouroffPopCallback(void *ptr) {
  setmoteur_extrajouroff.getValue(&moteurExtrajouroff);
  if (moteurExtrajouroff) {
    setmoteur_extrajouroff.setValue(1);
    setmoteur_extrajouron.setValue(0);
    setmoteur_extrajourtemp.setValue(0);
    setmoteur_extrajourhydro.setValue(0);
    moteurExtrajouron = 0;
    moteurExtrajourtemp = 0;
    moteurExtrajourhydro = 0;
  }
}

void moteurExtrajouronPopCallback(void *ptr) {
  setmoteur_extrajouron.getValue(&moteurExtrajouron);
  if (moteurExtrajouron) {
    setmoteur_extrajouroff.setValue(0);
    setmoteur_extrajouron.setValue(1);
    setmoteur_extrajourtemp.setValue(0);
    setmoteur_extrajourhydro.setValue(0);
    moteurExtrajouroff = 0;
    moteurExtrajourtemp = 0;
    moteurExtrajourhydro = 0;
  }
}

void moteurExtrajourtempPopCallback(void *ptr) {
  setmoteur_extrajourtemp.getValue(&moteurExtrajourtemp);
  if (moteurExtrajourtemp) {
    setmoteur_extrajouroff.setValue(0);
    setmoteur_extrajouron.setValue(0);
    setmoteur_extrajourtemp.setValue(1);
    setmoteur_extrajourhydro.setValue(0);
    moteurExtrajouroff = 0;
    moteurExtrajouron = 0;
    moteurExtrajourhydro = 0;
  }
}

void moteurExtrajourhydroPopCallback(void *ptr) {
  setmoteur_extrajourhydro.getValue(&moteurExtrajourhydro);
  if (moteurExtrajourhydro) {
    setmoteur_extrajouroff.setValue(0);
    setmoteur_extrajouron.setValue(0);
    setmoteur_extrajourtemp.setValue(0);
    setmoteur_extrajourhydro.setValue(1);
    moteurExtrajouroff = 0;
    moteurExtrajouron = 0;
    moteurExtrajourtemp = 0;
  }
}

void moteurExtranuitoffPopCallback(void *ptr) {
  setmoteur_extranuitoff.getValue(&moteurExtranuitoff);
  if (moteurExtranuitoff) {
    setmoteur_extranuitoff.setValue(1);
    setmoteur_extranuiton.setValue(0);
    setmoteur_extranuittemp.setValue(0);
    setmoteur_extranuithydro.setValue(0);
    moteurExtranuiton = 0;
    moteurExtranuittemp = 0;
    moteurExtranuithydro = 0;
  }
}

void moteurExtranuitonPopCallback(void *ptr) {
  setmoteur_extranuiton.getValue(&moteurExtranuiton);
  if (moteurExtranuiton) {
    setmoteur_extranuitoff.setValue(0);
    setmoteur_extranuiton.setValue(1);
    setmoteur_extranuittemp.setValue(0);
    setmoteur_extranuithydro.setValue(0);
    moteurExtranuitoff = 0;
    moteurExtranuittemp = 0;
    moteurExtranuithydro = 0;
  }
}

void moteurExtranuittempPopCallback(void *ptr) {
  setmoteur_extranuittemp.getValue(&moteurExtranuittemp);
  if (moteurExtranuittemp) {
    setmoteur_extranuitoff.setValue(0);
    setmoteur_extranuiton.setValue(0);
    setmoteur_extranuittemp.setValue(1);
    setmoteur_extranuithydro.setValue(0);
    moteurExtranuitoff = 0;
    moteurExtranuiton = 0;
    moteurExtranuithydro = 0;
  }
}

void moteurExtranuithydroPopCallback(void *ptr) {
  setmoteur_extranuithydro.getValue(&moteurExtranuithydro);
  if (moteurExtranuithydro) {
    setmoteur_extranuitoff.setValue(0);
    setmoteur_extranuiton.setValue(0);
    setmoteur_extranuittemp.setValue(0);
    setmoteur_extranuithydro.setValue(1);
    moteurExtranuitoff = 0;
    moteurExtranuiton = 0;
    moteurExtranuittemp = 0;
  }
}

void extra(int8_t EXV1, int8_t EXV2, int8_t EXV3, int8_t EXV4, int8_t EXV5) {
  speedEx[0] = 0;
  speedEx[1] = mcp_EXV1;
  speedEx[2] = mcp_EXV2;
  speedEx[3] = mcp_EXV3;
  speedEx[4] = mcp_EXV4;
  speedEx[5] = mcp_EXV5;
}

void icoex(int8_t PIC0, int8_t PIC1, int8_t PIC2, int8_t PIC3, int8_t PIC4, int8_t PIC5) {
  icoEx[0] = PIC_EXV0;
  icoEx[1] = PIC_EXV1;
  icoEx[2] = PIC_EXV2;
  icoEx[3] = PIC_EXV3;
  icoEx[4] = PIC_EXV4;
  icoEx[5] = PIC_EXV5;
}

void setSpeedEx(uint8_t value) {
  icoextra = icoEx[value];
  if (previousSpeedEx != value) {
    resetEx();
    if (value != 0) {
      mcp.digitalWrite(speedEx[value], HIGH);
    }
    previousSpeedEx = value;
  }
}

void resetEx(void) {
  for (uint8_t i = 1; i <= 5; i++) {
    mcp.digitalWrite(speedEx[i], LOW);
  }
  delay(60);
}

void intra(int8_t INV1, int8_t INV2, int8_t INV3, int8_t INV4, int8_t INV5) {
  speedIn[0] = 0;
  speedIn[1] = mcp_INV1;
  speedIn[2] = mcp_INV2;
  speedIn[3] = mcp_INV3;
  speedIn[4] = mcp_INV4;
  speedIn[5] = mcp_INV5;
}

void icoin(int8_t PIC0, int8_t PIC1, int8_t PIC2, int8_t PIC3, int8_t PIC4, int8_t PIC5) {
  icoIn[0] = PIC_INV0;
  icoIn[1] = PIC_INV1;
  icoIn[2] = PIC_INV2;
  icoIn[3] = PIC_INV3;
  icoIn[4] = PIC_INV4;
  icoIn[5] = PIC_INV5;
}

void setSpeedIn(uint8_t value) {
  icointra = icoIn[value];
  if (previousSpeedIn != value) {
    resetIn();
    if (value != 0) {
      mcp.digitalWrite(speedIn[value], HIGH);
    }
    previousSpeedIn = value;
  }
}

void resetIn(void) {
  for (uint8_t i = 1; i <= 5; i++) {
    mcp.digitalWrite(speedIn[i], LOW);
  }
  delay(60);
}

void moteurauto() {
  if (light == 1) {
    if (moteurExtrajouroff == 1) {
      setSpeedEx(0);
      icoextra = 50;
      home_extra11 = 63488;
      setSpeedIn(0);
      icointra = 56;
      home_intra11 = 63488;
    } else if (moteurExtrajouron == 1) {
      setSpeedEx(number_setmoteur_extrajour);
      setSpeedIn(number_setmoteur_intrajour);
      home_extra11 = 63488;
      home_intra11 = 63488;
    } else if (moteurExtrajourtemp == 1) {
      if (t > number_settemp_tempjour) {
        setSpeedEx(number_setmoteur_extrajour);
        setSpeedIn(number_setmoteur_intrajour);
        home_extra11 = 2016;
        home_intra11 = 2016;
      } else if (t < number_settemp_tempjour - number_settemp_hystjour) {
        setSpeedEx(0);
        icoextra = 50;
        home_extra11 = 31;
        setSpeedIn(0);
        icointra = 56;
        home_intra11 = 31;
      }
    } else if (moteurExtrajourhydro == 1) {
      if (h > number_sethydro_hydromaxjour) {
        setSpeedEx(number_setmoteur_extrajour);
        setSpeedIn(number_setmoteur_intrajour);
        home_extra11 = 2016;
        home_intra11 = 2016;
      } else if (h < number_sethydro_hydrominjour) {
        setSpeedEx(0);
        icoextra = 50;
        home_extra11 = 31;
        setSpeedIn(0);
        icointra = 56;
        home_intra11 = 31;
      }
    }
  }

  else {
    if (moteurExtranuitoff == 1) {
      setSpeedEx(0);
      icoextra = 50;
      home_extra11 = 63488;
      setSpeedIn(0);
      icointra = 56;
      home_intra11 = 63488;
    } else if (moteurExtranuiton == 1) {
      setSpeedEx(number_setmoteur_extranuit);
      setSpeedIn(number_setmoteur_intranuit);
    } else if (moteurExtranuittemp == 1) {
      if (t > number_settemp_tempnuit) {
        setSpeedEx(number_setmoteur_extranuit);
        setSpeedIn(number_setmoteur_intranuit);
        home_extra11 = 2016;
        home_intra11 = 2016;
      } else if (t < number_settemp_tempnuit - number_settemp_hystnuit) {
        setSpeedEx(0);
        icoextra = 50;
        home_extra11 = 31;
        setSpeedIn(0);
        icointra = 56;
        home_intra11 = 31;
      }
    } else if (moteurExtranuithydro == 1) {
      if (h > number_sethydro_hydromaxnuit) {
        setSpeedEx(number_setmoteur_extranuit);
        setSpeedIn(number_setmoteur_intranuit);
        home_extra11 = 2016;
        home_intra11 = 2016;
      } else if (h < number_sethydro_hydrominnuit) {
        setSpeedEx(0);
        icoextra = 50;
        home_extra11 = 31;
        setSpeedIn(0);
        icointra = 56;
        home_intra11 = 31;
      }
    }
  }
  if (previousMotExj != number_setmoteur_extrajour) {
    preferences.putUInt("motexj", number_setmoteur_extrajour);
    previousMotExj = number_setmoteur_extrajour;
  }
  if (previousMotInj != number_setmoteur_intrajour) {
    preferences.putUInt("motinj", number_setmoteur_intrajour);
    previousMotInj = number_setmoteur_intrajour;
  }
  if (previousMotExn != number_setmoteur_extranuit) {
    preferences.putUInt("motexn", number_setmoteur_extranuit);
    previousMotExn = number_setmoteur_extranuit;
  }
  if (previousMotInn != number_setmoteur_intranuit) {
    preferences.putUInt("motinn", number_setmoteur_intranuit);
    previousMotInn = number_setmoteur_intranuit;
  }
}

void HomeMoteurPopCallback(void *ptr) {
  setmoteur_extrajour.getValue(&number_setmoteur_extrajour);
  setmoteur_extranuit.getValue(&number_setmoteur_extranuit);
  setmoteur_intrajour.getValue(&number_setmoteur_intrajour);
  setmoteur_intranuit.getValue(&number_setmoteur_intranuit);
  setmoteur_extrajouroff.getValue(&moteurExtrajouroff);
  setmoteur_extrajouron.getValue(&moteurExtrajouron);
  setmoteur_extrajourtemp.getValue(&moteurExtrajourtemp);
  setmoteur_extrajourhydro.getValue(&moteurExtrajourhydro);
  setmoteur_extranuitoff.getValue(&moteurExtranuitoff);
  setmoteur_extranuiton.getValue(&moteurExtranuiton);
  setmoteur_extranuittemp.getValue(&moteurExtranuittemp);
  setmoteur_extranuithydro.getValue(&moteurExtranuithydro);
  preferences.putUInt("motexjoff", moteurExtrajouroff);
  preferences.putUInt("motexjon", moteurExtrajouron);
  preferences.putUInt("motexnoff", moteurExtranuitoff);
  preferences.putUInt("motexnon", moteurExtranuiton);
  preferences.putUInt("motexjtemp", moteurExtrajourtemp);
  preferences.putUInt("motexntemp", moteurExtranuittemp);
  preferences.putUInt("motexjhydro", moteurExtrajourhydro);
  preferences.putUInt("motexnhydro", moteurExtranuithydro);
  preferences.putUInt("icoextraj", icoextra);
  preferences.putUInt("icointraj", icointra);
  preferences.putUInt("extra11", home_extra11);
  preferences.putUInt("intra11", home_intra11);
  pageprinc();
}
/**************************
    CYCLE partial start
**************************/
void cyclePopCallback(void *ptr) {
  page5.show();
  currentPage = 5;

  setcycle_jouron.setValue(number_setcycle_jouron);
  setcycle_jouroff.setValue(number_setcycle_jouroff);
  setcycle_nuiton.setValue(number_setcycle_nuiton);
  setcycle_nuitoff.setValue(number_setcycle_nuitoff);

  if (cycle_Manual == 1) {
    manucycle.setValue(1);
  } else {
    manucycle.setValue(0);
  }
  if (cycle_Timer == 1) {
    autocycle.setValue(1);
  } else {
    autocycle.setValue(0);
  }
}

void cycleManualPopCallback(void *ptr) {
  manucycle.getValue(&cycle_Manual);
  // Allumer ou éteindre manuellement cycle en fonction de la valeur de cycle_Manual
  cycle = cycle_Manual ? 1 : 0;
  icocycle = cycle_Manual ? 22 : 20;
  ledState9 = cycle_Manual ? 1 : 0;
  ledState10 = 0;

  // Vérifier si light_Timer est HIGH, auquel cas le désactiver
  if (cycle_Timer) {
    cycle_Timer = false;
    manucycle.setValue(1);
    autocycle.setValue(0);
    icocycle = 22;
    ledState9 = 1;
  }
  mcp.digitalWrite(mcp_CYCLE, cycle_Manual ? HIGH : LOW);  // Allumer ou éteindre cycle
  // ws.textAll(String(ledState9) + "led9");
}

void cycleTimerPopCallback(void *ptr) {
  autocycle.getValue(&cycle_Timer);
  // Mettre à jour l'état de cycle en fonction de la valeur de cycle_Timer
  cycle = cycle_Timer ? 1 : (cycle_Manual ? 1 : 0);
  icocycle = cycle_Timer ? 23 : (cycle_Manual ? 22 : 20);
  ledState9 = cycle_Timer ? 0 : (cycle_Manual ? 1 : 0);
  ledState10 = cycle_Timer ? 1 : 0;

  // Vérifier si cycle_Manual est HIGH, auquel cas cycle_Manual reste HIGH et cycle_Timer devient LOW
  if (cycle_Manual) {
    manucycle.setValue(1);
    autocycle.setValue(0);
    icocycle = 22;
    ledState9 = 1;
    cycle_Timer = false;
  } else {
    cycleauto();  // Utiliser la fonction cycleauto() pour gérer l'état de hydro en mode automatique
  }

  if (!cycle_Timer && !cycle_Manual) {
    manucycle.setValue(0);
    autocycle.setValue(0);
    cycle_Manual = 0;
    cycle_Timer = 0;
    cycle = 0;
    icocycle = 20;
    ledState9 = 0;
    ledState10 = 0;
    mcp.digitalWrite(mcp_CYCLE, LOW);  // Éteindre la lampe en mode manuel
  }
  // ws.textAll(String(ledState10) + "led10");
}

void cycleauto() {
  if (cycle_Timer == 1) {
    unsigned long interval;
    unsigned long onTime, offTime;

    if (light == 1) {
      onTime = number_setcycle_jouron;
      offTime = number_setcycle_jouroff;
    } else {
      onTime = number_setcycle_nuiton;
      offTime = number_setcycle_nuitoff;
    }

    if (offTime == 0) {
      cycle = 1;
      icocycle = 21;
      mcp.digitalWrite(mcp_CYCLE, HIGH);
    } else {
      if (ledState) {
        interval = offTime;
      } else {
        interval = onTime;
      }
      if ((now.minute() - previousCycle) >= interval) {
        previousCycle = now.minute();
        ledState = !ledState;
        cycle = !ledState;
        delay(10);
        mcp.digitalWrite(mcp_CYCLE, cycle);
        if (cycle == true) {
          icocycle = 21;
        } else {
          icocycle = 23;
        }
      }
    }
  }
}

void HomeCyclePopCallback(void *ptr) {
  setcycle_jouron.getValue(&number_setcycle_jouron);
  setcycle_jouroff.getValue(&number_setcycle_jouroff);
  setcycle_nuiton.getValue(&number_setcycle_nuiton);
  setcycle_nuitoff.getValue(&number_setcycle_nuitoff);
  preferences.putUInt("cyclejon", number_setcycle_jouron);
  preferences.putUInt("cyclejoff", number_setcycle_jouroff);
  preferences.putUInt("cyclenon", number_setcycle_nuiton);
  preferences.putUInt("cyclenoff", number_setcycle_nuitoff);
  preferences.putUInt("cyclemanual", cycle_Manual);
  preferences.putUInt("cycletimer", cycle_Timer);
  preferences.putUInt("icocycle", icocycle);
  preferences.putBool("cycle", cycle);
  preferences.putBool("ledState9", ledState9);
  preferences.putBool("ledState10", ledState10);
  pageprinc();
}
/******************************
    CHAUFFAGE partial start
******************************/
void chauffagePopCallback(void *ptr) {
  page6.show();
  currentPage = 6;

  setchauffmax.setValue(number_setchauffmax);
  setchauffmin.setValue(number_setchauffmin);

  if (chauff_Manual == 1) {
    manuchauff.setValue(1);
  } else {
    manuchauff.setValue(0);
  }
  if (chauff_Timer == 1) {
    autochauff.setValue(1);
  } else {
    autochauff.setValue(0);
  }
}

void chauffManualPopCallback(void *ptr) {
  manuchauff.getValue(&chauff_Manual);
  // Allumer ou éteindre manuellement le chauffage en fonction de la valeur de chauff_Manual
  chauff = chauff_Manual ? 1 : 0;
  icochauff = chauff_Manual ? 27 : 25;
  ledState11 = chauff_Manual ? 1 : 0;
  ledState12 = 0;

  // Vérifier si chauff_Timer est HIGH, auquel cas le désactiver
  if (chauff_Timer) {
    chauff_Timer = false;
    manuchauff.setValue(1);
    autochauff.setValue(0);
    icochauff = 27;
    ledState11 = 1;
  }
  mcp.digitalWrite(mcp_CHAUFF, chauff_Manual ? HIGH : LOW);  // Allumer ou éteindre la lampe
  // ws.textAll(String(ledState11) + "led11");
}

void chauffTimerPopCallback(void *ptr) {
  autochauff.getValue(&chauff_Timer);
  // Mettre à jour l'état du chauffage en fonction de la valeur de chauff_Timer
  chauff = chauff_Timer ? 1 : (chauff_Manual ? 1 : 0);
  icochauff = chauff_Timer ? 28 : (chauff_Manual ? 27 : 25);
  ledState11 = chauff_Timer ? 0 : (chauff_Manual ? 1 : 0);
  ledState12 = chauff_Timer ? 1 : 0;

  // Vérifier si chauff_Manual est HIGH, auquel cas chauff_Manual reste HIGH et chauff_Timer devient LOW
  if (chauff_Manual) {
    manuchauff.setValue(1);
    autochauff.setValue(0);
    icochauff = 27;
    ledState11 = 1;
    chauff_Timer = false;
  } else {
    chauffauto();  // Utiliser la fonction chauffauto() pour gérer l'état du chauffage en mode automatique
  }

  if (!chauff_Timer && !chauff_Manual) {
    manuchauff.setValue(0);
    autochauff.setValue(0);
    chauff_Manual = 0;
    chauff_Timer = 0;
    chauff = 0;
    icochauff = 25;
    ledState11 = 0;
    ledState12 = 0;
    mcp.digitalWrite(mcp_CHAUFF, LOW);  // Éteindre la lampe en mode manuel
  }
  // ws.textAll(String(ledState12) + "led12");
}

void chauffauto() {
  if (chauff_Timer == 1) {
    chauff = (light == 1) ? 0 : ((t < number_setchauffmin) ? 1 : ((t > number_setchauffmax) ? 0 : chauff));
    icochauff = (light == 1) ? 25 : ((mcp.digitalRead(mcp_CHAUFF) == HIGH) ? 26 : 28);
    mcp.digitalWrite(mcp_CHAUFF, (light == 1) ? LOW
                                              : ((t < number_setchauffmin) ? ((t < number_setchauffmax) ? HIGH : LOW)
                                                                           : ((t > number_setchauffmax) ? LOW : HIGH)));
  }
}

void HomeChauffPopCallback(void *ptr) {
  setchauffmax.getValue(&number_setchauffmax);
  setchauffmin.getValue(&number_setchauffmin);
  preferences.putUInt("chauffmax", number_setchauffmax);
  preferences.putUInt("chauffmin", number_setchauffmin);
  preferences.putUInt("chauffmanual", chauff_Manual);
  preferences.putUInt("chaufftimer", chauff_Timer);
  preferences.putUInt("icochauff", icochauff);
  preferences.putBool("chauff", chauff);
  preferences.putBool("ledState11", ledState11);
  preferences.putBool("ledState12", ledState12);
  pageprinc();
}
/*************************
  TIME partial start
*************************/
void timePopCallback(void *ptr) {
  page7.show();
  currentPage = 7;

  setday.setValue(now.day());
  setmon.setValue(now.month());
  setyear.setValue(now.year());
  sethour.setValue(now.hour());
  setmin.setValue(now.minute());
}

void timeOkPopCallback(void *ptr) {
  setday.getValue(&number_settime_day);
  setmon.getValue(&number_settime_month);
  setyear.getValue(&number_settime_year);
  sethour.getValue(&number_settime_hour);
  setmin.getValue(&number_settime_minute);

  rtc.adjust(DateTime(number_settime_year, number_settime_month,
                      number_settime_day, number_settime_hour,
                      number_settime_minute, 0));
  delay(150);
}

void HomeTimePopCallback(void *ptr) {
  pageprinc();
}

void wifi() {
  if (now.minute() != previousWifi) {
    dtostrf(t, 5, 2, tbuffer);
    dtostrf(h, 5, 2, hbuffer);
    dtostrf(sht, 5, 2, sht31buffer);
    dtostrf(CO2, 7, 2, CO2buffer);
    moteur = (mcp.digitalRead(mcp_EXV1) == 1 || mcp.digitalRead(mcp_EXV2) == 1 || mcp.digitalRead(mcp_EXV3) == 1
              || mcp.digitalRead(mcp_EXV4) == 1 || mcp.digitalRead(mcp_EXV5) == 1 || mcp.digitalRead(mcp_INV1) == 1
              || mcp.digitalRead(mcp_INV2) == 1 || mcp.digitalRead(mcp_INV3) == 1 || mcp.digitalRead(mcp_INV4) == 1
              || mcp.digitalRead(mcp_INV5) == 1)
               ? 1
               : 0;
    ThingSpeak.setField(1, tbuffer);
    ThingSpeak.setField(2, hbuffer);
    ThingSpeak.setField(3, sht31buffer);
    ThingSpeak.setField(4, CO2buffer);
    ThingSpeak.setField(5, (light));
    ThingSpeak.setField(6, (hydro));
    ThingSpeak.setField(7, (moteur));
    ThingSpeak.setField(8, (cycle));
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    // write to the ThingSpeak channel
    int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (httpCode == 200) {
      Serial.println("Data sent to ThingSpeak successfully");
    } else {
      Serial.print("Failed to send data to ThingSpeak. HTTP error code: ");
      Serial.println(httpCode);
      previousWifi = now.minute();
    }
  }
}

void onOTAStart() {
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
}

void setup() {
  nexInit();
  Serial.begin(115200);
  xTaskCreatePinnedToCore(Core1, "Task1", 10000, NULL, 0, &Task1, 1);
  Wire.begin();
  rtc.begin();
  now = rtc.now();
  sht4.begin();
  scd30.begin();
  scd30.setMeasurementInterval(3);
  //scd30.setTemperatureOffset(210); // diff temp degrees celcius
  mcp.begin_I2C(0x20);
  preferences.begin("test", false);
  sondes();
  /*SPIFFS.begin();
  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });
  server.on("/jquery-3.4.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/jquery-3.4.1.min.js", "text/javascript");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/delayLed1", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("number_setmoteur_extrajour", true)) {
      String message;
      message = request->getParam("number_setmoteur_extrajour", true)->value();
      number_setmoteur_extrajour = message.toInt();
    }
    request->send(204);
  });
  server.on("/delayLed2", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("number_setmoteur_intrajour", true)) {
      String message;
      message = request->getParam("number_setmoteur_intrajour", true)->value();
      number_setmoteur_intrajour = message.toInt();
    }
    request->send(204);
  });
  server.on("/delayLed3", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("number_setmoteur_extranuit", true)) {
      String message;
      message = request->getParam("number_setmoteur_extranuit", true)->value();
      number_setmoteur_extranuit = message.toInt();
    }
    request->send(204);
  });
  server.on("/delayLed4", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("number_setmoteur_intranuit", true)) {
      String message;
      message = request->getParam("number_setmoteur_intranuit", true)->value();
      number_setmoteur_intranuit = message.toInt();
    }
    request->send(204);
  });
  server.on("/lirenbrj", HTTP_GET, [](AsyncWebServerRequest *request) {
    int val = cptejour;
    String cptejour2 = String(val);
    request->send(200, "text/plain", cptejour2);
  });
  server.on("/setBrightness", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("brightness", true)) {
      String message;
      message = request->getParam("valueLed", true)->value();
      valueLed = message.toInt();
    }
    request->send(204);  // Réponse 204 No Content pour indiquer que tout s'est bien passé
  });
  server.serveStatic("/", SPIFFS, "/");*/
  WiFi.mode(WIFI_AP_STA);
  delay(1000);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    delay(3000);
  }
  ThingSpeak.begin(client);
  ElegantOTA.begin(&server);
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  server.begin();
  // Initialiser ESP-NOW
    if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur lors de l'initialisation d'ESP-NOW");
    return;
  }
  /*// Une fois qu'ESPNow est initialisé avec succès, nous nous inscrirons pour le rappel d'envoi pour
  // obtenir le statut du paquet transmis
  esp_now_register_send_cb(OnDataSent);
  // Enregistrer le callback pour la réception de données
  esp_now_register_recv_cb(onDataRecv);*/
  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");

  delay(10);
  mcp.pinMode(mcp_LIGHT, OUTPUT);
  mcp.digitalWrite(mcp_LIGHT, LOW);
  mcp.pinMode(mcp_HYDRO, OUTPUT);
  mcp.digitalWrite(mcp_HYDRO, LOW);
  mcp.pinMode(mcp_CYCLE, OUTPUT);
  mcp.digitalWrite(mcp_CYCLE, LOW);
  mcp.pinMode(mcp_CHAUFF, OUTPUT);
  mcp.digitalWrite(mcp_CHAUFF, LOW);
  mcp.pinMode(mcp_EXV1, OUTPUT);
  mcp.digitalWrite(mcp_EXV1, LOW);
  mcp.pinMode(mcp_EXV2, OUTPUT);
  mcp.digitalWrite(mcp_EXV2, LOW);
  mcp.pinMode(mcp_EXV3, OUTPUT);
  mcp.digitalWrite(mcp_EXV3, LOW);
  mcp.pinMode(mcp_EXV4, OUTPUT);
  mcp.digitalWrite(mcp_EXV4, LOW);
  mcp.pinMode(mcp_EXV5, OUTPUT);
  mcp.digitalWrite(mcp_EXV5, LOW);
  mcp.pinMode(mcp_INV1, OUTPUT);
  mcp.digitalWrite(mcp_INV1, LOW);
  mcp.pinMode(mcp_INV2, OUTPUT);
  mcp.digitalWrite(mcp_INV2, LOW);
  mcp.pinMode(mcp_INV3, OUTPUT);
  mcp.digitalWrite(mcp_INV3, LOW);
  mcp.pinMode(mcp_INV4, OUTPUT);
  mcp.digitalWrite(mcp_INV4, LOW);
  mcp.pinMode(mcp_INV5, OUTPUT);
  mcp.digitalWrite(mcp_INV5, LOW);
  extra(0, 0, 0, 0, 0);
  intra(0, 0, 0, 0, 0);
  icoex(0, 0, 0, 0, 0, 0);
  icoin(0, 0, 0, 0, 0, 0);
  delay(10);

  // recuperation en cas de coupure elec: CPTE JOUR
  // cptejour = preferences.getUInt("compteur_jour", 0);

  // recuperation en cas de coupure elec: LIGHT
  number_setlight_hour_on = preferences.getUInt("lighthon", 19);
  number_setlight_hour_off = preferences.getUInt("lighthoff", 7);
  number_setlight_min_on = preferences.getUInt("lightmon", 13);
  number_setlight_min_off = preferences.getUInt("lightmoff", 0);
  valueLed = preferences.getUInt("lightled", 100);
  light_Manual = preferences.getUInt("lightmanual", 69);
  light_Timer = preferences.getUInt("lighttimer", 64);
  icolum = preferences.getUInt("icolum", 9);
  light = preferences.getBool("light", false);
  ledState1 = preferences.getBool("ledState1", false);
  ledState2 = preferences.getBool("ledState2", false);

  // recuperation en cas de coupure elec: TEMP
  number_settemp_tempjour = preferences.getUInt("tempjour", 26);
  number_settemp_tempnuit = preferences.getUInt("tempnuit", 22);
  number_settemp_hystjour = preferences.getUInt("hystjour", 2);
  number_settemp_hystnuit = preferences.getUInt("hystnuit", 2);

  // recuperation en cas de coupure elec: HYDRO
  number_sethydro_hydromaxjour = preferences.getUInt("hydromaxj", 57);
  number_sethydro_hydrominjour = preferences.getUInt("hydrominj", 55);
  number_sethydro_hydromaxnuit = preferences.getUInt("hydromaxn", 57);
  number_sethydro_hydrominnuit = preferences.getUInt("hydrominn", 55);
  hydro_Manual = preferences.getUInt("hydromanual", 69);
  hydro_Timer = preferences.getUInt("hydrotimer", 64);
  icohydro = preferences.getUInt("icohydro", 14);
  hydro = preferences.getBool("hydro", false);
  ledState3 = preferences.getBool("ledState3", false);
  ledState4 = preferences.getBool("ledState4", false);

  // recuperation en cas de coupure elec: MOTEUR
  number_setmoteur_extrajour = preferences.getUInt("motexj", 4);
  number_setmoteur_intrajour = preferences.getUInt("motinj", 3);
  number_setmoteur_extranuit = preferences.getUInt("motexn", 3);
  number_setmoteur_intranuit = preferences.getUInt("motinn", 2);
  moteurExtrajouroff = preferences.getUInt("motexjoff", 0);
  moteurExtrajouron = preferences.getUInt("motexjon", 0);
  moteurExtranuitoff = preferences.getUInt("motexnoff", 0);
  moteurExtranuiton = preferences.getUInt("motexnon", 0);
  moteurExtrajourtemp = preferences.getUInt("motexjtemp", 0);
  moteurExtranuittemp = preferences.getUInt("motexntemp", 0);
  moteurExtrajourhydro = preferences.getUInt("motexjhydro", 0);
  moteurExtranuithydro = preferences.getUInt("motexnhydro", 0);
  icoextra = preferences.getUInt("icoextraj", 50);
  icointra = preferences.getUInt("icointraj", 50);
  home_extra11 = preferences.getUInt("extra11", 65535);
  home_intra11 = preferences.getUInt("intra11", 65535);

  // recuperation en cas de coupure elec: CYCLE
  number_setcycle_jouron = preferences.getUInt("cyclejon", 60);
  number_setcycle_jouroff = preferences.getUInt("cyclejoff", 0);
  number_setcycle_nuiton = preferences.getUInt("cyclenon", 3);
  number_setcycle_nuitoff = preferences.getUInt("cyclenoff", 1);
  cycle_Manual = preferences.getUInt("cyclemanual", 69);
  cycle_Timer = preferences.getUInt("cycletimer", 64);
  icocycle = preferences.getUInt("icocycle", 20);
  cycle = preferences.getBool("cycle", false);
  ledState9 = preferences.getBool("ledState9", false);
  ledState10 = preferences.getBool("ledState10", false);

  // recuperation en cas de coupure elec: CHAUFFAGE
  number_setchauffmax = preferences.getUInt("chauffmax", 23);
  number_setchauffmin = preferences.getUInt("chauffmin", 20);
  chauff_Manual = preferences.getUInt("chauffmanual", 69);
  chauff_Timer = preferences.getUInt("chaufftimer", 64);
  icochauff = preferences.getUInt("icochauff", 25);
  chauff = preferences.getBool("chauff", false);
  ledState11 = preferences.getBool("ledState11", false);
  ledState12 = preferences.getBool("ledState12", false);

  // page ecran: HOME
  home_lightpic.attachPop(lightPopCallback);
  temperature.attachPop(temp);
  hydrometrie.attachPop(hydroPopCallback);
  home_hydropic.attachPop(hydroPopCallback);
  tmax.attachPop(moytmaxPopCallback);
  tmin.attachPop(moytminPopCallback);
  hmax.attachPop(moyhmaxPopCallback);
  hmin.attachPop(moyhminPopCallback);
  home_extra.attachPop(moteurPopCallback);
  home_extra1.attachPop(moteurPopCallback);
  home_intra.attachPop(moteurPopCallback);
  home_intra1.attachPop(moteurPopCallback);
  home_cyclepic.attachPop(cyclePopCallback);
  home_chauffpic.attachPop(chauffagePopCallback);
  home_nbrejour.attachPop(nbrejourPopCallback);
  settime.attachPop(timePopCallback);

  // page ecran: LIGHT
  manulight.attachPop(lightManualPopCallback);
  autolight.attachPop(lightTimerPopCallback);
  light_back_to_set.attachPop(HomeLightPopCallback);

  // page ecran: TEMP
  temp_back_to_set.attachPop(HomeTempPopCallback);

  // page ecran: HYDRO
  manuhydro.attachPop(hydroManualPopCallback);
  autohydro.attachPop(hydroTimerPopCallback);
  hydro_back_to_set.attachPop(HomeHydroPopCallback);

  // page ecran: MOTEUR
  setmoteur_extrajouroff.attachPop(moteurExtrajouroffPopCallback);
  setmoteur_extrajouron.attachPop(moteurExtrajouronPopCallback);
  setmoteur_extrajourtemp.attachPop(moteurExtrajourtempPopCallback);
  setmoteur_extrajourhydro.attachPop(moteurExtrajourhydroPopCallback);
  setmoteur_extranuitoff.attachPop(moteurExtranuitoffPopCallback);
  setmoteur_extranuiton.attachPop(moteurExtranuitonPopCallback);
  setmoteur_extranuittemp.attachPop(moteurExtranuittempPopCallback);
  setmoteur_extranuithydro.attachPop(moteurExtranuithydroPopCallback);
  moteur_back_to_set.attachPop(HomeMoteurPopCallback);

  // page ecran: CYCLE
  manucycle.attachPop(cycleManualPopCallback);
  autocycle.attachPop(cycleTimerPopCallback);
  cycle_back_to_set.attachPop(HomeCyclePopCallback);

  // page ecran: CHAUFFAGE
  manuchauff.attachPop(chauffManualPopCallback);
  autochauff.attachPop(chauffTimerPopCallback);
  chauffage_back_to_set.attachPop(HomeChauffPopCallback);

  // page ecran: TIME
  settime_ok.attachPop(timeOkPopCallback);
  backhome.attachPop(HomeTimePopCallback);

  previousDay = now.day();
  pageprinc();
}

void sondes(void) {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);
  t = temp.temperature;
  h = humidity.relative_humidity;
  if (scd30.dataReady()) {
    if (!scd30.read()) {
      Serial.println("Error reading sensor data");
      return;
    }
  }
  CO2 = scd30.CO2;
  sht = scd30.temperature;
  now = rtc.now();
}

void fonction(void) {
  lightauto();
  hydroauto();
  moteurauto();
  cycleauto();
  chauffauto();
  wifi();
}

void loop() {
  unsigned long start, stop = 0;
  start = millis();
  sondes();
  fonction();
  //ws.cleanupClients();
  ElegantOTA.loop();
  if (currentPage == 0) {
    temphydroDisplay();
  }
  // Envoyer la valeur stockée si demandé par l'esclave
   if (sendRequested && !sentValueStock) {
    Serial.print("L'esclave me demande l'entier, je lui envoie la valeur suivante: ");
    Serial.println(valueLed);
    esp_err_t result = esp_now_send(ESP_NOW.BROADCAST_ADDR, (uint8_t *)&valueLed, sizeof(valueLed));
    if (result == ESP_OK) {
      sentValueStock = true;  // Marquer la valeur comme envoyée
      sendRequested = false;  // Réinitialiser la demande après l'envoi réussi
    } else {
      Serial.println("Erreur lors de l'envoi de la demande");
    }
  }
  stop = millis();
  delta = stop - start;
  if (delta < 1000) {
    delay(1000 - delta);
  }
  Serial.print("freeMemory = ");
  Serial.println(ESP.getFreeHeap());
  /*Serial.print("led = ");
  Serial.println(valueLed);
  Serial.print("vit moteur = ");
  Serial.println(number_setmoteur_extrajour);*/
}
