#include <M5StickCPlus.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Preferences.h>

#define LED_PIN 10

char ssid[] = "myWifi";
char password[] = "secret";

WiFiUDP udp;
const int localPort = 12033; // input port
String remoteHost = "192.168.3.100";
const int remotePort = 12034; // output port

float power = 0;
Preferences preferences;

bool lastButtonBState = false;
long lastButtonBPress = 0;
bool lastButtonAState = false;
long lastButtonAPress = 0;

char buffer[256];
int bufferIndex = 0;



void setup() {
  M5.begin();    
  
  Serial.begin(115200);
  preferences.begin("config");
  delay(10);
  
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Lcd.drawFastHLine(0, 15, 320, TFT_DARKGREY);

  remoteHost = preferences.getString("REMOTE_IP", remoteHost);

  connect();
}



void loop() {
  M5.update();

  checkButtonA(); // big button
  checkButtonB(); // left-top button
  checkButtonAxp(); // right-bottom button

  receiveOSC();
  updateBattery();
  
  delay(10);
}


void checkButtonA() {
  if (M5.BtnA.isPressed()) {
    if (!lastButtonAState) {
      lastButtonAState = true;
      lastButtonAPress = millis();
    }
  } else if (lastButtonAState) {
    if (lastButtonAPress + 500 < millis()) {
      M5.Lcd.fillRect(155, 40, 14, 14, TFT_RED);
      sendStop();
      delay(300);
    } else {
      sendTogglePlay();
    }
    
    lastButtonAState = false;
  }  
}

void checkButtonAxp() {
  if (M5.Axp.GetBtnPress()) {
    sendNext();
  }   
}

void checkButtonB() {
  if (M5.BtnB.isPressed()) {
    if (!lastButtonBState) {
      lastButtonBState = true;
      lastButtonBPress = millis();
    }
  } else if (lastButtonBState) {
    if (lastButtonBPress + 500 < millis()) {
      sendPrevCue();
    } else {
      sendPrev();
    }
    
    lastButtonBState = false;
  }  
}

void updateBattery() {
  power = M5.Axp.GetBatVoltage();
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  
  M5.Lcd.setCursor(180, 5);
  M5.Lcd.printf(" %5.2f V", power);
}


void connect()
{
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.println("Connecting to: " + (String)ssid);
    
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println((String) ssid);
    Serial.println((String) password);

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19dBm);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    udp.begin(localPort);
    udp.flush();

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.drawFastHLine(0, 15, 320, TFT_DARKGREY);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.println(WiFi.localIP().toString() + "           ");

    delay(500);
}

void sendPlay() {
  OSCMessage msg("/play");
  sendMessage(msg);
}

void sendTogglePlay() {
  OSCMessage msg("/togglePlay");
  sendMessage(msg);
}

void sendStop() {
  OSCMessage msg("/stop");
  sendMessage(msg);
}

void sendNext() {
  OSCMessage msg("/next");
  sendMessage(msg);
}

void sendNextCue() {
  OSCMessage msg("/nextCue");
  sendMessage(msg);
}

void sendPrev() {
  OSCMessage msg("/prev");
  sendMessage(msg);
}

void sendPrevCue() {
  OSCMessage msg("/prevCue");
  sendMessage(msg);
}

void sendMessage(OSCMessage &msg) {
    if (remoteHost.length() == 0)
    {
        Serial.println("Send OSC message, remoteHost not set");
        return;
    }
    char addr[32];
    msg.getAddress(addr);
    //Serial.println("Send OSC message to "+ remoteHost + " : " + String(addr));
    udp.beginPacket((char *)remoteHost.c_str(), (uint16_t)remotePort);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
}

void receiveOSC() {
    OSCMessage msg;
    int size;
    if ((size = udp.parsePacket()) > 0)
    {
        while (size--)
            msg.fill(udp.read());
        if (!msg.hasError())
            processMessage(msg);
    }
}

void processMessage(OSCMessage &msg) {
    if (msg.match("/yo")) {
       char hostData[32];
       msg.getString(0, hostData, 32);
        
       Serial.println("Yo received : "+String(hostData));
        
       remoteHost = hostData;
       preferences.putString("REMOTE_IP", remoteHost);

       OSCMessage msg("/wassup");
       msg.add(WiFi.localIP().toString().c_str());

       sendMessage(msg);
    } else if (msg.match("/sleep")) {
       M5.Axp.PowerOff();
    } else if (msg.match("/currentIndex")) {
        if (msg.isInt(0)){
          int currentIndex = msg.getInt(0);
          displayIndex(currentIndex);
        }
    } else if (msg.match("/currentName")) {
        if (msg.isString(0)){
          char currentName[32];
          msg.getString(0, currentName, 32);
          displayName(currentName);
        }
    } else if (msg.match("/status")) {
        if (msg.isInt(0)){
          int currentIndex = msg.getInt(0);
          displayIndex(currentIndex);
        }
        if (msg.isString(1)){
          char currentName[32];
          msg.getString(1, currentName, 32);
          displayName(currentName);
        }
        if (msg.isString(2)){
          char currentTime[32];
          msg.getString(2, currentTime, 32);
          int isPlaying = msg.getInt(3);
          displayTime(currentTime, isPlaying);
        }
    } else {
      Serial.println("Blup");
    }
}

void displayIndex(int index) {
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(20, 35);
    M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    M5.Lcd.println(String(index) + "  ");
}

void displayTime(char* time, int isPlaying) {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(180, 40);

    if (isPlaying) {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
    }

    if (!lastButtonAState) {
      if (isPlaying) {
        M5.Lcd.fillRect(155, 40, 14, 14, TFT_BLACK);
        M5.Lcd.fillTriangle(155,40,155,53,165,47,TFT_GREEN);
      } else {
        M5.Lcd.fillRect(155, 40, 14, 14, TFT_BLACK);
        M5.Lcd.fillRect(155, 40, 3, 14, TFT_ORANGE);
        M5.Lcd.fillRect(162, 40, 3, 14, TFT_ORANGE);
      }
    }

    M5.Lcd.println(String(time) + " ");
}

void displayName(char* name) {
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(5, 70);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    M5.Lcd.println(String(name) + "               ");
}
