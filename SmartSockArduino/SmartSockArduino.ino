#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include<EEPROM.h>

#define DEBUG

volatile bool relayPower = true;
int power_int = EEPROM.read(0);
const uint8_t input= 6, sw = 7, btnToggle = 46, btnReset = 42;
float time_period = 0.0, Voltage=0.0, Current=0.0, Frequency = 0.0, Power=0.0, Energy = 0.0, PF = 0.0;
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
  pinMode(btnToggle, INPUT_PULLUP);
  pinMode(btnReset, INPUT_PULLUP);
  pinMode(sw, OUTPUT);
  digitalWrite(sw, LOW);
  attachInterrupt(digitalPinToInterrupt(btnToggle), btn_toggle_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(btnReset), btn_reset_handler, RISING);
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
  updateValues();
  lcdPrint();
  echoOutput();
  sendRawData();
  delay(500);
}

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
  sprintf(str, "Freq:%s PI:%5d", valBuffer, power_int);
  lcd.print(str);
  lcd.setCursor(0,2);
  dtostrf(Current, 6, 3, valBuffer);
  sprintf(str, "Amp: %s         ", valBuffer);
  lcd.print(str);
  lcd.setCursor(0,3);
  dtostrf(Energy, 4, 3, valBuffer);
  sprintf(str, "kWh:  %s        ",valBuffer);
  lcd.print(str);
}

void echoOutput() {
  if (Serial3.available()) {
    String str = Serial3.readString();
    if (str.indexOf('\x1A') >= 0)
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

void btn_toggle_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 250)
 {
   toggle();
 }
 last_interrupt_time = interrupt_time;
}

void btn_reset_handler()
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 250)
 {
   pzem.resetEnergy();
 }
 last_interrupt_time = interrupt_time;
}
