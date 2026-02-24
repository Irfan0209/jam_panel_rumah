/*/================= cek waktu sholat ===================//
void check() {
    if(adzan) return;
    //RtcDateTime now = Rtc.GetDateTime();
    uint8_t jam = now.Hour();
    uint8_t menit = now.Minute();
    uint8_t detik = now.Second();
    uint8_t daynow = now.DayOfWeek();
    uint8_t hours, minutes;
    static uint8_t counter = 0;//cekList = 0;
    static uint32_t lsTmr,saveTmr;
    static bool adzanFlag[5] = {false, false, false, false, false};
    float sholatT[]={JWS.floatSubuh,JWS.floatDzuhur,JWS.floatAshar,JWS.floatMaghrib,JWS.floatIsya};
    uint32_t tmr = millis();
    
    
    if (tmr - lsTmr > 100 && !stateSendSholat) {
        lsTmr = tmr;
        
        float stime = sholatT[counter];
        uint8_t hours = floor(stime);
        uint8_t minutes = floor((stime - (float)hours) * 60);
        //uint8_t ssecond = floor((stime - (float)hours - (float)minutes / 60) * 3600);
    
        if (!adzanFlag[counter]) {
            if (jam == hours && menit == minutes && detik == 0) {
                
              if(daynow == 5 && counter == 1){
                return ;
              }else{
                Disp.clear();
                sholatNow = counter;
                adzan = 1;
                reset_x = 1;
                show = ANIM_ADZAN;
                adzanFlag[counter] = true;
              }       
            }
        }
        if (jam != hours || menit != minutes) {
            adzanFlag[counter] = false;
        }
        counter = (counter + 1) % 5;
    }

    if (tmr - saveTmr > 500 && stateSendSholat == true) {
     saveTmr = tmr;
     String jwsData = "JWS:";
     for (uint8_t i = 0; i < 5; i++) {
       uint8_t h = floor(sholatT[i]);
       uint8_t m = floor((sholatT[i] - (float)h) * 60);
       jwsData += (h < 10 ? "0" : "") + String(h) + "," + (m < 10 ? "0" : "") + String(m);
       if (i < 4) jwsData += "|";
     }
     Serial.println(jwsData); // Dikirim ke Serial Monitor
     stateSendSholat = false; 
    // cekList = 0;
    }
}
*/



static float lastStime[5] = { -1, -1, -1, -1, -1 };
static uint8_t sholatHour[5];
static uint8_t sholatMinute[5];
static bool adzanFlag[5] = { 0 };


//================= cek waktu sholat (REFRAKTOR) ===================//
void check() {

  static uint8_t counter = 0;
  static uint32_t scanTmr = 0, sendTmr = 0;

  uint32_t nowMs = millis();

  // ================= CACHE UPDATE (float -> int) =================
  
  // Pilih waktu sholat sesuai list
  float stime;
  switch (counter) {
    case 0: stime = JWS.floatSubuh; break;
    case 1: stime = JWS.floatDzuhur; break;
    case 2: stime = JWS.floatAshar; break;
    case 3: stime = JWS.floatMaghrib; break;
    case 4: stime = JWS.floatIsya; break;
  }
  
  if (stime != lastStime[counter]) {

    lastStime[counter] = stime;

    uint8_t h = (uint8_t)stime;
    uint8_t m = (uint8_t)((stime - h) * 60);

    sholatHour[counter]   = h;
    sholatMinute[counter] = m;

    adzanFlag[counter] = false;   // reset jika jadwal berubah
  }

  // ================= CEK WAKTU ADZAN =================
  if (!stateSendSholat && nowMs - scanTmr >= 100) {
    scanTmr = nowMs;

    RtcDateTime now = Rtc.GetDateTime();
    uint8_t jam   = now.Hour();
    uint8_t menit = now.Minute();
    uint8_t detik = now.Second();
    uint8_t day   = now.DayOfWeek();

    uint8_t h = sholatHour[counter];
    uint8_t m = sholatMinute[counter];

    if (!adzanFlag[counter]) {
      if (jam == h && menit == m && detik == 0) {
          
        // Jumat - Dzuhur tidak adzan
        if (!(day == 5 && counter == 1)) {
          sholatNow = counter;
          adzan = 1;
          reset_x = 1;
          show = ANIM_ADZAN;
          adzanFlag[counter] = true;
        }
      }
    }

    // reset flag jika waktu sudah lewat
    if (jam != h || menit != m) {
      adzanFlag[counter] = false;
    }

    counter++;
    if (counter >= 5) counter = 0;
  }

  // ================= KIRIM DATA JWS =================
  if (stateSendSholat && nowMs - sendTmr >= 500) {
    sendTmr = nowMs;

    char buf[64];
    char *p = buf;
    p += sprintf(p, "JWS:");

    for (uint8_t i = 0; i < 5; i++) {
      p += sprintf(p, "%02d,%02d", sholatHour[i], sholatMinute[i]);
      if (i < 4) *p++ = '|';
    }

    *p = '\0';
    Serial.println(buf);

    stateSendSholat = false;
  }
}
