#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG
#define ESP Serial2
#define sim Serial3
#define NUM "+639503610262"

bool relayMain = true, relay1 = true, relay2 = true, relay3 = true, smsReady = false;
const uint8_t input= 6, pSwMain = 11, pSw1 = 10, pSw2 = 9, pSw3 = 8, btnToggle = 2, btnReset = 3;
float Voltage=0.0, Current=0.0, Frequency = 0.0, Power=0.0, Energy = 0.0, PF = 0.0;
char str[30], ip[4][5] = {};

PZEM004Tv30 pzem(&Serial1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  sim.begin(9600);
  ESP.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.noCursor();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Booting up");
  delay(4000);
  pinMode(input, INPUT);
  pinMode(btnToggle, INPUT);
  pinMode(btnReset, INPUT);
  pinMode(pSwMain, OUTPUT);
  pinMode(pSw1, OUTPUT);
  pinMode(pSw2, OUTPUT);
  pinMode(pSw3, OUTPUT);
  digitalWrite(pSwMain, !relayMain);
  digitalWrite(pSw1, !relay1);
  digitalWrite(pSw2, !relay2);
  digitalWrite(pSw3, !relay3);
  #ifdef INTERRUPT
  attachInterrupt(digitalPinToInterrupt(btnToggle), btn_toggleMain_handler, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnReset), btn_reset_handler, FALLING);
  #endif
  clearIP();
  sim.print("AT+ CMGF=1\r");
  sim.print("AT+CNMI=2,2,0,0,0\r");
  sim.print("AT+CMGD=1,3\r");
}
//
void loop() {
#ifdef DEBUG
  if(Serial.available()){
    if(Serial.read()=='a'){
      btn_toggleMain_handler();
      Serial.println("Toggle");
    }
  }
#endif
#ifndef INTERRUPT
  scanButtons();
#endif

  updateValues();
  lcdPrint();
  listenToESP();
  recv_msg();
  checkTime();
  sendRawData();
  delay(100);
}

#ifndef INTERRUPT
void scanButtons() {
  static bool lastStateReset = 0;
  static bool lastStateToggle = 0;

  bool currentStateReset = digitalRead(btnReset);
  bool currentStateToggle = digitalRead(btnToggle);
  if (lastStateReset != currentStateReset)
  {
    if (currentStateReset == 0) {
      btn_reset_handler();
    }
    lastStateReset = currentStateReset;
  }
  if (lastStateToggle != currentStateToggle)
  {
    if (currentStateToggle == 0) {
      btn_toggleMain_handler();
    }
    lastStateToggle = currentStateToggle;
  }
}
#endif

void updateValues() {
  Voltage = pzem.voltage();
  if (!isnan(Voltage))
  {
    Current = pzem.current();
    Energy  = pzem.energy();
    Power = pzem.power();
    PF = pzem.pf();
    Frequency = pzem.frequency();
  } else {
    Voltage = 0.0;
    Current = 0.0;
    Power = 0.0;
    PF = 0.0;
    Frequency = 0.0;
  }
}

void lcdPrint(){
  char valBuffer[15];
  char valBuffer2[15];
  lcd.setCursor(0,0);
  dtostrf(Voltage, 6, 1, valBuffer);
  dtostrf(PF, 5, 2, valBuffer2);
  sprintf(str, "Volt:%s PF:%s", valBuffer, valBuffer2);
  lcd.print(str);
  lcd.setCursor(0,1);
  dtostrf(Frequency, 6, 1, valBuffer);
  sprintf(str, "Freq:%s", valBuffer);
  
  lcd.print(str);
  lcd.setCursor(19,1);
  lcd.print((smsReady) ? '+' : '-');
  lcd.setCursor(0,2);
  dtostrf(Current, 6, 3, valBuffer);
  sprintf(str, "Amp: %s %-3s.%-3s.", valBuffer, ip[0], ip[1]);
  lcd.print(str);
  lcd.setCursor(0,3);
  dtostrf(Energy, 4, 3, valBuffer);
  sprintf(str, "kWh:  %s %-3s.%-4s",valBuffer, ip[2], ip[3]);
  lcd.print(str);
}

void listenToESP() {
  if (ESP.available()) {
    String str = "";
    while(ESP.available()){
      char temp = (char)ESP.read();
      if (temp == '\n') {
        break;
      } else {
        str+=temp;
      }
    }
    if (str.indexOf('\x1A') >= 0)
    {
      btn_toggleMain_handler();
    #ifdef DEBUG
      Serial.println("toggled main");
    #endif
    } else if (str.indexOf('\x1C') >= 0) {
      toggleSw1();
    } else if (str.indexOf('\x1D') >= 0) {
      toggleSw2();
    } else if (str.indexOf('\x1E') >= 0) {
      toggleSw3();
    } else if (str.indexOf('\x19') >= 0) {
      btn_reset_handler();
    #ifdef DEBUG
      Serial.println("resetted energy");
    #endif
    } else if (str.indexOf('\x12') >= 0) {
        // disconnected
        clearIP();
    } else if (str.indexOf('\x11') >= 0) {
      char * strPtr = strtok( str.c_str() + 1 , ".");
      int i = 0;
      while (i < 4 && strPtr != NULL) {
        #ifdef DEBUG
        Serial.println(strPtr);
        #endif
        strcpy( ip[i], strPtr);
        strPtr = strtok(NULL, ".");
        i++;
      }
    }
    else {
      Serial.println( str );
    }
      
  }
}

void clearIP() {
  for (int i = 0; i < 4; i++) {
    strcpy(ip[i], "0");
  }
}

void sendRawData() {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > 250) {
    lastMillis = currentMillis;
    
    ESP.print('!');
    ESP.print(Voltage);
    ESP.print(',');
    ESP.print(Frequency);
    ESP.print(',');
    ESP.print(Current, 3);
    ESP.print(',');
    ESP.print(Energy, 3);
    ESP.print(',');
    ESP.print(PF);
    ESP.print(',');
    ESP.print(relayMain);
    ESP.print(',');
    ESP.print(relay1);
    ESP.print(',');
    ESP.print(relay2);
    ESP.print(',');
    ESP.print(relay3);
    ESP.print('\n');
  }
  
}

void btn_toggleMain_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500)
 {
   relayMain = !relayMain;
   digitalWrite(pSwMain, !relayMain);
  
  
   #ifdef DEBUG
   Serial.println("t: triggered toggle main interrupt");
   #endif
 }
 last_interrupt_time = interrupt_time;
}

void toggleSw1()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500)
 {
   relay1 = !relay1;
   digitalWrite(pSw1, !relay1);
  
  
   #ifdef DEBUG
   Serial.println("t: toggled switch 1");
   #endif
 }
 last_interrupt_time = interrupt_time;
}

void toggleSw2()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500)
 {
   relay2 = !relay2;
   digitalWrite(pSw2, !relay2);
  
  
   #ifdef DEBUG
   Serial.println("t: toggled switch 2");
   #endif
 }
 last_interrupt_time = interrupt_time;
}

void toggleSw3()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500)
 {
   relay3 = !relay3;
   digitalWrite(pSw3, !relay3);
  
  
   #ifdef DEBUG
   Serial.println("t: toggled switch 3");
   #endif
 }
 last_interrupt_time = interrupt_time;
}

void btn_reset_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 2000)
 {
   pzem.resetEnergy();
   
   #ifdef DEBUG
   Serial.println("rtriggered reset interrupt");
   #endif
 }
 last_interrupt_time = interrupt_time;
}

void recv_msg() {
  static String textMessage = "";
  if (sim.available()) {
    textMessage = sim.readString();
    #ifdef DEBUG
    Serial.println(textMessage);
    #endif
    delay(10);
    textMessage.toLowerCase();
    if (textMessage.indexOf("+cmgf: 0") > 0 || textMessage.indexOf("+cmti") > 0 ) {
      sim.print("AT+ CMGF=1\r");
      sim.print("AT+CNMI=2,2,0,0,0\r");
    }
    if (textMessage.indexOf("+cmt") > 0 || textMessage.indexOf("+cmti") > 0) {
      sim.print("AT+CMGD=1,3\r"); //deletes recv read sms
      smsReady = true;
    }
    if (textMessage.indexOf("consumption") > 0) {
      send_msg("Power Consumption: " + String(pzem.energy(), 3) + " Kwh", NUM);
    } else if (textMessage.indexOf("off1") > 0) {
      relay1 = false;
    } else if (textMessage.indexOf("off2") > 0) {
      relay2 = false;
    } else if (textMessage.indexOf("off3") > 0) {
      relay3 = false;
    } else if (textMessage.indexOf("on1") > 0) {
      relay1 = true;
    } else if (textMessage.indexOf("on2") > 0) {
      relay2 = true;
    } else if (textMessage.indexOf("on3") > 0) {
      relay3 = true;
    } else if (textMessage.indexOf("off") > 0) {
      relayMain = false;
    } else if (textMessage.indexOf("on") > 0) {
      relayMain = true;
    } else if (textMessage.indexOf("sms ready") > 0) {
      smsReady = true;
    }
    digitalWrite(pSwMain, !relayMain);
    digitalWrite(pSw1, !relay1);
    digitalWrite(pSw2, !relay2);
    digitalWrite(pSw3, !relay3);
  }
}

void checkTime() {
  static bool wasSmsReady = false;
  if (!wasSmsReady && smsReady) {
    wasSmsReady = true;
    send_msg("Power Consumption: " + String(pzem.energy(), 3) + " Kwh", NUM);
  }
  
  static unsigned int prevMillis = 0;
  unsigned int currMillis = millis();
  if (currMillis - prevMillis > 86400000) {
    prevMillis = currMillis;
    send_msg("Power Consumption: " + String(pzem.energy(), 3) + " Kwh", NUM);
  }
}

void send_msg(String txt, String number) {
  #ifdef DEBUG
  Serial.println("Sending SMS...");
  #endif
  if (smsReady) {
    sim.print("AT+CMGF=1\r");
    delay(50);
    sim.println("AT+CMGS=\"" + number + "\"");
    delay(50);
    sim.println(txt);
    delay(50);
    sim.println((char)26);
    delay(50);
    sim.println();
    #ifdef DEBUG
    Serial.println("Send SMS done");
    #endif
  }
}
