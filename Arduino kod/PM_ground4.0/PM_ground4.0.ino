#include <CanSatKit.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <cmath>

using namespace CanSatKit;

// varables for lcd display 
float altitude;
int last_rssi;
int nr_of_digits_alti;
int nr_of_digits_counter;
int nr_of_digits_rssi;

// debiging variables 
// only the is_lm_active has to hard coded, the rest are updating in the program process.

bool is_sd_active = false; 
bool is_radio_active = false;
bool is_bmp_active = false;
bool is_lm_active = true;

const int lm35_pin = A0; 

// measurments variables 
int counter;
float t_ground_bmp;
float t_ground_lm;
float t_difference;
float p_ground;
float t_cansat;
float p_cansat;
float send_time;

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

File log_main_file;
File log_additional_file;

const int rs = 13, en = 9, d4 = 3, d5 = 4, d6 = 5, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

using namespace CanSatKit;

// packet 
struct Packet {
  int packet_id;
  double temperature_bmp;
  float temperature_lm;
  double pressure;
  unsigned long int sending_time;
  float mpu_data [15];
  float current [10];
} packet;

// set radio receiver parameters - see comments below
// remember to set the same radio parameters in
// transmitter and receiver boards!
Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.7,                  // frequency in MHz
            Bandwidth_125000_Hz,    // bandwidth - check with CanSat regulations to set allowed value
            SpreadingFactor_9,      // see provided presentations to determine which setting is the best
            CodingRate_4_8);        // see provided presentations to determine which setting is the best

void setup() {
  SerialUSB.begin(9600);

  lcd.begin(16, 2);

  // start radio module  
  radio_start ();

  // initialize sd connection 
  sd_start();

  // use begin() function to check if sensor is conneted and is able to communicate
  // with the CanSat board
  bmp_start();

  if (is_lm_active) analogReadResolution(12);
}

void loop() {
  if (is_bmp_active && is_radio_active) {
    double temperature_ground, pressure_ground;
    bmp.measureTemperatureAndPressure(temperature_ground, pressure_ground); 
    
    radio.receive((char *)(&packet));
    
    // get and print signal level (rssi)
    counter = packet.packet_id;
    SerialUSB.print("Package counter: ");
    SerialUSB.println(counter);
    SerialUSB.print("RSSI = ");
    last_rssi = radio.get_rssi_last();
    SerialUSB.println(last_rssi);
  
    // print received message
    if (is_lm_active) {
      t_ground_lm = lm35_raw_to_temperature(analogRead(lm35_pin));
    }
    t_ground_bmp = temperature_ground;
    p_ground = pressure_ground;
    t_cansat = packet.temperature_bmp;
    p_cansat = packet.pressure;
    send_time = packet.sending_time;
    
    altitude = 44308.23 * (1.0 - pow(p_cansat / p_ground, 0.190284));
  
    SerialUSB.print("Temp bmp ground: ");
    SerialUSB.println(t_ground_bmp);
    if (is_lm_active) {
      SerialUSB.print("Temp lm35 ground: ");
      SerialUSB.println(t_ground_lm);
      SerialUSB.print("Difference in temperatures (lm and BMP280): ");
      if (t_ground_lm >= t_ground_bmp){
        t_difference = t_ground_lm - t_ground_bmp;
      } else {
        t_difference = t_ground_bmp - t_ground_lm;
      }
      SerialUSB.println(t_difference);
    }
    SerialUSB.print("Pressure ground: ");
    SerialUSB.println(p_ground);
    SerialUSB.print("Temp cansat: ");
    SerialUSB.println(t_cansat);
    SerialUSB.print("Pressure cansat: ");
    SerialUSB.println(p_cansat);
    SerialUSB.print("Altitude: ");
    SerialUSB.println(altitude);
    SerialUSB.print("Send time: ");
    SerialUSB.println(send_time);  

    if (is_sd_active) {
      if (is_lm_active) {
        additional_log_to_sd (last_rssi, counter, p_cansat, p_ground, t_cansat, t_ground_bmp, t_ground_lm, t_difference, altitude, send_time);
      } else {
        main_log_to_sd (last_rssi, counter, p_cansat, p_ground, t_cansat, t_ground_bmp, altitude, send_time);
      }
    } else {
      sd_start ();
      delay (1);
    }
    print_to_lcd(last_rssi, counter, altitude);
  
    delay(1);
  } else {
    if (!is_radio_active) {
      radio_start ();
      delay (1);
    }
    if (!is_bmp_active) {
      bmp_start ();
      delay(1);
    }
  }
}

bool sd_start () {
  if (!SD.begin()) {
    SerialUSB.println("SD card init failed");
    is_sd_active = false;
    return false;
  } else {
    SerialUSB.println("SD card init success!");
    is_sd_active = true;
    return true;
  }
}

bool bmp_start () {
  if (!bmp.begin()) {
    SerialUSB.println("BMP 280 init failed!");
    is_bmp_active = false;
    return false;
  } else {
    bmp.setOversampling(16);
    SerialUSB.println("BMP 280 init success!");
    is_bmp_active = true;
    return true; 
  }
}

bool radio_start () {
  if (!radio.begin()) {
    SerialUSB.println("Radio module init failed!");
    is_radio_active = false;
    return false;
  } else {
    SerialUSB.println("Radio module init success!");
    is_radio_active = true; 
    return true; 
  }
}

bool main_log_to_sd (int last_rssi, int package_counter, float pressure_cansat, float temperature_cansat, float pressure_ground, float temperature_ground, float altitude, float send_time) {
  log_main_file = SD.open("log.txt", FILE_WRITE);
  if (log_main_file) {
    log_main_file.print("Package send time = ");
    log_main_file.println(send_time);
    log_main_file.print("Package #");
    log_main_file.println(package_counter);
    log_main_file.print("RSSI = ");
    log_main_file.println(last_rssi);    
    log_main_file.print("Temperature cansat = ");
    log_main_file.print(temperature_cansat);
    log_main_file.println(" deg C");
    log_main_file.print("Presure cansat = ");
    log_main_file.print(pressure_cansat);
    log_main_file.println(" hPa");    
    log_main_file.print("Temperature ground = ");
    log_main_file.print(temperature_ground);
    log_main_file.println(" deg C");
    log_main_file.print("Presure ground = ");
    log_main_file.print(pressure_ground);
    log_main_file.println(" hPa");
    log_main_file.print("Altitude = ");
    log_main_file.print(altitude);
    log_main_file.println(" m");
    log_main_file.close();
    SerialUSB.println("Data saved");
    return true;
  } else {
    SerialUSB.println("Opening file failed!");  
    return false;
  } 
}

bool additional_log_to_sd (int last_rssi, int package_counter, float pressure_cansat, float temperature_cansat, float pressure_ground, float temperature_ground_lm, float temperature_ground_bmp, float temperature_ground_difference, float altitude, float send_time) {
  log_additional_file = SD.open("log_extended.txt", FILE_WRITE);
  if (log_main_file) {
    log_main_file.print("Package send time = ");
    log_main_file.println(send_time);
    log_main_file.print("Package #");
    log_main_file.println(package_counter);
    log_main_file.print("RSSI = ");
    log_main_file.println(last_rssi);    
    log_main_file.print("Temperature cansat = ");
    log_main_file.print(temperature_cansat);
    log_main_file.println(" deg C");
    log_main_file.print("Presure cansat = ");
    log_main_file.print(pressure_cansat);
    log_main_file.println(" hPa");    
    log_main_file.print("Temperature ground (from bmp)= ");
    log_main_file.print(temperature_ground_bmp);
    log_main_file.println(" deg C");
    log_main_file.print("Temperature ground (from lm)");
    log_main_file.print(temperature_ground_lm);
    log_main_file.println(" deg C");
    log_main_file.print("Difference in ground temperature measurments = ");
    log_main_file.print(temperature_ground_difference);
    log_main_file.println(" deg C");
    log_main_file.print("Presure ground = ");
    log_main_file.print(pressure_ground);
    log_main_file.println(" hPa");
    log_main_file.print("Altitude = ");
    log_main_file.print(altitude);
    log_main_file.println(" m");
    log_main_file.close();
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

bool print_to_lcd (int last_rssi, int package_counter, double alti) {
  SerialUSB.println("LCD");
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
  lcd.print(" ");
  lcd.print("RSSI: ");
  lcd.print(last_rssi);
  lcd.print(empty_string_lenght_n(16-(8+nr_of_digits_counter+nr_of_digits_rssi)));
  lcd.setCursor(0,1);
  lcd.print("A. ");
  //lcd.setCursor(3, 1);
  lcd.print(alti, 2);
  lcd.print(" m");
  lcd.print(empty_string_lenght_n(16-(8+nr_of_digits_alti)));
  return true;
}

// convert analog reading to temperature in celsius 
float lm35_raw_to_temperature(int raw) {
  float voltage = raw * 3.3 / (std::pow(2, 12));
  float temperature = 100.0 * voltage;
  return temperature;
}
