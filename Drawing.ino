 char * const pasar[]  = {"WAGE", "KLIWON", "LEGI", "PAHING", "PON"}; 
 char * const Hari[]  = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
//const char * const bulanMasehi[] PROGMEM = {"JANUARI", "FEBRUARI", "MARET", "APRIL", "MEI", "JUNI", "JULI", "AGUSTUS", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DESEMBER" };
// char* jadwal[] = {"SUBUH", "TERBIT", "DHUHA", "DZUHUR", "ASHAR", "MAGRIB", "ISYA'"};
 char* jadwalAzzan[] = {"SUBUH","DZUHUR", "ASHAR", "MAGRIB", "ISYA'"};
//const char * const namaBulanHijriah[] PROGMEM = {
//    "MUHARRAM", "SHAFAR", "RABIUL AWAL",
//    "RABIUL AKHIR", "JUMADIL AWAL", 
//    "JUMADIL AKHIR", "RAJAB",
//    "SYA'BAN", "RAMADHAN", "SYAWAL",
//    "DZULQA'DAH", "DZULHIJAH"
//};


//================= tampilan animasi ==================//

void showAnimasi() {
    //if (adzan) return;
   //  RtcDateTime now = Rtc.GetDateTime();
    static uint8_t y = 0;
    static uint8_t s = 0; // 0 = in, 1 = out
    static uint32_t lsRn = 0;
    static bool state = false;
    //static char lastBuffJam[6] = "";

    uint32_t Tmr = millis();
    //RtcDateTime now = Rtc.GetDateTime();
    // Ambil waktu dari RTC hanya jika diperlukan
    uint8_t daynow = now.DayOfWeek();
    uint8_t detik = now.Second();
    uint16_t tahun = now.Year();
    
    // Format jam berkedip setiap detik ganjil
    char buff_jam[8];
    sprintf(buff_jam, (detik & 1) ? "%02d:%02d" : "%02d %02d", now.Hour(), now.Minute());

    // Format tanggal, bulan, tahun
    char buff_tgl[3], buff_bln[3], buff_thn[3];
    sprintf(buff_tgl, "%02d", now.Day());
    sprintf(buff_bln, "%02d", now.Month());
    sprintf(buff_thn, "%02d", tahun - 2000);

   // Ambil nama hari dan pasar Jawa dari PROGMEM
    char buff_hari[8], buff_jawa[8];
    snprintf(buff_hari, sizeof(buff_hari), "%s", Hari[daynow]);
    snprintf(buff_jawa, sizeof(buff_jawa), "%s", pasar[jumlahhari() % 5]);

 
      if((Tmr-lsRn)>75) 
      { 
        if(s==0 and y<9){lsRn=Tmr;y++; }
        if(s==1 and y>0){lsRn=Tmr;y--; }
        
      }

    if ((Tmr - lsRn) > 3000 && y == 9) s = 1;
    if (y == 0 && s == 1) { s = 0; state = !state; }
   
    fType(1); Disp.drawText(3,17-y, (state == true)?buff_hari : buff_jawa);

    fType(0);

        Disp.drawText(3, 1, buff_jam);

        Disp.drawText(40, 0, buff_tgl);
 
        Disp.drawText(40, 9, buff_bln);
 
        Disp.drawText(52, 4, buff_thn);
 
    DoSwap = true; 
}


void sendTimeEvery5Min() {

  //RtcDateTime now = Rtc.GetDateTime();

  static int8_t lastSentMinute = -1;
  uint8_t hh = now.Hour();
  uint8_t mm = now.Minute();
  uint8_t ss = now.Second();
  uint8_t dd = now.DayOfWeek();

  // Kirim hanya tiap 5 menit
  if ((mm % 5) != 0) {
    lastSentMinute = -1;   // reset agar siap kirim di menit berikutnya
    return;
  }

  // Cegah kirim berulang di menit yang sama
  if (mm == lastSentMinute) return;

  lastSentMinute = mm;

  char buf[32];
  snprintf(buf, sizeof(buf),
           "TIME:%02d,%02d,%02d,%d",
           hh, mm, ss, dd);

  Serial.println(buf);
}

/*/================= animasi jadwal sholat new =================//
void updateAnimSholat() {
  if(adzan) return;
  
  RtcDateTime now = Rtc.GetDateTime();
  static int y = 0, y1 = 0;
  static uint8_t s = 0, s1 = 0;
  static bool run = false;

  static uint32_t lsRn_y1 = 0;
  static uint32_t lsRn_y = 0;
  static uint32_t tHold = 0;
  static uint8_t list = 0;

  uint32_t Tmr = millis();

  // Pilih waktu sholat sesuai list
  float stime;
  switch (list) {
    case 0: stime = JWS.floatImsak; break;
    case 1: stime = JWS.floatSubuh; break;
    case 2: stime = JWS.floatTerbit; break;
    case 3: stime = JWS.floatDzuhur; break;
    case 4: stime = JWS.floatAshar; break;
    case 5: stime = JWS.floatMaghrib; break;
    case 6: stime = JWS.floatIsya; break;
    default: stime = 0; break;
  }

  // Transisi vertikal y1 (jam muncul/hilang)
  if ((Tmr - lsRn_y1) > 55) {
    lsRn_y1 = Tmr;

    if (s1 == 0 && y1 < 17) {
      y1++;
    } else if (s1 == 1 && y1 > 0) {
      y1--;
    }
  }
  //Serial.println("y1:" + String(y1));
  // Saat y1 selesai muncul, mulai animasi jadwal
  if (y1 == 17 && s1 == 0) {
    run = true; 
    if(now.Second() % 2 ){
      Disp.drawRect(14, 3, 15, 5, 1); //posisi y = 6
      Disp.drawRect(14, 10, 15, 12, 1); //posisi y = 9
    }else{
      Disp.drawRect(14, 3, 15, 5, 0); //posisi y = 5
      Disp.drawRect(14, 10, 15, 12, 0); //posisi y = 8
    }
  }

  // Animasi gerakan teks (y)
  if (run && (Tmr - lsRn_y) > 55) {
    lsRn_y = Tmr;

    if (s == 0 && y < 9) {
      y++;
    } else if (s == 1 && y > 0) {
      y--;
    }
  }

  // Delay sebelum animasi keluar (reverse)
  if (y == 9 && s == 0 && tHold == 0) {
    tHold = millis();
  }
  if (tHold > 0 && (millis() - tHold > 1500)) {//4000
    s = 1;     // mulai keluar
    tHold = 0; // reset timer
  }

  // Setelah animasi selesai
  if (y == 0 && s == 1) {
    s = 0;
    list = (list + 1) % 7;
    if (list == 0) {
      run = false;
      s1 = 1; // trigger keluar vertikal
      Disp.drawRect(14, 4, 15, 5, 0); //posisi y = 5
      Disp.drawRect(14, 11, 15, 12, 0); //posisi y = 8
    }
  }

// Tampilkan jam digital
  fType(3);
  Disp.drawChar(0, y1 - 17, '0' + now.Hour() / 10); //12
  Disp.drawChar(7, y1 - 17, '0' + now.Hour() % 10);

  Disp.drawChar(17, y1 - 17, '0' + now.Minute() / 10);
  Disp.drawChar(24, y1 - 17, '0' + now.Minute() % 10);

  // Tampilkan teks jadwal sholat
  uint8_t shour = (uint8_t)stime;
  uint8_t sminute = (uint8_t)((stime - shour) * 60);

  char buf[6];
  buf[0] = '0' + shour / 10;
  buf[1] = '0' + shour % 10;
  buf[2] = ':';
  buf[3] = '0' + sminute / 10;
  buf[4] = '0' + sminute % 10;
  buf[5] = '\0';

  fType(1);
  dwCtr(31, y - 9, jadwal[list]);
  dwCtr(32, 18 - y, buf);
  DoSwap = true;
  if (y1 == 0 && s1 == 1) {
    s1 = 0;
    show = ANIM_SHOLAT; // ganti mode jika perlu
  }
}*/

//======================= end ==========================//

/*======================= animasi memasuki waktu sholat ====================================*/
void drawAzzan()
  {
    //RtcDateTime now = Rtc.GetDateTime();
    static const char *jadwal[] = {"SUBUH", "DZUHUR", "ASHAR", "MAGHRIB","ISYA'"};
    const char *sholat = jadwal[sholatNow]; 
    static uint8_t ct = 0;
    static uint32_t lsRn = 0;
    uint32_t Tmr = millis();
    const uint8_t limit = config.durasiadzan;
    char buff_jam[10];
    char buff_sec[2];
    char buff_text1[10];
    char buff_text2[10];
  
    sprintf(buff_jam,"%02d:%02d",now.Hour(),now.Minute());
    sprintf(buff_sec,"%02d  ",now.Second());
    sprintf(buff_text1,"%s","ADZAN");
    sprintf(buff_text2,"%s",jadwalAzzan[sholatNow]);  
   
   if (Tmr - lsRn > 500 && ct <= limit)
    {
        lsRn = Tmr;
        if (!(ct & 1))  // Lebih cepat dibandingkan ct % 2 == 0
        {
          fType(1); Disp.drawText(2,0,buff_text1);
          fType(1); Disp.drawText(1,8,buff_text2);
          Buzzer(1);
          DoSwap = true;
        }
        else 
          {
            Buzzer(0);
          }
         
        ct++;
    }
      fType(1); dwCtr(42,9,buff_sec);
      fType(1); dwCtr(34,0,buff_jam);
       //DoSwap = true;
   if ((Tmr - lsRn) > 1500 && (ct > limit))
    {
        adzan = 0;
        show = ANIM_SHOW;
        ct = 0;
        Buzzer(0);
    }
  }
//===================================== end =================================//

//=========================== setingan untuk tampilan text=================//
void fType(int x)
  {
    if(x==0) Disp.setFont(Font0);
    else if(x==1) Disp.setFont(Font1); 
    else if(x==2) Disp.setFont(Font2);
    else if(x==3) Disp.setFont(Font3);
    else if(x==4) Disp.setFont(Font4);
    else if(x==5) Disp.setFont(Font5);
  
  }

  void dwCtr(int x, int y, String Msg){
   uint16_t   tw = Disp.textWidth(Msg);
   uint16_t   c = int((DWidth-x-tw)/2);
   Disp.drawText(x+c,y,Msg);
}
//====================== end ==========================//
