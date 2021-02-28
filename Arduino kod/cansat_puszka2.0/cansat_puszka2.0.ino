#include <CanSatKit.h>
#include <SD.h>
#include <SPI.h>
#include <cmath>

using namespace CanSatKit;

int counter = 1;
bool led_state = false;
const int led_pin = 13;

// create new instance of class radio (it will be used later)
Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,
            Bandwidth_125000_Hz,
            SpreadingFactor_9,
            CodingRate_4_8);

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

//  define log and test file 
File log_file;

void setup() {
  SerialUSB.begin(9600);
  pinMode(led_pin, OUTPUT);

  
  // starting radio
  if (!radio.begin()){
    SerialUSB.println("Radio init failed!"); // it returns true if radio is ok, else it returns false 
    SerialUSB.println("Restart device to continue");
    while(1);
  } else {
    SerialUSB.println("Radio init ok!");
    radio.disable_debug();
  }

  // initializing SD card 
  if (!SD.begin(11)) {
    SerialUSB.println("SD card init failed!");
    SerialUSB.println("Restart device to continue");
    while (1);
  } else {
    SerialUSB.println("SD card init ok!");
  }

  // open test file
  test_file = SD.open("test.txt", FILE_WRITE);

  if (test_file) {
    SerialUSB.println("Writing to test.txt...");
    test_file.println("testing 1, 2, 3.");
    test_file.close();
    SerialUSB.println("done. ");
    delay (500);
    SD.remove("test.txt");
  } else {
    SerialUSB.println("Opening test.txt failed");
  }

  // starting BMP280 sensor 
  if(!bmp.begin()) {
    // if connection failed - print message to the user
    SerialUSB.println("BMP init failed!");
    while(1);
  } else {
    // print message to the user if everything is OK
    SerialUSB.println("BMP init success!");
  }

  bmp.setOversampling(16);/*

  unsigned int delay_for_BMP280_sensor = bmp.startMeasurment();

  SerialUSB.print("Delay for starting BMP sensor");
  SerialUSB.println(delay_for_BMP280_sensor);

  delay(delay_for_BMP280_sensor);*/
}

void loop() {
  digitalWrite(led_pin, led_state);
  led_state = !led_state;

  // sensors part 
  
  // BMP280
  // declare variables for temperature (T) and presure (P) readings
  double T, P;  

  bmp.measureTemperatureAndPressure(T, P);
  
  // print values on serial 
  SerialUSB.print("Presure = ");
  SerialUSB.print(P, 4);
  SerialUSB.println(" hPa");

  SerialUSB.print("Temperature (from BMP sensor) = ");
  SerialUSB.print(T, 4);
  SerialUSB.println(" deg C");

  send_measurments_via_radio(counter, P, T);

  log_to_sd(counter, P, T);
    
  counter++;

  // wait for 1 s
  delay(200);
}

bool log_to_sd (int package_counter, float presure, float temp) {
  log_file = SD.open("log.txt", FILE_WRITE);
  if (log_file) {
    log_file.print("Package #");
    log_file.println(package_counter);
    log_file.print("Presure = ");
    log_file.print(presure);
    log_file.println(" hPa");
    log_file.print("Temperature");
    log_file.print(temp);
    log_file.println(" deg C");
    log_file.close();
    SerialUSB.println("Data saved");
    return true;
  } else {
    SerialUSB.println("Opening file failed!");  
    return false;
  }
}

bool send_measurments_via_radio (int package_counter, float presure, float temp) {

  double packet[] = {(float) package_counter, presure, temp};
  
  if (!radio.transmit((uint8_t *)(packet), sizeof packet)) {
    SerialUSB.println("Sending failed!");
    return false;
  } else {
    SerialUSB.println("Sending success");
    return true;
  }
}
