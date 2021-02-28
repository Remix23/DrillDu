#include <CanSatKit.h>
#include <cmath>
using namespace CanSatKit;
#include <SPI.h>
#include <SD.h>

File myFile;
int counter = 1;
bool led_state = false;
const int led_pin = 13;

Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,
            Bandwidth_125000_Hz,
            SpreadingFactor_9,
            CodingRate_4_8);

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

// port where LM35 temperature sensor is connected
const int lm35_pin = A0;

// a function that calculates temperature (in *C) from analogRead raw reading
// see datasheet of LM35 and analogRead Arduino function documentation
float lm35_raw_to_temperature(int raw) {
  float voltage = raw * 3.3 / (std::pow(2, 12));
  float temperature = 100.0 * voltage;
  return temperature;
}

void setup() {
  SerialUSB.begin(115200);
  pinMode(led_pin, OUTPUT);

  radio.begin();
    // use begin() function to check if sensor is conneted and is able to communicate
  // with the CanSat board
  if(!bmp.begin()) {
    // if connection failed - print message to the user
    SerialUSB.println("BMP init failed!");
    // the program will be 'halted' here and nothing will happen till restart
    while(1);
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
  analogReadResolution(12);
    // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (!SD.begin(11)) {
    Serial.println("initialization failed!");
    while (1);
  }

}

void loop() {
  digitalWrite(led_pin, led_state);
  led_state = !led_state;

  double T, P;

  // start measurement, wait for result and save results in T and P variables 
  bmp.measureTemperatureAndPressure(T, P);
  
  // since we use it solely as pressure sensor print only "Pressure = " text
  SerialUSB.print(" ");
  // print also value of pressure with two decimal places
  SerialUSB.print(P, 2);
  // print units - in this case the library returns hPa
  SerialUSB.print(" ");

  int raw = analogRead(lm35_pin);

  // use lm35_raw_to_temperature function to calculate temperature
  float temperature = lm35_raw_to_temperature(raw);

  // print temperature on SerialUSB
  SerialUSB.print(" ");
  SerialUSB.print(temperature);
  SerialUSB.println(" ");
  
  send_measurments_via_radio(counter, P, temperature);
  
  myFile = SD.open("test.txt", FILE_WRITE);

  if (myFile) {
    myFile.println(counter);
    myFile.println(P);
    myFile.println(temperature);
    myFile.close();
  }
  counter++;

  // wait for 1 s
  delay(1000);
}
bool send_measurments_via_radio (int package_counter, float presure, float temp) {

  float packet[] = {(float)package_counter, presure, temp};
  
  if (!radio.transmit((uint8_t *)(packet), sizeof packet)) {
    SerialUSB.println("Sending failed!");
    return false;
  } else {
    SerialUSB.println("Sending success");
    return true;
  }
}
