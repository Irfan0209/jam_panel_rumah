 char * const pasar[]  = {"WAGE", "KLIWON", "LEGI", "PAHING", "PON"}; 
 char * const Hari[]  = {"MINGGU","SENIN","SELASA","RABU","KAMIS","JUM'AT","SABTU"};
const char * const bulanMasehi[] PROGMEM = {"JANUARI", "FEBRUARI", "MARET", "APRIL", "MEI", "JUNI", "JULI", "AGUSTUS", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DESEMBER" };
 char* jadwal[] = {"SUBUH", "TERBIT", "DHUHA", "DZUHUR", "ASHAR", "MAGRIB", "ISYA'"};
 char* jadwalAzzan[] = {"SUBUH","DZUHUR", "ASHAR", "MAGRIB", "ISYA'"};
const char * const namaBulanHijriah[] PROGMEM = {
    "MUHARRAM", "SHAFAR", "RABIUL AWAL",
    "RABIUL AKHIR", "JUMADIL AWAL", 
    "JUMADIL AKHIR", "RAJAB",
    "SYA'BAN", "RAMADHAN", "SYAWAL",
    "DZULQA'DAH", "DZULHIJAH"
};


//================= tampilan animasi ==================//

void showAnimasi() {
    if (adzan) return;
    
    static uint8_t y = 0;
    static uint8_t s = 0; // 0 = in, 1 = out
    static uint32_t lsRn = 0;
    static bool state = false;
    static char lastBuffJam[6] = "";

    uint32_t Tmr = millis();
    RtcDateTime now = Rtc.GetDateTime();
    // Ambil waktu dari RTC hanya jika diperlukan
    uint8_t daynow = now.DayOfWeek();
    uint8_t detik = now.Second();
    uint16_t tahun = now.Year();
    
    // Format jam berkedip setiap detik ganjil
    char buff_jam[6];
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
    if (strcmp(buff_jam, lastBuffJam) != 0) {
        strcpy(lastBuffJam, buff_jam);
        Disp.drawText(3, 1, buff_jam);
    }

    if (strcmp(buff_tgl, lastBuffTgl) != 0) {
        strcpy(lastBuffTgl, buff_tgl);
        Disp.drawText(40, 0, buff_tgl);
        Serial.println(F("tgl run"));
    }

    if (strcmp(buff_bln, lastBuffBln) != 0) {
        strcpy(lastBuffBln, buff_bln);
        Disp.drawText(40, 9, buff_bln);
        Serial.println(F("bln run"));
    }

    if (strcmp(buff_thn, lastBuffThn) != 0) {
        strcpy(lastBuffThn, buff_thn);
        Disp.drawText(52, 4, buff_thn);
        Serial.println(F("thn run"));
    }
    DoSwap = true; 
}

//======================= end ==========================//

/*======================= animasi memasuki waktu sholat ====================================*/
void drawAzzan()
{
    //static const char *jadwal[] = {"SUBUH", "DZUHUR", "ASHAR", "MAGRIB","ISYA'"};
    const char *sholat = jadwalAzzan[sholatNow]; 
    static uint8_t ct = 0;
    static uint32_t lsRn = 0;
    uint32_t Tmr = millis();
    const uint8_t limit = 80;//config.durasiadzan;

    if (Tmr - lsRn > 500 && ct <= limit)
    {
        lsRn = Tmr;
        if (!(ct & 1))  // Lebih cepat dibandingkan ct % 2 == 0
        {
          fType(0);
            dwCtr(1, 0, "ADZAN");
            fType(0);
            dwCtr(1, 9, sholat);
            Buzzer(1);
        }
        else
        {
            Buzzer(0);
            Disp.clear();
        }
        DoSwap = true; 
        ct++;
    }
    
    if ((Tmr - lsRn) > 1500 && (ct > limit))
    {
        show = ANIM_SHOW;
        Disp.clear();
        ct = 0;
        sholatNow = -1;
        adzan = false;
        Buzzer(0);
        strcpy(lastBuffTgl, "");
        strcpy(lastBuffBln, "");
        strcpy(lastBuffThn, "");
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
