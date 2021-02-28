#include <CanSatKit.h>
#include <cmath>
using namespace CanSatKit;
#include <SPI.h>
#include <SD.h>
#include <Servo.h>

int counter = 1;
bool led_state = false;
const int led_pin = 13;

unsigned long int sending_time_start = millis();
unsigned long int send_time;

// servo setup
int servo_pin = 9;
int servo_check_pin = 3;
//int button_pin = 2;
int starting_pos = 55; 
int finish_position = 160;
int servo_pos = 60; 
int delay_turn_off = 50; 

bool is_drill_made = false; 
bool on_ground = false;
bool sleeping_phase = false;

float last_measurments [10][2];

Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.0,
            Bandwidth_125000_Hz,
            SpreadingFactor_9,
            CodingRate_4_8);

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

// creting file object 
File log_file;

// creating servo object 
Servo myservo;

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

  if (!radio.begin()) {
    SerialUSB.println("Radio init failed");
  } else {
    SerialUSB.println("Radio init success");
  }
  if(!bmp.begin()) {
    SerialUSB.println("BMP init failed!");
    //while(1);
  } else {
    SerialUSB.println("BMP init success!");
  }
  
  bmp.setOversampling(16);
  analogReadResolution(12);

  // open serial port 
  Serial.begin(9600);
  while (!Serial) {
    delay (1);
  }

  if (!SD.begin(11)) {
    Serial.println("initialization failed!");
    while (1) {
      SerialUSB.println("Tu jesteś");
    }
  }
  //servo_setup();
}

void loop() {
  
  digitalWrite(led_pin, led_state);
  led_state = !led_state;

  double T, P;

  // start measurement, wait for result and save results in T and P variables 
  bmp.measureTemperatureAndPressure(T, P);
  
  SerialUSB.print("Pressure bmp: ");
  // print also value of pressure with two decimal places
  SerialUSB.print(P, 2);
  // print units - in this case the library returns hPa
  SerialUSB.println(" hPa ");
  
  SerialUSB.print("Temperature bmp: ");
  // print also value of pressure with two decimal places
  SerialUSB.print(T, 2);
  // print units - in this case the library returns hPa
  SerialUSB.println(" hPa ");

  int raw = analogRead(lm35_pin);

  // use lm35_raw_to_temperature function to calculate temperature
  float temperature = lm35_raw_to_temperature(raw);

  // print temperature on SerialUSB
  SerialUSB.print("Temperature lm: ");
  SerialUSB.print(temperature);
  SerialUSB.println(" deg C");

  SerialUSB.print("Temperature difference: ");
  if (temperature >= T) {
    SerialUSB.print(temperature - T);
  } else {
    SerialUSB.print(T- temperature);
  }
  SerialUSB.println(" deg C");
  
  send_time = millis () - sending_time_start;
  
  send_measurments_via_radio(counter, P, temperature, send_time);

  log_to_sd(counter, P, temperature, send_time);
  
  counter++;
  /*
  on_ground = is_on_ground ()

  if (on_ground && !is_drill_made) {
    SerialUSB.println("Stand Up phase...");
    stand_up(1, 500);
    delay (500);
    //make_drill ();
    // take sample
    // measure sample
    sleeping_phase = true; 
    is_drill_made = true; 
  }  
  */
  
  // wait for 1/2 s 
  delay(500);
}
bool send_measurments_via_radio (int package_counter, float pressure, float temp, unsigned int send_time) {

  float packet[] = {(float)package_counter, pressure, temp, (float) send_time};
  
  if (!radio.transmit((uint8_t *)(packet), sizeof packet)) {
    SerialUSB.println("Sending failed!");
    return false;
  } else {
    SerialUSB.println("Sending success");
    return true;
  }
}

bool log_to_sd (int package_counter, float pressure, float temp, unsigned int send_time) {
  log_file = SD.open("log.txt", FILE_WRITE);
  if (log_file) {
    log_file.print("Package send time = ");
    log_file.println(send_time);
    log_file.print("Package #");
    log_file.println(package_counter);   
    log_file.print("Temperature cansat = ");
    log_file.print(temp);
    log_file.println(" deg C");
    log_file.print("Presure cansat = ");
    log_file.print(pressure);
    log_file.println(" hPa");    
    log_file.close();
    SerialUSB.println("Data saved");
    return true;
  } else {
    SerialUSB.println("Opening file failed!");  
    return false;
  } 
}

bool is_on_ground () {
  return true;
}

// servo part 
bool servo_setup () {

  delay (3000);
  SerialUSB.println("Servo s");
  //pinMode(button_pin, INPUT_PULLUP);
  pinMode(servo_check_pin, OUTPUT);
  digitalWrite(servo_check_pin, LOW);
  myservo.attach(servo_pin);
  delay (delay_turn_off);
  digitalWrite(servo_check_pin, HIGH);
  delay(delay_turn_off);
  myservo.write(servo_pos);
  delay (delay_turn_off);
  digitalWrite(servo_check_pin, LOW);

  return true; 
}

bool stand_up (int number_of_tries, int delay_time) {
  for (int i = 1; i < number_of_tries; i++) {
    servo_move (true);
    delay (delay_time);
    servo_move (false);
    delay (delay_time);
  }
  servo_move(true);

  return true; 
}

bool servo_move (bool move_direction) {
  //SerialUSB.println("Waiting fot button...");
  //while (digitalRead(button_pin) != 0 ) {
    //delay (1);
  //}
  int start_position = 55; 
  int finish_position = 160;
  delay (1);

  SerialUSB.println("Starting servo rotation");

  // if move direction is True the servo will open the 
  if (move_direction) {

    digitalWrite(servo_check_pin, HIGH);
    delay (delay_turn_off);
    myservo.write(start_position);
    delay (delay_turn_off);
    digitalWrite(servo_check_pin, LOW);
  
    for (int i = starting_pos + 1; i <= finish_position; i++) {
    
      servo_pos = i;
      digitalWrite(servo_check_pin, HIGH);
      delay (delay_turn_off);
      myservo.write(servo_pos);
      delay (delay_turn_off);
      digitalWrite(servo_check_pin, LOW);
  
      SerialUSB.print("Servo position: ");
      SerialUSB.println(servo_pos);
      SerialUSB.print("Direction: ");
      SerialUSB.println("closing");
  
      delay (5); 
    }
  } else {
    
    digitalWrite(servo_check_pin, HIGH);
    delay (delay_turn_off);
    myservo.write(finish_position);
    delay (delay_turn_off);
    digitalWrite(servo_check_pin, LOW);
    
    for (int i = finish_position - 1; i >= start_position; i--) {
    
      servo_pos = i;
      digitalWrite(servo_check_pin, HIGH);
      delay (delay_turn_off);
      myservo.write(servo_pos);
      delay (delay_turn_off);
      digitalWrite(servo_check_pin, LOW);
  
      SerialUSB.print("Servo position: ");
      SerialUSB.println(servo_pos);
      SerialUSB.print("Direction: ");
      SerialUSB.println("closing");
      
      delay (5);
    }
  }
  return true; 
}
