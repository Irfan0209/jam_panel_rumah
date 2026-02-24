
//////hijiriyah voidku/////////////////////////////////////////////////
void islam() {
  if(adzan) return;
  RtcDateTime now = Rtc.GetDateTime();
  static uint32_t sv=0;
  uint32_t timer = millis();
  if(now.Hour() == 00 && now.Minute() == 00 && now.Second() == 00){
    stateBuzzWar = 1;
  }

  if(timer - sv > 5000){
    //RtcDateTime now = Rtc.GetDateTime();
    JWS.Update(config.zonawaktu, config.latitude, config.longitude, config.altitude, now.Year(), now.Month(), now.Day()); // Jalankan fungsi ini untuk update jadwal sholat
    JWS.setIkhtiSu = dataIhty[0];
    JWS.setIkhtiDzu = dataIhty[1];
    JWS.setIkhtiAs = dataIhty[2];
    JWS.setIkhtiMa = dataIhty[3];
    JWS.setIkhtiIs = dataIhty[4];
    Hijir.Update(now.Year(), now.Month(), now.Day(), config.Correction);
    sv = timer;
  }
}

// digunakan untuk menghitung hari pasaran
uint16_t jumlahhari() { 
  RtcDateTime now = Rtc.GetDateTime();
  uint8_t d = now.Day();
  uint8_t m = now.Month();
  uint16_t y = now.Year();

  static const uint16_t hb[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  uint16_t ht = (y - 1970) * 365 - 1;
  uint16_t hs = hb[m - 1] + d;

  if (y % 4 == 0 && m > 2) hs++; // Tambahkan 1 hari jika tahun kabisat dan lewat Februari

  uint16_t kab = (y - 1969) / 4;  // Hitung langsung jumlah tahun kabisat

  return (ht + hs + kab);
}
