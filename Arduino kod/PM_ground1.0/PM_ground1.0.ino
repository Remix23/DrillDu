#include <CanSatKit.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <cmath>

using namespace CanSatKit;

float altitude;
int last_rssi;
int nr_of_digits_alti;
int nr_of_digits_counter;
int nr_of_digits_rssi;

const int lm35_pin = A0;

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

File log_file;

const int rs = 13, en = 9, d4 = 3, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

using namespace CanSatKit;

// set radio receiver parameters - see comments below
// remember to set the same radio parameters in
// transmitter and receiver boards!
Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,                  // frequency in MHz
            Bandwidth_125000_Hz,    // bandwidth - check with CanSat regulations to set allowed value
            SpreadingFactor_9,      // see provided presentations to determine which setting is the best
            CodingRate_4_8);        // see provided presentations to determine which setting is the best

void setup() {
  SerialUSB.begin(115200);

  lcd.begin(16, 2);

  // start radio module  
  radio.begin();
  
  if (!SD.begin(11)) {
    SerialUSB.println("SD card init failed!");
    SerialUSB.println("Restart device to continue");
  } else {
    SerialUSB.println("SD card init ok!");
  }

  // use begin() function to check if sensor is conneted and is able to communicate
  // with the CanSat board
  if(!bmp.begin()) {
    SerialUSB.println("BMP init failed!");
  } else {
    SerialUSB.println("BMP init success!");
  }
  bmp.setOversampling(16);

  analogReadResolution(12);
}

void loop() {

  float received_from_cansat [3];

  double T, pressure_ground;
  
  radio.receive((char *)(received_from_cansat));

  bmp.measureTemperatureAndPressure(T, pressure_ground); 
  
  // get and print signal level (rssi)
  SerialUSB.print("Package counter: ");
  SerialUSB.println(received_from_cansat[0]);
  SerialUSB.print("RSSI = ");
  last_rssi = radio.get_rssi_last();
  SerialUSB.println(last_rssi);

  // print received message
  int counter = received_from_cansat[0];
  float t_ground = lm35_raw_to_temperature(analogRead(lm35_pin));
  float p_ground = pressure_ground;
  float t_cansat = received_from_cansat[2];
  float p_cansat = received_from_cansat[1];
  
  altitude = 44308.23 * (1.0 - pow(p_cansat / p_ground, 0.190284));

  SerialUSB.print("Temp ground: ");
  SerialUSB.println(t_ground);
  SerialUSB.print("Pressure ground: ");
  SerialUSB.println(p_ground);
  SerialUSB.print("Temp cansat: ");
  SerialUSB.println(t_cansat);
  SerialUSB.print("Pressure cansat: ");
  SerialUSB.println(p_cansat);
  SerialUSB.print("Altitude: ");
  SerialUSB.println(altitude);
  
  log_to_sd (last_rssi, counter, p_cansat, p_ground, t_cansat, t_ground, altitude);

  print_to_lcd(last_rssi, received_from_cansat[0], altitude);

  delay(1);
}

bool log_to_sd (int last_rssi, int package_counter, float pressure_cansat, float temperature_cansat, float pressure_ground, float temperature_ground, float altitude) {
  log_file = SD.open("log.txt", FILE_WRITE);
  if (log_file) {
    log_file.print("Package #");
    log_file.println(package_counter);
    log_file.print("RSSI = ");
    log_file.println(last_rssi);    
    log_file.print("Temperature cansat = ");
    log_file.print(temperature_cansat);
    log_file.println(" deg C");
    log_file.print("Presure cansat = ");
    log_file.print(pressure_cansat);
    log_file.println(" hPa");    
    log_file.print("Temperature ground = ");
    log_file.print(temperature_ground);
    log_file.println(" deg C");
    log_file.print("Presure ground = ");
    log_file.print(pressure_ground);
    log_file.println(" hPa");
    log_file.print("Altitude = ");
    log_file.print(altitude);
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

// convert analog reading to temperature in celsius 
float lm35_raw_to_temperature(int raw) {
  float voltage = raw * 3.3 / (std::pow(2, 12));
  float temperature = 100.0 * voltage;
  return temperature;
}
