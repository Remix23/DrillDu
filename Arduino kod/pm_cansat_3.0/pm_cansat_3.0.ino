#include <CanSatKit.h>
#include <cmath>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>

// mpu readings 
#include <MPU6050_tockn.h>
#include <Wire.h>

using namespace CanSatKit;

int counter = 1;
bool led_state = false;
const int led_pin = 13;

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
bool is_on_ground = false;
bool sleeping_phase = false;

// check last measuremnts
float last_pressure_measurements [10];
float last_temperature_measurements [10];

// mpu data packet 
float mpu_data [15];

// delay time 
int delay_time = 300;

// active devices 
bool is_sd_active = false;
bool is_bmp_active = false;
bool is_lm_active = false;
bool is_radio_active = false;
bool is_mpu_active = false;

// INSTANCES \\

Radio radio(Pins::Radio::ChipSelect,
            Pins::Radio::DIO0,
            433.7,
            Bandwidth_125000_Hz,
            SpreadingFactor_9,
            CodingRate_4_8);

// BMP280 is a pressure sensor, create the sensor object
BMP280 bmp;

// creting file object 
File log_file;

// creating servo object 
Servo myservo;

// mpu object 
MPU6050 mpu6050(Wire)

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
  Wire.begin();
  pinMode(led_pin, OUTPUT);

  if (is_lm_active) analogReadResolution(12);
  
  radio_start();
  bmp_start ();
  sd_start ();
  mpu_start ();
  
  //servo_setup(); // uncoment the first comment to allow servo setup
}

void loop() {
  
  if (is_bmp_active && is_radio_active) {
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

    // update values in list 
    last_pressure_measurements[counter / 10] = P;
    last_temperature_measurements[counter / 10] = T;

    float temperature = T;

    if (is_lm_active) {
    // use lm35_raw_to_temperature function to calculate temperature
      temperature = lm35_raw_to_temperature(analogRead(lm35_pin));
    
      // print temperature on SerialUSB
      SerialUSB.print("Temperature lm: ");
      SerialUSB.print(temperature);
      SerialUSB.println(" deg C");
    
      SerialUSB.print("Temperature difference: ");
      if (temperature >= T) {
        SerialUSB.print(temperature - T);
      } else {
        SerialUSB.print(T - temperature);
      }
      SerialUSB.println(" deg C");
    }
    
    send_time = millis ();
    
    send_measurments_via_radio(counter, P, temperature, send_time);

    if (is_sd_active) {
      log_to_sd(counter, P, temperature, send_time);
    } else {
      sd_start ();
      delay (1);
    }
    
    counter++;
    
  } else {
    if (!is_radio_active) {
      radio_start ();
      delay (1);
    }
    if (!is_bmp_active) {
      bmp_start();
      delay(1);
    }
  }
  // uncomment the next session if you want the servo to move
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
  delay(delay_time);
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

// setup part 

bool mpu_start () {
  if (!mpu6050.begin()) {
    SerialUSB.println("MMPU init failed");
    is_mpu_active = false;
    return false;
  } else {
    mpu6050.calcGyroOffsets(true);
    SerialUSB.println("MPU init success!");
    is_mpu_active = true;
    return true;
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

// check if on the ground 
bool check_is_on_ground () {
  update_mpu_data();
  if (check_last_pressure() && check_last_temperature()) {
    // checking mpu data
    is_on_ground = true;
    return true;
  }
  is_on_ground = false; 
  return false;
}

bool check_last_pressure () {
  int j;
  if (counter > 9) {
    j = 10;
  } else {
    j = counter;
  }
  float average = 0; 
  for (int i = 0; i < j; i++) {
    average += last_pressure_measurements [i];
  }
  average /= j;
  for (int i = 0; i < j; i++) {
    if (last_pressure_measurements[i] > average) {
      if (last_pressure_measurements[i] - average > 1) {
        return false;
      }
    } else {
      if (average - last_pressure_measurements[i] > 1) {
        return false;
      }
    }
  }
  return true; 
}

bool check_last_temperature () {
  int j;
  if (counter > 9) {
    j = 10;
  } else {
    j = counter; 
  }
  float average = 0; 
  for (int i = 0; i < j; i++) {
    average += last_temperature_measurements [i];
  }
  average /= j;
  for (int i = 0; i < j; i++) {
    if (last_temperature_measurements[i] > average) {
      if (last_temperature_measurements[i] - average > 1) {
        return false;
      }
    } else {
      if (average - last_temperature_measurements[i] > 1) {
        return false;
      }
    }
  }
  return true; 
}

bool update_mpu_data () {

  mpu6050.update();
  
  mpu_data [0] = mpu6050.getTemp();
  mpu_data [1] = mpu6050.getAccX();
  mpu_data [2] = mpu6050.getAccX();
  mpu_data [3] = mpu6050.getAccZ();
  mpu_data [4] = mpu6050.getGyroX;
  mpu_data [5] = mpu6050.getGyroY();
  mpu_data [6] = mpu6050.getGyroZ();
  mpu_data [7] = mpu6050.getAccAngleX();
  mpu_data [8] = mpu6050.getAccAngleY();
  mpu_data [9] = mpu6050.getGyroAngleX();
  mpu_data [10] = mpu6050.getGyroAngleY();
  mpu_data [11] = mpu6050.getGyroAngleZ();
  mpu_data [12] = mpu6050.getAngleX();
  mpu_data [13] = mpu6050.getAngleY();
  mpu_data [14] = mpu6050.getAngleZ();
  
  return true;
}

bool check_mpu_data () {
  // checking mpu data -> does it is repeating (ground) or it still changing (flight)
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

// update delay time 
int change_delay (int current_delay) {
  if (!is_on_ground) {
    return 300; // delay for the flight 
  } else {
    if (millis() < 3600000){
      if (!is_drill_made) {
        return 300; // if we are in the ground but the drill doesn't go 
      } else {
        return 700; // we are on the ground after the drill
      }
    } else {
      sleeping_phase = true;
      return 1; // we are after 4 hours of sendinng data, we can slow down
    }
  }

}

// drilling 
bool drill_move (bool move_direction) {
  return true;
}

bool fan_start_stop (bool mode) {
  if (mode) {
    return true;
  } else {
    return false;
  }
}
