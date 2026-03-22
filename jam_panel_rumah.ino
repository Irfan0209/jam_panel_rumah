

//SETUP DMD
#define DISPLAYS_WIDE 2
#define DISPLAYS_HIGH 1

#include <DMD3asis.h>

#include <EEPROM.h>
DMD3  Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)

#include <Wire.h>
#include <RtcDS3231.h>


#include <Prayer.h>


#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/SystemFont5x7.h>
#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/Font4x6.h>
#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/System4x7.h>
#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/SmallCap4x6.h>
#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/EMSans8x16.h>
#include <C:\Users\irfan\Documents\Project\jws_masjid_AL-MA_ANY\fonts/BigNumber.h>


#define BUZZ  2 // PIN BUZZER

#define Font0 SystemFont5x7
#define Font1 Font4x6
#define Font2 System4x7 
#define Font3 SmallCap4x6
#define Font4 EMSans8x16
#define Font5 BigNumber

//create object
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;

// Constractor
Prayer JWS;
Hijriyah Hijir;

uint8_t iqomah[]        = {1,1,1,1,1,1};
uint8_t displayBlink[]  = {1,1,1,1,1,1};
uint8_t dataIhty[]      = {3,4,2,3,3,0};

struct Config {
  uint8_t durasiadzan = 60;
  uint8_t altitude = 10;
  double latitude = -7.364057;
  double longitude = 112.646222;
  uint8_t zonawaktu = 7;
  int8_t Correction = -1; //Koreksi tanggal hijriyah, -1 untuk mengurangi, 0 tanpa koreksi, 1 untuk menambah
};

Config config;



// Variabel untuk waktu, tanggal, teks berjalan, tampilan ,dan kecerahan
char text1[101], text2[101],name[101];
uint8_t   brightness    = 160;
bool       adzan         = 0;
bool       stateBuzzer   = 1;
uint8_t    DWidth        = Disp.width();
uint8_t    DHeight       = Disp.height();
int8_t     sholatNow     = -1;
bool       reset_x       = 0; 

/*======library tambahan=======*/
bool       stateSendSholat = false; 
bool       stateBuzzWar    = 0;
bool       DoSwap          = false;
static uint32_t  lastRtcUpdate = 0;
 
#define SERIAL_BUF 32

char serialBuf[SERIAL_BUF];
uint8_t serialPos = 0;
/*============== end ================*/

enum Show{
  ANIM_SHOW,
  ANIM_SHOLAT,
  ANIM_ADZAN,
  ANIM_IQOMAH,
  ANIM_BLINK,
};

Show show = ANIM_SHOW;


//#define EEPROM_SIZE 512

// Alamat EEPROM
#define ADDR_TEXT1        0     // text1, max 100 bytes
#define ADDR_TEXT2       100   // text2, max 100 bytes
#define ADDR_BRIGHTNESS  200
#define ADDR_SPEEDTX1    202
#define ADDR_SPEEDTX2    204   // Tambahan untuk speed text 2
#define ADDR_SPEEDDT     206
#define ADDR_LATITUDE    208
#define ADDR_LONGITUDE   212
#define ADDR_TZ          216
#define ADDR_ALTITUDE    218
#define ADDR_IQOMAH      220  // 6 byte
#define ADDR_BLINK       226  // 6 byte
#define ADDR_IHTY        232  // 6 byte
#define ADDR_BUZZER      238
#define ADDR_PASSWORD    240  // 8 byte
#define ADDR_DURASIADZAN 248
#define ADDR_CORRECTION  250
#define ADDR_MODE        256
#define ADDR_SPEEDNAME   258
#define ADDR_NAME        260


void saveStringToEEPROM(int startAddr, String data, int maxLength) {
  for (int i = 0; i < maxLength; i++) {
    if (i < data.length()) {
      EEPROM.write(startAddr + i, data[i]);
    } else {
      EEPROM.write(startAddr + i, 0); // null terminate / padding
    }
  }
}

void saveFloatToEEPROM(int addr, float value) {
  byte *data = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(float); i++) {
    EEPROM.write(addr + i, data[i]);
  }
}

void saveIntToEEPROM(int addr, int16_t value) {
  EEPROM.write(addr, lowByte(value));
  EEPROM.write(addr + 1, highByte(value));
}

void handleSetTimeSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      serialBuf[serialPos] = '\0';
      parseCommand(serialBuf);
      serialPos = 0;
    }
    else if (serialPos < SERIAL_BUF - 1) {
      serialBuf[serialPos++] = c;
    }
  }
}

void parseCommand(char *cmd) {
  char *eq = strchr(cmd, '=');
  if (!eq) return;

  *eq = '\0';
  char *key = cmd;
  char *value = eq + 1;

  if (strcmp(key, "jadwal") == 0) {
    stateSendSholat = atoi(value);
  }
}


// =========================================
// DMD3 P10 utility Function================
// =========================================
void Disp_init() 
  { Disp.setDoubleBuffer(true);
    Timer1.initialize(1500);
    Timer1.attachInterrupt(scan);
    setBrightness(brightness);  
    Disp.clear();
    Disp.swapBuffers();
    }

void setBrightness(uint8_t bright)
  { Timer1.pwm(9,bright);}

void scan()
  { Disp.refresh();}


void setup() {
  Serial.begin(9600);
  EEPROM.begin();
  
  pinMode(BUZZ, OUTPUT); 
  digitalWrite(BUZZ,HIGH);
  delay(200);
  digitalWrite(BUZZ,LOW);
  int8_t rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
    if (rtn != 0) {
      Serial.println(F("I2C bus error. Could not clear"));
      if (rtn == 1) {
        Serial.println(F("SCL clock line held low"));
      } else if (rtn == 2) {
        Serial.println(F("SCL clock line held low by slave clock stretch"));
      } else if (rtn == 3) {
        Serial.println(F("SDA data line held low"));
      }
    } 
    else { // bus clear, re-enable Wire, now can start Wire Arduino master
      Wire.begin();
    }
  
  Rtc.Begin();
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  delay(1000);
  
  Serial.println("PANEL_OK");
  Disp_init();
  delay(1000);
  for(uint8_t i = 0; i < 4; i++)
   {
      Buzzer(1);
      delay(80);
      Buzzer(0);
      delay(80);
   }

}

void loop() {

  DoSwap = false;
  handleSetTimeSerial();
  check();
  islam();
  sendTimeEvery5Min();

  Disp.clear();

  switch (show) {
    case ANIM_SHOW:
      showAnimasi();
      break;

    case ANIM_ADZAN:
      drawAzzan();
      break;

    case ANIM_SHOLAT:
      // updateAnimSholat();
      break;
  }

  buzzerWarning(stateBuzzWar);
  if(DoSwap){Disp.swapBuffers();}
}


 //----------------------------------------------------------------------
// I2C_ClearBus menghindari gagal baca RTC (nilai 00 atau 165)

uint8_t I2C_ClearBus() {
  
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}

void buzzerWarning(int cek){

   static bool state = false;
   static uint32_t save = 0;
   uint32_t tmr = millis();
   static uint8_t con = 0;
    
    if(tmr - save > 2500 && cek == 1){
      save = tmr;
      state = !state;
      digitalWrite(BUZZ, state);
      //Serial.println("active");
      if(con <= 6) { con++; }
      if(con == 7) { cek = 0; con = 0; state = false; stateBuzzWar = 0; digitalWrite(BUZZ, LOW);}
      //Serial.println("con:" + String(con));
    } 
    
}

void Buzzer(bool state)
  {
    if(!stateBuzzer) return;
    
    switch(state){
      case 0 :
        digitalWrite(BUZZ,LOW);
      break;
      case 1 :
        digitalWrite(BUZZ,HIGH);
      break;
    };
  }
