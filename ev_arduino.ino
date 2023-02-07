#define temp_Pin A1
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <stdlib.h>

//fingerprint-----------------
SoftwareSerial mySerial(2, 3); // rx, tx
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//LCD-----------------
int totalColumns = 20;
int totalRows = 4;
LiquidCrystal_I2C lcd(0x3F, totalColumns, totalRows);  
String lock_status ="BIKE LOCKED";
//SDA A4 SCL A5

//Vibration---------------
int vibrationPin = 9;

//Relay------------------- relay to turn off bike controller 
int relayPin=8;

//GSM--------------------
SoftwareSerial SIM900A(10,11);//RX | TX

// GPS-------------------
TinyGPSPlus gps;
SoftwareSerial ss(5, 4); //RX | TX 
float latitude , longitude;
String gmap;//for gmap link
String strlati ="";// for storing the longittude value in str
String strlong ="";//for storing the latitude value
char buff[10];


//BUTTON------------------
const int buttonPin = 7; // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

//BUZZER-----------------
const int buzzerPin = 12;


void setup()
{
  
  Serial.begin(9600);
  //LCD---------------------------
  lcd.init();                    
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(lock_status);
  
  //Fingerprint-------------------
  // set the data rate for the sensor serial port
  finger.begin(57600);

  //Relay---------------------------
  pinMode(relayPin, OUTPUT); // RELAY IS INITIALLY LOW, NEED TO TURN IT TO HIGH
  digitalWrite(relayPin, HIGH); //HIGH FOR RELAY OFF, LOW FOR RELAY ON
  //Vibration------------------------
  pinMode(vibrationPin , INPUT);

  //BUTTON----------------------------
  pinMode(buttonPin, INPUT);
  
  //BUZZER----------------------------
  pinMode(buzzerPin, OUTPUT);

  //GSM-------------------------------
  //SIM900A.begin(9600);   // Setting the baud rate of GSM Module

  //GPS-------------------------------
  //ss.begin(9600); //Setting GPS baud rate
}

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0. For manual button reset.

void loop(){                     
  
  int fingerprint = check_fingerprint();
  if (fingerprint == FINGERPRINT_OK){
    digitalWrite(relayPin, LOW);
    lcd.clear();    
    lcd.print("BIKE UNLOCKED");
    tone(buzzerPin, 10000,500);
    delay(1000);
    noTone(buzzerPin);
    delay(1000);    
  }    
  //temp
  temp();
  //vibration-------------------
  long measurement = vibration();
  if (measurement > 1000){
    digitalWrite(relayPin ,HIGH);
    tone(buzzerPin, 10000,500);
    delay(1000);
    noTone(buzzerPin);
    delay(1000);    
    lcd.clear();
    lcd.print("CRASH DETECTED");
    delay(2500);
    lcd.clear(); 
    lcd.print("GETTING GPS");
    latitude, longitude = GPS();
    strlati = dtostrf(latitude, 3, 6, buff);
    strlong = dtostrf(longitude, 3, 6, buff);
    gmap = gmap + "http://www.google.com/maps/place/" + strlati + "," + strlong ;
    lcd.clear();
    lcd.print("GPS FOUND");
    delay(2500);
    lcd.clear();
    lcd.print("SENDING SMS");
    delay(2500);
    SendMessage();
    lcd.clear();
    lcd.print(" SMS SENT");
    delay(2500);
    lcd.clear();
    lcd.print("BIKE LOCKED");
    resetFunc(); //call reset
    }
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
        resetFunc(); //call reset
  } 
}

 void temp(){
  // Get the voltage reading from the LM35
  int reading = analogRead(temp_Pin);

  // Convert that reading into voltage
  float voltage = reading * (5.0 / 1024.0);

  // Convert the voltage into the temperature in Celsius
  float temperatureC = voltage * 100;
  lcd.setCursor(0, 3);
  lcd.print("BATTERY TEMP:");
  lcd.print(temperatureC);
  lcd.print((char)223);
  lcd.print("C");
}
//FINGERPRINT----------------------------------------------
int check_fingerprint (){
  uint8_t p = finger.getImage();
  p = finger.image2Tz();
  p = finger.fingerSearch();
  return p;
}
//LCD SCROL-------------------------------------------------
void scrollMessage(int row, String message, int delayTime, int totalColumns) {
  for (int i=0; i < totalColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int position = 0; position < message.length(); position++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(position, position + totalColumns));
    delay(delayTime);
  }
}
//vibration----------------------------------------------------
  long vibration(){
  //pinMode(vibrationPin , INPUT);
  long measurement = pulseIn (vibrationPin ,HIGH);
  Serial.println("vibration =");
  Serial.println(measurement);
  return measurement;
}
//GSM-----------------------------------------------------------
void SendMessage()
{ 
  SIM900A.begin(9600);
  SIM900A.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  SIM900A.println("AT+CMGS=\"PHONE NUMBER\"\r"); //Mobile phone number to send messag
  SIM900A.println("CRASH DETECTED IN \"USER NAME\" BIKE");// Messsage content
  Serial.println(gmap);  
  SIM900A.println(gmap);
  SIM900A.println((char)26);// ASCII code of CTRL+Z
}
//GPS------------------------------------------------------------

float GPS()
{
  int k=0; 
  ss.begin(9600);    
  while(k==0){ 
    while (ss.available() > 0){
      gps.encode(ss.read());
      if (gps.location.isUpdated()){
        Serial.print("Latitude= "); 
        Serial.print(gps.location.lat(), 6);
        latitude = gps.location.lat();
        Serial.print(" Longitude= "); 
        Serial.println(gps.location.lng(), 6);
        longitude = gps.location.lng();
        k=1;
        return latitude, longitude;  
    }
  }  
  }
}
//reset function
//void(* resetFunc) (void) = 0;  // declare reset fuction at address 0
