//Created by: Slavko Zdravevski

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

const char* ssid = "YOUR SSID"; // your name of your car's wifi
const char* password = "YOUR PASSWORD"; // your password of your car's wifi
ESP8266WebServer server(80);

int lockingOptocoupler = D5;
int unlockingOptocoupler = D6;
bool serverStateChanged = false;
bool carIsLocked = false;
bool lightsAreOn = false;

int prepareEngineForStart = D1;
int prepareAlternator = D7;
int startEngine = D0;
int lights = D2;

int remoteState;

void setup() {
  Serial.begin(115200);
  
  pinMode(lockingOptocoupler, OUTPUT);
  pinMode(unlockingOptocoupler, OUTPUT);
  turnOffOptocouplers();

  pinMode(prepareEngineForStart, OUTPUT);
  pinMode(prepareAlternator, OUTPUT);
  pinMode(startEngine, OUTPUT);
  pinMode(lights, OUTPUT);
  digitalWrite(prepareEngineForStart, HIGH);
  digitalWrite(prepareAlternator, LOW);
  digitalWrite(startEngine, HIGH);
  digitalWrite(lights, HIGH);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on ( "/", HTTP_handleRoot );
  server.onNotFound ( HTTP_handleRoot );
  server.begin();  
}

void loop() {
  if (serverStateChanged) {
    if (remoteState == 1) {
      carIsLocked = !carIsLocked;
      lockOrUnlockCar();
    } else if (remoteState == 2) {
      Serial.println("Prepare the engine");
      digitalWrite(prepareEngineForStart, LOW);
      digitalWrite(prepareAlternator, HIGH);
      digitalWrite(startEngine, HIGH);
    } else if (remoteState == 3) {
      Serial.println("Start the engine");
      digitalWrite(prepareEngineForStart, LOW);
      digitalWrite(prepareAlternator, HIGH);
      digitalWrite(startEngine, LOW);
    } else if (remoteState == 4) {
      Serial.println("Keep the engine started");
      digitalWrite(prepareEngineForStart, LOW);
      digitalWrite(prepareAlternator, HIGH);
      digitalWrite(startEngine, HIGH);
      delay(1000);
      digitalWrite(lights, LOW);
    } else if (remoteState == 6) {
      turnLightsOnOrOff();
      lightsAreOn = !lightsAreOn;
    } else {
      Serial.println("Stop The Engine");
      digitalWrite(prepareEngineForStart, HIGH);
      digitalWrite(prepareAlternator, LOW);
      digitalWrite(startEngine, HIGH);
      digitalWrite(lights, HIGH);
    }
    
    serverStateChanged = false;
  }

  server.handleClient();
}

void lockOrUnlockCar () {
  Serial.println("Lock or unlock the car");
  if (carIsLocked) {
    digitalWrite(unlockingOptocoupler, HIGH);
  } else {
    digitalWrite(lockingOptocoupler, HIGH);
  }
  delay(1000);
  turnOffOptocouplers();
  delay(1000);
}

void turnLightsOnOrOff () {
  Serial.println("Turn lights on or off");
  if (lightsAreOn) {
    digitalWrite(lights, HIGH);
  } else {
    digitalWrite(lights, LOW);
  }
}

void turnOffOptocouplers () {
  digitalWrite(unlockingOptocoupler, LOW);
  digitalWrite(lockingOptocoupler, LOW);
}

void HTTP_handleRoot(void) {
  if(server.hasArg("state")){
    remoteState = server.arg("state").toInt();
    serverStateChanged = true;
  }
}
