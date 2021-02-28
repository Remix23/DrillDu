// include CanSatKit library used for pressure sensor
#include <CanSatKit.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

using namespace CanSatKit;

float altitiude;
int last_rssi;
int nr_of_digits_alti;
int nr_of_digits_counter;
int nr_of_digits_rssi;

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

File log_file;

const int rs = 13, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,                  // frequency in MHz
            Bandwidth_125000_Hz,    // bandwidth - check with CanSat regulations to set allowed value
            SpreadingFactor_9,      // see provided presentations to determine which setting is the best
            CodingRate_4_8);        // see provided presentations to determine which setting is the best

void setup() {
  SerialUSB.begin(9600);
  lcd.begin(16, 2);
  radio.begin();

  // initializing SD card 
  if (!SD.begin(11)) {
    SerialUSB.println("SD card init failed!");
    SerialUSB.println("Restart device to continue");
  } else {
    SerialUSB.println("SD card init ok!");
  }

  // use begin() function to check if sensor is conneted and is able to communicate
  // with the CanSat board
  if(!bmp.begin()) {
    // if connection failed - print message to the user
    SerialUSB.println("BMP init failed!");
  } else {
    // print message to the user if everything is OK
    SerialUSB.println("BMP init success!");
  }
  
  // setOversampling function allows to set resolution of the pressure sensor
  // possible values of setOversampling:
  //  1 -- 2.62 Pa
  //  2 -- 1.31 Pa
  //  4 -- 0.66 Pa
  //  8 -- 0.33 Pa
  // 16 -- 0.16 Pa 
  bmp.setOversampling(16);
}

void loop() {

  double received_from_cansat [3];
  
  double T, pressure_ground;

  radio.receive((char *)(received_from_cansat));

  last_rssi = radio.get_rssi_last();
  
  bmp.measureTemperatureAndPressure(T, pressure_ground); // get presure on the ground
  
  // get and print signal level (rssi)
  SerialUSB.print("Received (RSSI = ");
  SerialUSB.print(last_rssi);
  SerialUSB.println("): ");

  // print package_counter
  SerialUSB.println(received_from_cansat[1]);
  SerialUSB.println(pressure_ground);

  altitiude = 44308.23 * (1.0 - pow(received_from_cansat[1] / pressure_ground, 0.190284));

  log_to_sd (last_rssi, received_from_cansat[0], received_from_cansat[1], pressure_ground, received_from_cansat[2], altitiude);

  print_to_lcd(last_rssi, received_from_cansat[0], altitiude);
}

bool log_to_sd (int last_rssi, int package_counter, double pressure_cansat, double pressure_ground, double temp, double alti) {
  log_file = SD.open("log.txt", FILE_WRITE);
  if (log_file) {
    log_file.print("Package #");
    log_file.println(package_counter);
    log_file.print("RSSI = ");
    log_file.println(last_rssi);
    log_file.print("Presure cansat = ");
    log_file.print(pressure_cansat);
    log_file.println(" hPa");
    log_file.print("Presure groud = ");
    log_file.print(pressure_ground);
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

bool print_to_lcd (int last_rssi, int package_counter, double alti) {
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
