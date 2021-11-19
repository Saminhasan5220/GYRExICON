#include <RTClib.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include "NTPClient.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "HTTPSRedirect.h"
#include "SPI.h"
#include "SD.h"

struct data {
  int Year = 0;
  int Month = 0;
  int Date = 0;
  int Day = 0;
  int H = 0;
  int M = 0;
  int S = 0;
  bool Connection = 0 ;// connection to internet
  float Analog0 = 0;
  bool Digital0 = 0;
};
struct data fData;

String SerialprintDataStruct(struct data d);
bool connect_to_wifi(const char* ssid, const char* password);
void get_ntp_time();
DateTime get_rtc_time();
void set_rtc_time();
void sync_clock();
void log_data();
void connect_to_gs();
void log_cloud(String datastr);
void showNewData();
void recvWithStartEndMarkers();
const int chipSelect = D8;
bool debug=false;
//
const byte numChars = 64;
char receivedChars[numChars];
boolean newData = false;
//

/*
 * TODO:
 * implement log_cloud function
*/


// Enter network credentials:
const char* ssid     = "Flynn";
const char* password = "sammflynn";

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
const long utcOffsetInSeconds = 6 * 60 * 60;//seconds + 6 GMT
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


//
bool connected, RTC, sdcard;
////////////////////////////////////////////////////////////////////////////////////////
// Enter Google Script Deployment ID:

//AKfycbyHb6GvqEiTJwx8WL0OcgvWVeLwvcwp2Wc2MMGz8zl8IHDoq5-sa8iCUc31hhCBDmwWmw
//https://docs.google.com/spreadsheets/d/1M3xaxG51xiiUtlAeIqKY_2oDXReSYx8Zt0Uc16y3_Ok/edit#gid=0
const char *GScriptId = "AKfycbyHb6GvqEiTJwx8WL0OcgvWVeLwvcwp2Wc2MMGz8zl8IHDoq5-sa8iCUc31hhCBDmwWmw";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec?cal";
HTTPSRedirect* client = nullptr;
//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\//





String filename;




 bool sdebug = true;
void setup() {
  delay(5000);
  Serial.begin(115200);
  connected = connect_to_wifi(ssid, password);
  delay(5000);
  if (connected)
  get_ntp_time();
  delay(1000);

  if (! rtc.begin()) {
    if (debug)
    {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    }
    RTC = false;
  }
  else
    RTC = true;

  //??
  if (RTC)
  get_rtc_time();
  if (RTC && connected)
  
  sync_clock();

  pinMode(A0, INPUT); //A0
  pinMode(D0, INPUT); //D0


  char buff [64];
DateTime now = rtc.now(); 
//Updated now.day to now.date
sprintf(buff, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 


filename = buff;
filename += String(".txt");
if(sdebug)
Serial.println(filename);
  if (SD.begin(chipSelect))
  {
sdcard = true;
  Serial.println("SD card Found");

  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    
    dataFile.println(filename);
    dataFile.close();
    // print to the serial port too:
    if(debug)
    Serial.println(filename);
  }
  }
  else{
  sdcard=false;
  Serial.println("No SD card");
  }
  connect_to_gs();
}
void loop() {
  // put your main code here, to run repeatedly:
  //main loop

  if(( WiFi.status() == WL_CONNECTED )){
  connected = true;
  }
  else{
  connected = false;
  }
  fData.Connection = connected;
  if(connected){
   sync_clock();
  }
  
  DateTime timestamp = get_rtc_time();
fData.Year=timestamp.year();
fData.Month=timestamp.month();
fData.Date=timestamp.day();
fData.Day=timestamp.dayOfTheWeek();
fData.H=timestamp.hour();
fData.M=timestamp.minute();
fData.S=timestamp.second();
//get pinState A0, D0 
fData.Analog0 = (float)(analogRead(A0)/1024)*3.3;
fData.Digital0 = digitalRead(D0);
  
  // send incomplete struct to nano
  //SerialprintDataStruct(Data);
  //implement log_local() here
///
if(sdcard){
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
   if(sdebug)
Serial.println(SerialprintDataStruct(fData));
if (dataFile) {

  dataFile.println(SerialprintDataStruct(fData));
  dataFile.close();
}
  else {
    Serial.println("error opening datalog.txt");
  }
}
///
  //Serial.println(SerialprintDataStruct(fData));
    if(connected){
  log_cloud(SerialprintDataStruct(fData));//(TBI)
  }


  delay(1000);

}


bool connect_to_wifi(const char* ssid, const char* password)
{
  if (debug) {
    Serial.println("Wifi connecting to : ");
    Serial.println( ssid );
  }
  WiFi.begin(ssid, password);

  if ( WiFi.status() == WL_CONNECTED ) {
    if (debug) {
      Serial.println("connected");
      Serial.println("NodeMCU IP Address : ");
      Serial.println(WiFi.localIP() );
    }
    return true;
  }
  else {
    if (debug) {
      Serial.println("not connected");

    }
    return false;
  }
}

void get_ntp_time()
{
    int Year, Month, Date, Day,H, M, S;

  timeClient.update();
  if (debug){
    Year = timeClient.getYear();
    Month = timeClient.getMonth();
    Date = timeClient.getDate();
    Day = timeClient.getDay();
    H = timeClient.getHours();
    M = timeClient.getMinutes();
    S = timeClient.getSeconds();
    Serial.println("NTP time : ");
    Serial.print(Year);
    Serial.print('/');
    Serial.print(Month);
    Serial.print('/');
    Serial.print(Date);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[Day]);
    Serial.print(") ");
    Serial.print(H);
    Serial.print(':');
    Serial.print(M);
    Serial.print(':');
    Serial.print(S);
    Serial.println();
  }
}

DateTime get_rtc_time() {
  DateTime now = rtc.now();
  if (debug){
    Serial.println("RTC time : ");
    Serial.print(now.year());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.print(now.second());
    Serial.println();
    }
    return now;
    }
void set_rtc_time() {
  int Year, Month, Date, Day,H, M, S;
  get_ntp_time();
  Year = timeClient.getYear();
  Month = timeClient.getMonth();
  Date = timeClient.getDate();
  H = timeClient.getHours();
  M = timeClient.getMinutes();
  S = timeClient.getSeconds();
  rtc.adjust(DateTime(Year, Month, Date, H, M, S));

}

void sync_clock()
{
  if(debug)
  Serial.println("syncing clock");

  DateTime Time_RTC = get_rtc_time();
   int Year, Month, Date, Day,H, M, S;
  get_ntp_time();
  Year = timeClient.getYear();
  Month = timeClient.getMonth();
  Date = timeClient.getDate();
  H = timeClient.getHours();
  M = timeClient.getMinutes();
  S = timeClient.getSeconds();
  
  if(Year != Time_RTC.year() || Month != Time_RTC.month() || Date != Time_RTC.day() || H!=Time_RTC.hour() || M != Time_RTC.minute() || S != Time_RTC.second()){
  if (debug)
  Serial.println("Synced R");
  set_rtc_time();
  }
}


String SerialprintDataStruct(struct data d)

{  String dataString = "";
dataString += String("[");
dataString += String(d.Year);
dataString += String("-");
dataString += String(d.Month);
dataString += String("-");
dataString += String(d.Date);
dataString += String("-");
dataString += String(d.Day);
dataString += String("-");
dataString += String(d.H);
dataString += String("-");
dataString += String(d.M);
dataString += String("-");
dataString += String(d.S);
dataString += String("]");
dataString += String(",");
dataString += String(d.Digital0);
dataString += String(",");
dataString += String(d.Analog0);
//dataString += String("");
if(debug)
Serial.println(dataString);
return dataString;
}


  
void connect_to_gs()
{
   // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
 // Serial.print("Connecting to ");
 // Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       //Serial.println("Connected");
       break;
    }
    //else()
      //Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
   // Serial.print("Could not connect to server: ");
    //Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  }

void log_cloud(String datastr)
{
static bool flag = false;
if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    //Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + datastr + "\"}";
  
  // Publish data to Google Sheets
  //Serial.println("Publishing data...");
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
      //Serial.println(payload);

  }
  else{
    // do stuff here if publish was not successful
   // Serial.println("Error while connecting");
  }
}
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}
void showNewData() {
  if (newData) {
    if (newData){
    Serial.print("R: ");
    Serial.println(receivedChars);
    }

    newData = false;
  }
}
