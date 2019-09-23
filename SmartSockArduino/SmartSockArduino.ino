#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include<EEPROM.h>

#define DEBUG

bool relayPower = false, swState = false;
int power_int = EEPROM.read(0);
const int input= 6, sw = 7;
float time_period = 0, Voltage=0, Current=0, Frequency = 0, Power=0, Energy = 0, PF = 0;
char str[30];

PZEM004Tv30 pzem(&Serial1);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
//  EEPROM.update(0,0);
  Serial.begin(9600);
  Serial3.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  pinMode(input, INPUT);
  pinMode(sw, OUTPUT);
  digitalWrite(sw, HIGH);
}
//
void loop() {
#ifdef DEBUG
  if(Serial.available()){
    if(Serial.read()=='a'){
      toggle();
      Serial.println("Toggle");
    }
  }
#endif
  Voltage = pzem.voltage();
  Current = pzem.current();
  Energy  = pzem.energy();
  Power = pzem.power();
  PF = pzem.pf();
  Frequency = pzem.frequency();
  lcdPrint();
  echoOutput();
  sendRawData();
  delay(500);
  if (Voltage == NAN) {

    Serial.print("NO CURRENT WIW");
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
  dtostrf(Frequency, 5, 1, valBuffer);
  sprintf(str, "Freq:%s  PI:%5d", valBuffer, power_int);
  lcd.print(str);
  lcd.setCursor(0,2);
  dtostrf(Current, 7, 3, valBuffer);
  sprintf(str, "Amp:%s", valBuffer);
  lcd.print(str);
  lcd.setCursor(0,3);
  dtostrf(Energy, 4, 3, valBuffer);
  sprintf(str, "kWh:  %s",valBuffer);
  lcd.print(str);
}

void echoOutput() {
  if (Serial3.available()) {
    String str = Serial3.readString();
    if (str == "!")
    {
      toggle();
    #ifdef DEBUG
      Serial.println("toggled");
    #endif
    } else if (str.indexOf("192") > 0) {
      lcd.setCursor(0,0);
      lcd.print(str);
      Serial.println( str );
    }
    else {
      Serial.println( str );
    }
      
  }
}

void toggle() {
  relayPower = !relayPower;
  digitalWrite(sw, !relayPower);
}

void sendRawData() {
  Serial3.print('!');
  Serial3.print(Voltage);
  Serial3.print(',');
  Serial3.print(Current);
  Serial3.print(',');
  Serial3.print(Frequency);
  Serial3.print(',');
  Serial3.print(Energy, 3);
  Serial3.print(',');
  Serial3.print(power_int);
  Serial3.print(',');
  Serial3.print(relayPower);
  Serial3.print('\n');
}
