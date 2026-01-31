

//SETUP DMD
#define DISPLAYS_WIDE 2
#define DISPLAYS_HIGH 1


#include <DMD3asis.h>



#include <EEPROM.h>
DMD3  Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // Jumlah Panel P10 yang digunakan (KOLOM,BARIS)


// Pengaturan hotspot WiFi dari ESP8266
char ssid[20]     = "JAM_PANEL";
char password[20] = "00000000";

//pengaturan wifi untuk upload program
const char* idwifi = "KELUARGA02";
const char* passwifi = "khusnul23";
const char* host = "JAM_PANEL_RUMAH";

//ESP8266WebServer server(80);

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
uint8_t dataIhty[]      = {0,0,0,0,0,0};

struct Config {
  uint8_t durasiadzan = 40;
  uint8_t altitude = 10;
  double latitude = -7.364057;
  double longitude = 112.646222;
  uint8_t zonawaktu = 7;
  int16_t Correction = -1; //Koreksi tanggal hijriyah, -1 untuk mengurangi, 0 tanpa koreksi, 1 untuk menambah
};

Config config;



// Variabel untuk waktu, tanggal, teks berjalan, tampilan ,dan kecerahan
char text1[101], text2[101],name[101];
uint16_t   brightness    = 50;
bool       adzan         = 0;
bool       stateBuzzer   = 1;
uint8_t    DWidth        = Disp.width();
uint8_t    DHeight       = Disp.height();
uint8_t    sholatNow     = -1;
bool       reset_x       = 0; 

/*======library tambahan=======*/
//bool       flagAnim = false;
uint8_t    speedDate      = 40; // Kecepatan default date
uint8_t    speedText1     = 40; // Kecepatan default text  
uint8_t    speedText2     = 40;
uint8_t    speedName      = 40;
float      dataFloat[10];
int        dataInteger[10];
bool       stateSendSholat = false; 
uint8_t    list,lastList;
bool       stateMode       = 0;
bool       stateBuzzWar    = 0;
uint8_t    counterName     = 0;
bool       DoSwap          = false;

char lastBuffTgl[3] = "";
char lastBuffBln[3] = "";
char lastBuffThn[3] = "";
/*============== end ================*/

enum Show{
  ANIM_SHOW,
  ANIM_CLOCK_BIG,
  ANIM_DATE,
  ANIM_NAME,
  ANIM_TEXT1,
  ANIM_TEXT2,
  ANIM_SHOLAT,
  ANIM_ADZAN,
  ANIM_IQOMAH,
  ANIM_BLINK,
  UPLOAD
};

Show show = ANIM_SHOW;


#define EEPROM_SIZE 512

//#define EEPROM_SIZE       512

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

// Fungsi untuk mengatur jam, tanggal, running text, dan kecerahan dari Serial
void handleSetTimeSerial() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim(); // hapus spasi dan newline

  if (input.length() == 0) return;

//  Serial.print("Input diterima: ");
//  Serial.println(input);

  // Panggil fungsi getData() untuk memproses input
  getData(input);
}

// =========================================
// DMD3 P10 utility Function================
// =========================================
void Disp_init() 
  { Disp.setDoubleBuffer(false);
    Timer1.initialize(1500);
    Timer1.attachInterrupt(scan);
    setBrightness(100);  
    Disp.clear();
    Disp.swapBuffers();
    }

void setBrightness(int bright)
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
  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
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
  loadFromEEPROM();
  delay(1000);
  
  Serial.println("PANEL_OK");
  Disp_init();
  delay(1000);
  for(int i = 0; i < 4; i++)
   {
      Buzzer(1);
      delay(80);
      Buzzer(0);
      delay(80);
   }

}

void loop() {
  
    //handleSetTimeSerial()
    check();
    islam();
    DoSwap  = false ;
    //Disp.clear();
 

 switch(show){
  case ANIM_SHOW :
    showAnimasi();
  break;
 
  case ANIM_ADZAN :
    drawAzzan();
  break;
 };
  
  buzzerWarning(stateBuzzWar);
  if(DoSwap){Disp.swapBuffers();} // Swap Buffer if Change
}

void getData(String input) {

  int eq = input.indexOf('=');
  if (eq != -1) {
    String key = input.substring(0, eq);
    String value = input.substring(eq + 1);
    
    if (key == "Tm") {
  String setJam = value;

  // Format: HH:MM:SS-Tanggal-Bulan-Tahun
  uint8_t colon1 = value.indexOf(':');
  uint8_t colon2 = value.indexOf(':', colon1 + 1);
  uint8_t dash1 = value.indexOf('-');
  uint8_t dash2 = value.indexOf('-', dash1 + 1);
  uint8_t dash3 = value.indexOf('-', dash2 + 1);

  if (colon1 != -1 && colon2 != -1 && dash1 != -1 && dash2 != -1 && dash3 != -1) {
    uint8_t jam    = value.substring(0, colon1).toInt();
    uint8_t menit  = value.substring(colon1 + 1, colon2).toInt();
    uint8_t detik  = value.substring(colon2 + 1, dash1).toInt();
    uint8_t tanggal= value.substring(dash1 + 1, dash2).toInt();
    uint8_t bulan  = value.substring(dash2 + 1, dash3).toInt();
    uint16_t tahun = value.substring(dash3 + 1).toInt();

    Rtc.SetDateTime(RtcDateTime(tahun, bulan, tanggal,jam, menit, detik));
    //JWS.Update(config.zonawaktu, config.latitude, config.longitude, config.altitude, year(),month(), day());
     stateSendSholat = 1;
  }
}

    else if (key == "text") {
      int separatorIndex = value.indexOf('-');
      if (separatorIndex != -1) {
        int indexText = value.substring(0, separatorIndex).toInt();
        String pesan = value.substring(separatorIndex + 1);

        if (pesan.length() > 100) pesan = pesan.substring(0, 100);

        if (indexText == 1) {
          pesan.toCharArray(text1, 101);
          saveStringToEEPROM(ADDR_TEXT1, String(text1), 100);
        } else if (indexText == 2) {
          pesan.toCharArray(text2, 101);
          saveStringToEEPROM(ADDR_TEXT2, String(text2), 100);
        }
      }
      Buzzer(1);
      delay(500);
      //ESP.restart();
    }

    else if (key == "name") {
       if (value.length() > 100) {value = value.substring(0, 100);} // Batasi max 100 karakter
       value.toCharArray(name, 101); // +1 untuk null-terminator
       Serial.println(name);
       saveStringToEEPROM(ADDR_NAME, String(name), 100);

      Buzzer(1);
      delay(500);
      //ESP.restart();
    }


    else if (key == "Br") {
      brightness = map(value.toInt(), 0, 100, 10, 255);
      setBrightness(brightness);
      saveIntToEEPROM(ADDR_BRIGHTNESS, brightness);
    }

    else if (key == "Sptx1") {
      speedText1 = map(value.toInt(), 0, 100, 10, 80);
      saveIntToEEPROM(ADDR_SPEEDTX1, speedText1);
    }

    else if (key == "Sptx2") {
      speedText2 = map(value.toInt(), 0, 100, 10, 80);
      saveIntToEEPROM(ADDR_SPEEDTX2, speedText2);
    }

    else if (key == "Spdt") {
      speedDate = map(value.toInt(), 0, 100, 10, 80);
      saveIntToEEPROM(ADDR_SPEEDDT, speedDate);
    }

    else if (key == "Spnm") {
      speedName = map(value.toInt(), 0, 100, 10, 80);
      saveIntToEEPROM(ADDR_SPEEDNAME, speedName);
    }

    else if (key == "Lt") {
      config.latitude = roundf(value.toFloat() * 1000000.0) / 1000000.0;
      saveFloatToEEPROM(ADDR_LATITUDE, config.latitude);
    }

    else if (key == "Lo") {
      config.longitude = roundf(value.toFloat() * 1000000.0) / 1000000.0;
      saveFloatToEEPROM(ADDR_LONGITUDE, config.longitude);
    }

    else if (key == "Tz") {
      config.zonawaktu = value.toInt();
      saveIntToEEPROM(ADDR_TZ, config.zonawaktu);
    }

    else if (key == "Al") {
      config.altitude = value.toInt();
      saveIntToEEPROM(ADDR_ALTITUDE, config.altitude);
    }

    else if (key == "Iq") {
      int separatorIndex = value.indexOf('-');
      int indexSholat = value.substring(0, separatorIndex).toInt();
      int indexKoreksi = value.substring(separatorIndex + 1).toInt();  
      iqomah[indexSholat] = indexKoreksi;
      EEPROM.write(ADDR_IQOMAH + indexSholat, indexKoreksi);
    }

    else if (key == "Dy") {
      int separatorIndex = value.indexOf('-');
      int indexSholat = value.substring(0, separatorIndex).toInt();
      int indexKoreksi = value.substring(separatorIndex + 1).toInt();  
      displayBlink[indexSholat] = indexKoreksi;
      EEPROM.write(ADDR_BLINK + indexSholat, indexKoreksi);
    }

    else if (key == "Kr") {
      int separatorIndex = value.indexOf('-');
      int indexSholat = value.substring(0, separatorIndex).toInt();
      int indexKoreksi = value.substring(separatorIndex + 1).toInt();  
      dataIhty[indexSholat] = indexKoreksi;
      EEPROM.write(ADDR_IHTY + indexSholat, indexKoreksi);
    }

    else if (key == "Da") {
      config.durasiadzan = value.toInt();
      EEPROM.write(ADDR_DURASIADZAN, config.durasiadzan & 0xFF);
      EEPROM.write(ADDR_DURASIADZAN + 1, (config.durasiadzan >> 8) & 0xFF);
    }

    else if (key == "CoHi") {
      config.Correction = value.toInt();
      EEPROM.write(ADDR_CORRECTION, config.Correction & 0xFF);
      EEPROM.write(ADDR_CORRECTION + 1, (config.Correction >> 8) & 0xFF);
}


    else if (key == "Bzr") {
      stateBuzzer = value.toInt();
      EEPROM.write(ADDR_BUZZER, stateBuzzer);
    }

    /*else if (key == "mode") {
      stateMode = value.toInt();
      EEPROM.write(ADDR_MODE, stateMode);
      delay(1000);
      //ESP.restart();
    }*/

    else if (key == "status") {
      int state = value.toInt();
      if(state) Serial.println("PANEL_OK");
    }

    else if (key == "jadwal") {
      stateSendSholat = value.toInt();
    }

    else if (key == "restart") {
      int state = value.toInt();
      if(state) {
        Buzzer(1); 
        Serial.println("RESTART_OK"); 
        stateMode = 0;
        EEPROM.write(ADDR_MODE, stateMode); 
        delay(1000);
        //ESP.restart();
      }
    }

    else if (key == "passwordPanel") {
      if (value.length() == 8) {
        value.toCharArray(password, value.length() + 1);
        saveStringToEEPROM(ADDR_PASSWORD, value, 8);
        //server.send(200, "text/plain", "Password WiFi diupdate");
        Buzzer(1);
        delay(500);
        //ESP.restart();
      }
    }

    //EEPROM.commit(); // Penting! simpan perubahan
  }
  
}


void loadFromEEPROM() {
  //Serial.println("=== Membaca Data dari EEPROM ===");
 
  for (int i = 0; i < 100; i++) {
    text1[i] = EEPROM.read(ADDR_TEXT1 + i);
    if (text1[i] == 0) break;
  }
//  Serial.print("Text1: ");
//  Serial.println(text1);
  
  for (int i = 0; i < 100; i++) {
    text2[i] = EEPROM.read(ADDR_TEXT2 + i);
    if (text2[i] == 0) break;
  }
//  Serial.print("Text2: ");
//  Serial.println(text2);

  for (int i = 0; i < 100; i++) {
    name[i] = EEPROM.read(ADDR_NAME + i);
    if (name[i] == 0) break;
  }
//  Serial.print("nama: ");
//  Serial.println(name);

  brightness = EEPROM.read(ADDR_BRIGHTNESS);
//  Serial.print("Brightness: ");
//  Serial.println(brightness);

  speedText1 = EEPROM.read(ADDR_SPEEDTX1);
//  Serial.print("Speed Text1: ");
//  Serial.println(speedText1);

  speedText2 = EEPROM.read(ADDR_SPEEDTX2);
//  Serial.print("Speed Text2: ");
//  Serial.println(speedText2);

  speedDate = EEPROM.read(ADDR_SPEEDDT);
//  Serial.print("Speed Date: ");
//  Serial.println(speedDate);

  speedName = EEPROM.read(ADDR_SPEEDNAME);
//  Serial.print("Speed Name: ");
//  Serial.println(speedName);

  // Latitude
  float latVal;
  byte *ptrLat = (byte*)(void*)&latVal;
  for (int i = 0; i < sizeof(float); i++) {
    ptrLat[i] = EEPROM.read(ADDR_LATITUDE + i);
  }
  config.latitude = latVal;
//  Serial.print("Latitude: ");
//  Serial.println(config.latitude, 6);

  // Longitude
  float lonVal;
  byte *ptrLon = (byte*)(void*)&lonVal;
  for (int i = 0; i < sizeof(float); i++) {
    ptrLon[i] = EEPROM.read(ADDR_LONGITUDE + i);
  }
  config.longitude = lonVal;
//  Serial.print("Longitude: ");
//  Serial.println(config.longitude, 6);

  config.zonawaktu = EEPROM.read(ADDR_TZ) | (EEPROM.read(ADDR_TZ + 1) << 8);
//  Serial.print("Zona Waktu: ");
//  Serial.println(config.zonawaktu);

  config.altitude = EEPROM.read(ADDR_ALTITUDE) | (EEPROM.read(ADDR_ALTITUDE + 1) << 8);
//  Serial.print("Altitude: ");
//  Serial.println(config.altitude);

  for (int i = 0; i < 6; i++) {
    iqomah[i] = EEPROM.read(ADDR_IQOMAH + i);
//    Serial.print("Iqomah[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.println(iqomah[i]);
  }

  for (int i = 0; i < 6; i++) {
    displayBlink[i] = EEPROM.read(ADDR_BLINK + i);
//    Serial.print("Blink[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.println(displayBlink[i]);
  }

  for (int i = 0; i < 6; i++) {
    dataIhty[i] = EEPROM.read(ADDR_IHTY + i);
//    Serial.print("Ihtiyath[");
//    Serial.print(i);
//    Serial.print("]: ");
//    Serial.println(dataIhty[i]);
  }

  stateBuzzer = EEPROM.read(ADDR_BUZZER);
//  Serial.print("Buzzer: ");
//  Serial.println(stateBuzzer);

  stateMode = EEPROM.read(ADDR_MODE);
//  Serial.print("mode: ");
//  Serial.println(stateMode);

  for (int i = 0; i < 8; i++) {
    password[i] = EEPROM.read(ADDR_PASSWORD + i);
  }
  password[8] = '\0';
//  Serial.print("Password: ");
//  Serial.println(password);

  // Tambahan yang baru:
  config.durasiadzan = EEPROM.read(ADDR_DURASIADZAN) | (EEPROM.read(ADDR_DURASIADZAN + 1) << 8);
//  Serial.print("Durasi Adzan: ");
//  Serial.println(config.durasiadzan);

  config.Correction = EEPROM.read(ADDR_CORRECTION) | (EEPROM.read(ADDR_CORRECTION + 1) << 8);
//  Serial.print("Correction: ");
//  Serial.println(config.Correction);

//  Serial.println("=== Selesai Membaca EEPROM ===\n");
//  Serial.println("OK");
}

 //----------------------------------------------------------------------
// I2C_ClearBus menghindari gagal baca RTC (nilai 00 atau 165)

int I2C_ClearBus() {
  
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

void Buzzer(uint8_t state)
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
