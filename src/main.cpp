#include <Arduino.h>
#include <ESP8266WiFi.h>         
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyAadn6mFyqZZk4Jg6Jv17M8jPfO9-gWpds"
#define DATABASE_URL "https://uergs-app-default-rtdb.firebaseio.com/"



WiFiManager wifiManager;
// Device Location config
FirebaseData fbdo; 
FirebaseAuth auth; // Firebase Authentication Object 
FirebaseConfig config;  // Firebase configuration Object 
String fuid = ""; // Firebase Unique Identifier 
unsigned long elapsedMillis = 0;  // Stores the elapsed time from device start up 
unsigned long elapsedMillis_reset_device = 0;  // Stores the elapsed time from device start up 
unsigned long update_interval = 5000; // The frequency of sensor updates to firebase, set to 5 seconds 
unsigned long update_interval_reset_device = 63000; 
int count = 0; 
bool isAuthenticated = false; // Store device authentication status
bool signupOK = false;


void firebase_init() 
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  Serial.println("------------------------------------");
  Serial.println("Sign up new user...");
  // Sign in to firebase Anonymously
  
  if(Firebase.signUp(&config, &auth, " ", "")){
    Serial.println("Success");
    isAuthenticated = true;// Set the database path where updates will be loaded for this device
    fuid = auth.token.uid.c_str();
  }
  else
  {
    Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str()); isAuthenticated = false;
  }
    // Assign the callback function for the long running token generation task, see addons/TokenHelper.h
    config.token_status_callback = tokenStatusCallback;// Initialise the firebase library
    Firebase.begin(&config, &auth);
}

int read_sensor_fake ()
{
  return count++ ;
}

void database_send_sensor_data() {
  // Check that 5 seconds has elapsed before, device is authenticated and the firebase service is ready.
  if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready())
  {
    elapsedMillis = millis();
    Serial.println("------------------------------------");
    Serial.println("Set int test...");// Specify the key value for our data and append it to our 
    String node = "uergs/consumo/energia/total";// Send the value our count to the firebase realtime database 
    if (Firebase.RTDB.set(&fbdo, node.c_str(), read_sensor_fake()))
    {
      // Print firebase server 
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println("ETag: " + fbdo.ETag());
      Serial.print("VALUE: ");
      printResult(fbdo); //see addons/RTDBHelper.h
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
}

void reset_device() {
  // Check that 5 seconds has elapsed before, device is authenticated and the firebase service is ready.
  if (millis() - elapsedMillis_reset_device > update_interval_reset_device && isAuthenticated && Firebase.ready())
  {
    elapsedMillis_reset_device = millis();
    Serial.println("------------------------------------");
    Serial.println("Pegando Boleano de reiniciar");// Specify the key value for our data and append it to our 
    String node = "uergs/dispositivo/reiniciar";// Send the value our count to the firebase realtime database 
    Firebase.RTDB.getBool(&fbdo,node.c_str());
    bool reset =  fbdo.boolData();
    Serial.println("Reset: "+reset);
    if (reset)
    {
      digitalWrite(D1,HIGH);
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println("ETag: " + fbdo.ETag());
      Serial.print("VALUE: ");
      printResult(fbdo); //see addons/RTDBHelper.h
      Serial.println("------------------------------------");
      Serial.println();
      Firebase.RTDB.setBool(&fbdo,node.c_str(),false);
      delay(1000);
      ESP.eraseConfig();
      delay(1000);
      ESP.restart();
    }
  }
}

void setup() {
    pinMode(D1,OUTPUT);
    Serial.begin(115200);
    delay(300);
    Serial.println("iniciando ....");
    //wifi
    bool res;
    res = wifiManager.autoConnect("PROJETO-UERGS");
    if(!res){
      Serial.println("Failed to connect");
      delay(3000);
      WiFi.disconnect();
      ESP.restart();
    } 

    Serial.println("connected...yeey :)");
    
    firebase_init();
}

void loop() {
  database_send_sensor_data();
  reset_device();
}