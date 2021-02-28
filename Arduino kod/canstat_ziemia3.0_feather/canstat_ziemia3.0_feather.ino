// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <Adafruit_BMP280.h>


// for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// for bmp
Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// define lcd display 
const int rs = 13, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// define log file
File log_file; 

double pressure_ground;
float altitude;
int last_rssi;
int nr_of_digits_alti;
int nr_of_digits_counter;
int nr_of_digits_rssi;

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial) {
    delay(1);
  }
  
  // radio config 
  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  /*
  if (!rf95.setSpreadingFactor(9)) {
    Serial.println("set spreading factor failed");
    while(1);
  }
  
  if (!rf95.setCodingRate4(8)) {
    Serial.println("Setiing coding rate failed");
    while(1);
  }
  */
  Serial.println("LoRa radio init OK!");
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  // lcd config 
  lcd.begin(16, 2);

  // sd card init
  if (SD.begin(11)) {
    Serial.println("Sd card init failed");
    Serial.println("Restart device to continue");
  } else {
    Serial.println("Sd card init ok!");
  }

  // bmp init and test 
  Serial.println(F("BMP280 Sensor event test"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{
  
  delay(1); // Wait 1 second between transmits, could also 'sleep' here!

  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  float data[3];
  
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);   

      memcpy(data, buf, sizeof data);
      
      last_rssi = rf95.lastRssi();

      pressure_ground = pressure_event.pressure;

      altitude = 44330 * (1.0 - pow(data[1] / pressure_ground, 0.1903));

      log_to_sd (last_rssi, data[0], data[1], pressure_ground, data[2], altitude);

      print_to_lcd(last_rssi, data[0], altitude);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No signal after 1 s.");
  }

}

bool log_to_sd (int last_rssi, int package_counter, float presure_cansat, float presure_ground, float temp, float alti) {
  log_file = SD.open("log.txt", FILE_WRITE);
  if (log_file) {
    log_file.print("Package #");
    log_file.println(package_counter);
    log_file.print("RSSI = ");
    log_file.println(last_rssi);
    log_file.print("Presure cansat = ");
    log_file.print(presure_cansat);
    log_file.println(" hPa");
    log_file.print("Presure groud = ");
    log_file.print(presure_ground);
    log_file.println(" hPa");
    log_file.print("Temperature = ");
    log_file.print(temp);
    log_file.println(" deg C");
    log_file.print("Altitude = ");
    log_file.print(alti);
    log_file.println(" m");
    log_file.close();
    SerialUSB.println("Data saved");
    return true;
  } else {
    SerialUSB.println("Opening file failed!");  
    return false;
  } 
}


// lcd part 
String empty_string_lenght_n (int n){
  String mystr = "";
  for (int i = 0; i < n; i++) {
    mystr += " ";
  }
  return mystr;
}

bool print_to_lcd (int last_rssi, int package_counter, float alti) {
  if (alti >= 0) {
    nr_of_digits_alti = log10(alti) + 1;
  } else {
    nr_of_digits_alti = log(alti*(-1)) + 1;
  }
  
  nr_of_digits_counter = log10(package_counter) + 1;
  nr_of_digits_rssi = log10(last_rssi*(-1)) + 2; 
  lcd.setCursor(0, 0);
  lcd.print("#");
  lcd.print(package_counter);
  //lcd.setCursor(1+nr_of_digits_counter,0);
  lcd.print(" ");
  //lcd.setCursor(2+nr_of_digits_counter,0);
  lcd.print("RSSI: ");
  //lcd.setCursor(8+nr_of_digits_counter,0);
  lcd.print(last_rssi);
  //lcd.setCursor(8+nr_of_digits_counter+nr_of_digits_rssi,0);
  lcd.print(empty_string_lenght_n(16-(8+nr_of_digits_counter+nr_of_digits_rssi)));
  lcd.setCursor(0,1);
  lcd.print("P. ");
  //lcd.setCursor(3, 1);
  lcd.print(alti, 2);
  //lcd.setCursor(7+nr_of_digits_alti, 1);
  lcd.print(" m");
  lcd.print(empty_string_lenght_n(16-(8+nr_of_digits_alti)));
  return true;
}
