// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.3   // EMA weight of new sample (range: 0 to 1)
                          // Setting EMA to 1 effectively disables EMA filter.
#define N_MEDIAN 30   ///N 을 3, 10, 30으로 해보

// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_prev = _DIST_MAX;        // Distance last-measured
float dist_ema;                     // EMA distance
float samples_med[N_MEDIAN];    
int idx_med = 0;  

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);

for(int i=0; i<N_MEDIAN; ++i) samples_med[i] = _DIST_MAX;
}

float calc_median(float* buf, int n) {
  float tmp[n];
  for(int i=0;i<n;i++) tmp[i]=buf[i];
  // 오름차순 단순정렬(삽입)
  for(int i=1;i<n;i++){
    float t=tmp[i]; int j=i-1;
    while(j>=0 && tmp[j]>t){tmp[j+1]=tmp[j];j--;}
    tmp[j+1]=t;
  }
  return tmp[n/2];
}

void loop() {
  float dist_raw, dist_filtered, dist_med;
  
  // wait until next sampling time. 
  // millis() returns the number of milliseconds since the program started. 
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  // Modify the below if-else statement to implement the range filter
if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX) || (dist_raw < _DIST_MIN)) {
    dist_filtered = dist_prev; // 범위 벗어나면 이전 값 사용
} else {
    dist_filtered = dist_raw;  // 정상 범위면 현재 값 사용
    dist_prev = dist_raw;      // 이전 값 업데이트
}

  // Modify the below line to implement the EMA equation
dist_ema = _EMA_ALPHA * dist_filtered + (1 - _EMA_ALPHA) * dist_ema;

  samples_med[idx_med++] = dist_filtered;
  if(idx_med >= N_MEDIAN) idx_med = 0;
  dist_med = calc_median(samples_med, N_MEDIAN);


  // output the distance to the serial port
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",raw:"); Serial.print(min(dist_raw, _DIST_MAX + 100));
  Serial.print(",filtered:");  Serial.print(min(dist_filtered, _DIST_MAX + 100));
  Serial.print(",ema:");  Serial.print(min(dist_ema, _DIST_MAX + 100));
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  int pwm_val = 255;
  if ((dist_med <= _DIST_MIN) || (dist_med >= _DIST_MAX)) {
    pwm_val = 255;
  } else if (dist_med <= 200.0) {
    pwm_val = 255 - (int)((dist_med - _DIST_MIN) * (255.0 / (200.0 - _DIST_MIN)));
  } else {
    pwm_val = 255 - (int)((_DIST_MAX - dist_med) * (255.0 / (_DIST_MAX - 200.0)));
  }
  analogWrite(PIN_LED, pwm_val);

  last_sampling_time += INTERVAL;
}

// 초음파 거리 측정 함수
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
