#include <LiquidCrystal_I2C.h>
#include<EEPROM.h>

bool power = false;
int power_int = EEPROM.read(0);
int ctr, input= 7, high_time, low_time, volts, amps, freq, kWh;
float time_period;
char str[30];

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);
  Serial3.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  pinMode(input, INPUT);
}
//
void loop() {
  high_time = pulseIn(input, HIGH);
  low_time = pulseIn(input, LOW);
  time_period = high_time + low_time;
  time_period /= 273;
  amps = random(0,100);
  volts = random(0,250);
  freq = random(40,70);
  kWh = (volts*amps)/1000;
  lcdPrint();
  echoOutput();
  sendRawData();
  delay(500);

}

void lcdPrint(){
  lcd.setCursor(0,0);
  sprintf(str, "Volts:%4d Freq:%4d", volts, freq);
  lcd.print(str);
  lcd.setCursor(0,1);
  sprintf(str, "Amps:%5d kWh:%5d", amps, kWh);
  lcd.print(str);
  lcd.setCursor(0,2);
  sprintf(str, "P.I:%6d", power_int);
  lcd.print(str);
  lcd.setCursor(0,3);
   sprintf(str, " ");
  lcd.print(str);
}

void echoOutput() {
  if (Serial3.available()) {
    String str = Serial3.readString();
    if (str == "!")
    {
      toggle();
      Serial.println("toggled");
    } else {
      Serial.println( str );
    }
      
  }
}

void toggle() {
  power = !power;
}

void sendRawData() {
  Serial3.print('!');
  Serial3.print(volts);
  Serial3.print(',');
  Serial3.print(amps);
  Serial3.print(',');
  Serial3.print(freq);
  Serial3.print(',');
  Serial3.print(kWh);
  Serial3.print(',');
  Serial3.print(power_int);
  Serial3.print(',');
  Serial3.print(power);
  Serial3.print('\n');
}
