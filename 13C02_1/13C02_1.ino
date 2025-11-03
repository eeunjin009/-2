#include <Servo.h>

// Arduino pin assignment
#define PIN_SERVO 10

// -----------------------------------------------------------------
#define _DUTY_MIN 500   // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1500  // servo neutral position (90 degree)
#define _DUTY_MAX 2500  // servo full counter-clockwise position (180 degree)

//

#define _SERVO_SPEED 3.0       // servo speed limit (unit: degree/second) 
#define _POS_START _DUTY_MIN   // 0도에서 시작
#define _POS_TARGET _DUTY_MAX  // 180도로 이동 (180도 이동)
// ---

#define INTERVAL 20     // servo update interval (unit: msec)

// global variables
unsigned long last_sampling_time; // unit: msec

Servo myservo;

float duty_change_per_interval; // maximum duty difference per interval
float duty_curr;                // Current duty time
int duty_target;                // Target duty time

void setup() {
  // initialize GPIO pins
  myservo.attach(PIN_SERVO);
  duty_target = _POS_TARGET;
  duty_curr = _POS_START;
  myservo.writeMicroseconds((int)duty_curr);
  
  // initialize serial port
  Serial.begin(57600);

  // convert angular speed into duty change per interval
  // 180도 범위가 (_DUTY_MAX - _DUTY_MIN) us에 해당
  duty_change_per_interval =  
    (_SERVO_SPEED / 180.0) * (_DUTY_MAX - _DUTY_MIN) * (INTERVAL / 1000.0);

  // 시리얼 모니터에 현재 실험 정보 출력
  Serial.println("--- 도전과제 실험 1: 60초간 180도 이동 (3.0 deg/s) ---");
  Serial.print("Target Duty (us): "); Serial.println(duty_target);
  Serial.print("Duty Change per Interval (us): "); Serial.println(duty_change_per_interval, 6);

  // initialize last sampling time
  last_sampling_time = millis();
}

void loop() {
  // wait until next sampling time
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  // update last sampling time
  last_sampling_time += INTERVAL;

  // 목표 위치에 도달하면 정지
  if (duty_curr >= duty_target) {
    duty_curr = duty_target; // 정확히 목표 위치로 설정
    myservo.writeMicroseconds((int)duty_curr);
    Serial.println("Target Reached. Servo Stopped.");
    return; 
  }

  // adjust duty_curr toward duty_target by duty_change_per_interval
  // 목표(duty_target)가 현재 위치(duty_curr)보다 크므로 증가
  if (duty_target > duty_curr) {
    duty_curr += duty_change_per_interval;
    // 오버슈트 방지
    if (duty_curr > duty_target)
      duty_curr = duty_target;
  }

  // update servo position
  myservo.writeMicroseconds((int)duty_curr);

  // output the read value to the serial port
  Serial.print("Min:"); Serial.print(_DUTY_MIN);
  Serial.print(", duty_target:"); Serial.print(duty_target);
  Serial.print(", duty_curr:"); Serial.print(duty_curr, 2);
  Serial.print(", Max:"); Serial.println(_DUTY_MAX);
}
