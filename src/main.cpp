#include <Arduino.h>
#include <SoftwareSerial.h>
#include <OneButton.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverEncoder.h>

#include <main.h>
#include <fan.h>
#include <lights.h>

SoftwareSerial mySerial(TX_PIN, RX_PIN);
Encoder enc(ENC_A, ENC_B, BUTT_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Fan fan(FAN_PIN);
Lights leds(RED_PIN, GREEN_PIN, BLUE_PIN);

typedef enum {
  mMain,
  mSetting
} Mode;

typedef enum {
  msmLights = 0,
  msmBright,
  msmFan
} MainSettingMode;

typedef struct {
  uint8_t cpu_temp;
  uint8_t gpu_temp;
  uint8_t mb_temp;
  uint8_t hdd_temp;
  uint8_t cpu_load;
  uint8_t gpu_load;
  uint8_t ram_use;
  uint8_t gpu_mem_use;
  uint8_t fan_max;
  uint8_t fan_min;
  uint8_t temp_max;
  uint8_t temp_min;
  uint8_t fan_manual;
  uint8_t color_manual;
  uint8_t fan_ctrl;
  uint8_t color_ctrl; 
  uint8_t bright_ctrl;
} OHMInfo;

typedef union {
  OHMInfo info;
  byte data[16];
} PCInfo;

PCInfo info;
Mode mode = mMain;
MainSettingMode main_sett_mode = msmLights;

byte charGrad[] = {
  0x1C,
  0x14,
  0x1C,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};

byte charFan[] = {
  0x00,
  0x1D,
  0x05,
  0x1F,
  0x14,
  0x17,
  0x00,
  0x00
};

byte charBright[] = {
  0x00,
  0x15,
  0x0E,
  0x1B,
  0x0E,
  0x15,
  0x00,
  0x00
};

byte charTemp[] = {
  0x04,
  0x11,
  0x12,
  0x11,
  0x15,
  0x09,
  0x0A,
  0x1F
};

uint8_t bright = 50;
bool isSelect;
bool isCurs;

unsigned long timer_curs;
unsigned long timer_info;
// парсинг
char inData[82];       // массив входных значений (СИМВОЛЫ)
byte index = 0;
String string_convert;



void parse(PCInfo *info){
  while (Serial.available() > 0) {
    char aChar = Serial.read();
    if (aChar != 'E') {
      inData[index] = aChar;
      index++;
      inData[index] = '\0';
    } else {
      char *p = inData;
      char *str;
      index = 0;
      String value = "";
      while ((str = strtok_r(p, ";", &p)) != NULL) {
        string_convert = str;
        info->data[index] = string_convert.toInt();
        index++;
      }
      index = 0;
    }
  }
}



void showInfo(PCInfo *info){
  // Вывод информации о подсветке
  lcd.setCursor(1,0); lcd.print(leds.getMode()); 
  lcd.setCursor(1,1); lcd.write(2); lcd.print("=");
  lcd.print(map(bright, 0, 255, 0, 100)); lcd.print("%");

  // Вывод информации о вентиляторах
  lcd.setCursor(10,0); lcd.write(3); lcd.print("="); lcd.print(info->info.cpu_temp); lcd.write(0);
  lcd.setCursor(10,1); lcd.write(1); lcd.print(":"); lcd.print(fan.getMode()); 

  // TOASK: Как сделать main_sett_mode = main_sett_mode + 1;

  if(!isSelect){
    if(enc.isRight()){
      switch(main_sett_mode){
        case msmLights: main_sett_mode = msmBright; break;
        case msmBright: main_sett_mode = msmFan; break;
        case msmFan: break;
      }
      lcd.clear();
    }
    else if(enc.isLeft()){
      switch(main_sett_mode){
        case msmLights: break;
        case msmBright: main_sett_mode = msmLights; break;
        case msmFan: main_sett_mode = msmBright; break;
      }
      lcd.clear();
    }

    if(enc.isRelease()){
      isSelect = true;
    }
  }

  else if(isSelect){
    if(enc.isHold()){
        isSelect = false;
    }
  }

  switch(main_sett_mode){ // Переключение между режимами настроек
    case msmLights: 
    // Если мы еще не выбрали, какой режим настраивать(кнопка не была нажата).
    if(!isSelect){
      // Маргаем курсором
      lcd.setCursor(0,0);
      if(millis() - timer_curs > 500){
        isCurs = !isCurs;
        timer_curs = millis();
      }
      if(isCurs){
        lcd.print(">"); 
      }
      else{
        lcd.print(" "); 
      }
    }

    // Если выбрали режим(кнопка была нажата)
    else{
      lcd.setCursor(0,0);
      lcd.print(">"); 
      // Настройка выбранного режима
      if(enc.isLeft()){
        leds.prevMode();
        lcd.clear();
      }
      else if(enc.isRight()){
        leds.nextMode();
        lcd.clear();
      }
    }
    break;
  
    case msmBright: // Настройка яркости
      // Если мы еще не выбрали, какой режим настраивать(кнопка не была нажата).
      if(!isSelect){
        // Маргаем курсором
        lcd.setCursor(0,1);
        if(millis() - timer_curs > 500){
          isCurs = !isCurs;
          timer_curs = millis();
        }
        if(isCurs){
          lcd.print(">"); 
        }
        else{
          lcd.print(" "); 
        }
      }

      // Если выбрали режим(кнопка была нажата)
      else{
        lcd.setCursor(0,1);
        lcd.print(">"); 
        // Настройка выбранного режима
        bright = constrain(bright, 0, 255);
        if(enc.isLeft()){
          bright-= 5;
          lcd.clear();
        }
        else if(enc.isRight()){
          bright+= 5;
          lcd.clear();
        }
        leds.setBrightness(bright);
      }
      break;

    case msmFan: // Настройка яркости
      // Если мы еще не выбрали, какой режим настраивать(кнопка не была нажата).
      if(!isSelect){
        // Маргаем курсором
        lcd.setCursor(9,1);
        if(millis() - timer_curs > 500){
          isCurs = !isCurs;
          timer_curs = millis();
        }
        if(isCurs){
          lcd.print(">"); 
        }
        else{
          lcd.print(" "); 
        }
      }

      // Если выбрали режим(кнопка была нажата)
      else{
        lcd.setCursor(9,1);
        lcd.print(">"); 
        // Настройка выбранного режима
        if(enc.isLeft()){
          fan.prevMode();
          lcd.clear();
        }
        else if(enc.isRight()){
          fan.nextMode();
          lcd.clear();
        }
      }
      break;
  }
}



void showSett(){

}


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, charGrad);
  lcd.createChar(1, charFan);
  lcd.createChar(2, charBright);
  lcd.createChar(3, charTemp);

  lcd.setCursor(4,0); lcd.print("Who Man");
  lcd.setCursor(2,1); lcd.print("Technologies");
  delay(2000);
  lcd.clear();
  
  enc.setType(TYPE1);
  enc.tick();
  if(enc.isHold()){ 
    mode = mSetting;
  }
}



void loop() {
  enc.tick();
  leds.tick(info.info.cpu_temp, info.info.gpu_temp);
  fan.tick(info.info.cpu_temp, info.info.gpu_temp);
  if(millis() - timer_info > 100){
    parse(&info);
    timer_info = millis();
  }

  switch(mode){
    case mMain: // Если включен рабочий режим
      showInfo(&info); // Отображаем основную информацию
      break;

    case mSetting:
      showSett(); // Показываем настройки
      break;
  }  
}


