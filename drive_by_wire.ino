const int hall_sensor_pin = A0;
const int blazing_speed_sensor_pin = A1;
const int sport_switch_sensor_pin = A2;

const int voltage_5 = 1020;
int hall_min = 200;
int hall_max = 600;
const int throttle_pin = 3; // Pin 3 is used for PWM output

const float kid_mode = .30;
const float sport_mode = .50;
const float blazing_throttle = 70;
const float dead_band = .025;

float driving_mode;
int max_throttle;
int blazing_speed = 0;


float throttle = 0; // 0-100

unsigned long throttle_timer = 0;
unsigned long blazing_speed_timer = 0;
unsigned long lastUpdateTime = 0;

void setup()
{
  Serial.println("starting setup");
  // put your setup code here, to run once:
  pinMode(throttle_pin, OUTPUT);  // sets the pin as output

  int sensor_value = analogRead(sport_switch_sensor_pin);

  if(sensor_value > voltage_5)
  {
    driving_mode = sport_mode;
  }
  else
  {
    driving_mode = kid_mode;
  }

  max_throttle = 100 * driving_mode;

  Serial.begin(9600);
}

int sanitize_hall(int value)
{
  if(value < hall_min)
  {
     hall_min = value;
    //value = hall_min;
  }
  else if(value > hall_max)
  {
    //value = hall_max;
    hall_max = value;
  }
  return value;
}

int limit_throttle_rate(int new_throttle_value)
{
  delay(200);
  unsigned long currentTime = millis();  // Get the current time in milliseconds
  unsigned long elapsedTime = currentTime - lastUpdateTime;  // Calculate elapsed time

  // Calculate the maximum allowed increase based on the elapsed time
  float maxIncrease = max_throttle * driving_mode * 2 * elapsedTime / 1000.0;  // 0.3 represents 30% per second
  // Update the last update time
  lastUpdateTime = currentTime;

  // Limit the increase to the maximum allowed increase
  if (new_throttle_value - throttle > maxIncrease)
  {
    new_throttle_value = throttle + maxIncrease;
  }

  return new_throttle_value;
}

int throttle_response(int hall_sensor)
{
//  int new_throttle_value = map(hall_sensor,(hall_min*(1+dead_band)), (hall_max*(1-dead_band)), 0, max_throttle);
  int new_throttle_value = map(hall_sensor,hall_min, hall_max, 0, max_throttle);
 // new_throttle_value = limit_throttle_rate(new_throttle_value);

  // if(new_throttle_value < sport_mode * max_throttle)
  // {
  //     throttle_timer = millis(); //low throttle moves the timer forward so elapsed time will be zero
  // } 

  return new_throttle_value;
}

int check_for_blazing_speed(int blazing_speed_value, int throttle)
{

  if(throttle < sport_mode * max_throttle)
  {
    blazing_speed = false;
  }
  else if(blazing_speed == true )
  {
    if(millis() - blazing_speed_timer < 5000)
    {
      throttle = blazing_throttle;
    }
    else
    {
      blazing_speed = false;
      throttle_timer = millis(); //
    }
  }
  else if (blazing_speed_value > voltage_5 && millis() - throttle_timer > 5000 )
  {
    blazing_speed_timer = millis(); //
    blazing_speed = true;
    throttle = blazing_throttle;
  }
  else
  {
    blazing_speed = false;
  }

  return throttle;
}

void loop()
{
  int hall_value = analogRead(hall_sensor_pin); // Read the analog value from the sensor
  int blazing_speed_value = analogRead(blazing_speed_sensor_pin); // Read the analog value from the sensor
  int sport_switch_value = analogRead(sport_switch_sensor_pin);

  if(sport_switch_value < voltage_5)
  {
    driving_mode = kid_mode;
    max_throttle = 100 * driving_mode;
  }

  hall_value = sanitize_hall(hall_value);
  throttle = throttle_response(hall_value);

  if(driving_mode == sport_mode)
  {
    throttle = check_for_blazing_speed(blazing_speed_value, throttle);
  }

  analogWrite(throttle_pin, map(throttle,0,100,0,255));
 //   analogWrite(throttle_pin, 255);
 // Serial.print(hall_value);
    Serial.print("throttle value ");
  Serial.print(throttle);
   Serial.print("blazing speed value ");
   Serial.print(blazing_speed_value);
    Serial.print("sport mode value ");
   Serial.println(sport_switch_value);

}