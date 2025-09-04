#include <Wire.h>
#include <MPU6050.h>

/* ==== PINS (Arduino UNO) ====
   Left motor: IN1=D8, IN2=D7, ENA=D10  (CH-A)
   Right motor: IN3=D4, IN4=D3, ENB=D9  (CH-B)
   NOTE: Mapping verified during testing:
   - left  → CH-B, -pwm
   - right → CH-A, -pwm
*/
const uint8_t IN1 = 8, IN2 = 7, ENA = 10;   // CH-A (driver channel A)
const uint8_t IN3 = 4, IN4 = 3, ENB = 9;    // CH-B (driver channel B)

/* ==== PARAMETERS ==== */
#define ORIENTATION_AX 1

// Balance sign: assumed +angleDeg > 0 when tilting forward.
// If motors react in the wrong direction, set the value below to -1.
#define ERR_SIGN (-1)

const int   MIN_PWM   = 45;        // threshold to overcome friction (soft start ~50)
const float SAFE_TILT = 60.0f;     // stop motors if tilt exceeds this angle
const float DEAD_DEG  = 0.15f;     // dead zone for small angle errors
const float CF_ALPHA  = 0.99f;     // complementary filter coefficient (0.98–0.995 recommended)

/* ==== PID (soft start) ==== */
float kP = 45.0f, kI = 0.2f, kD = 6.5f; 
const float D_ALPHA = 0.35f;       // low-pass filter for derivative (gyro)

/* ==== Slew limiter (smooth step response) ==== */
const int SLEW_STEP = 2;           // max output change per 1 ms step

/* ==== IMU ==== */
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;
float angleDeg = 0.0f, accOffset = 0.0f, gyroBias = 0.0f;

/* ==== STATE ==== */
float integ = 0.0f, gyroFilt = 0.0f;
const unsigned long CONTROL_US = 1000;  // 1 kHz control loop
unsigned long nextTick = 0;

/* ==== Helpers ==== */
static inline float normalize180(float a){
  while (a > 180.0f) a -= 360.0f;
  while (a < -180.0f) a += 360.0f;
  return a;
}
float accelAngleDeg(){
#if ORIENTATION_AX
  return atan2((float)ax, (float)az) * 180.0f / PI;
#else
  return atan2((float)ay, (float)az) * 180.0f / PI;
#endif
}
float gyroRateDegS(){
#if ORIENTATION_AX
  return ((float)gx / 131.0f) - gyroBias;
#else
  return ((float)gy / 131.0f) - gyroBias;
#endif
}

/* ==== Basic drive: signed PWM (-255..+255), no deadtime ==== */
static inline int clip255(int v){ return v<0?0:(v>255?255:v); }

void driveSide(uint8_t IN_A, uint8_t IN_B, uint8_t EN, int pwm){
  if (pwm > 0){
    int v = clip255(pwm); if (v < MIN_PWM) v = MIN_PWM;
    digitalWrite(IN_A, HIGH); digitalWrite(IN_B, LOW);  analogWrite(EN, v);
  } else if (pwm < 0){
    int v = clip255(-pwm); if (v < MIN_PWM) v = MIN_PWM;
    digitalWrite(IN_A, LOW);  digitalWrite(IN_B, HIGH); analogWrite(EN, v);
  } else {
    digitalWrite(IN_A, LOW);  digitalWrite(IN_B, LOW);  analogWrite(EN, 0);
  }
}

/* ==== Left/Right mapping (must match tested configuration) ==== */
inline void leftPWM (int pwm){ driveSide(IN3, IN4, ENB, -pwm); }  // left → CH-B, -pwm
inline void rightPWM(int pwm){ driveSide(IN1, IN2, ENA, -pwm); }  // right → CH-A, -pwm

void driveMotorLR(int left, int right){
  leftPWM(left);
  rightPWM(right);
}

/* ==== Motor self-test ==== */
void motorSelfTest(){
  const int V = 180, T = 900;
  Serial.println(F("[TEST] Left forward"));  leftPWM(+V); delay(T); leftPWM(0); delay(300);
  Serial.println(F("[TEST] Left backward")); leftPWM(-V); delay(T); leftPWM(0); delay(400);
  Serial.println(F("[TEST] Right forward")); rightPWM(+V); delay(T); rightPWM(0); delay(300);
  Serial.println(F("[TEST] Right backward"));rightPWM(-V); delay(T); rightPWM(0); delay(400);
  Serial.println(F("[TEST] Both forward"));  driveMotorLR(+V,+V); delay(T); driveMotorLR(0,0); delay(300);
  Serial.println(F("[TEST] Both backward")); driveMotorLR(-V,-V); delay(T); driveMotorLR(0,0); delay(500);
  Serial.println(F("[TEST] Finished\n"));
}

/* ==== IMU calibration ==== */
void calibrateIMU(){
  Serial.println(F("Calibration: please hold upright."));
  delay(500);
  const int N = 500;
  long sumAcc_mdeg = 0; long sumG = 0;
  for (int i=0;i<N;i++){
    mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
#if ORIENTATION_AX
    sumAcc_mdeg += (long)(atan2((float)ax, (float)az) * 180.0f / PI * 1000.0f);
    sumG += gx;
#else
    sumAcc_mdeg += (long)(atan2((float)ay, (float)az) * 180.0f / PI * 1000.0f);
    sumG += gy;
#endif
    delayMicroseconds(800);
  }
  accOffset = (float)sumAcc_mdeg / (1000.0f * N);
  gyroBias  = ((float)sumG / (float)N) / 131.0f;

  angleDeg = 0.0f; integ = 0.0f; gyroFilt = 0.0f;

  Serial.print(F("accOffset=")); Serial.print(accOffset,2);
  Serial.print(F(" deg, gyroBias=")); Serial.print(gyroBias,3);
  Serial.println(F(" deg/s"));
}

/* ==== Setup ==== */
void setup(){
  Serial.begin(115200);
  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT); pinMode(ENA,OUTPUT);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT); pinMode(ENB,OUTPUT);

  Wire.begin();
  Wire.setClock(400000);

  // Serial.println(F("Motor self-test starting..."));
  // motorSelfTest();
  delay(5000);
  
  mpu.initialize();
  if(!mpu.testConnection()){
    Serial.println(F("Warning: MPU6050 not detected (motor test will still run)"));
  } else {
    calibrateIMU();
  }

  delay(500);
  
  nextTick = micros();
}

/* ==== 1 kHz balance control loop ==== */
void fastControlStep(){
  // IMU
  mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
  float accDeg   = normalize180(accelAngleDeg() - accOffset);
  float gyroDegS = gyroRateDegS();

  // Complementary filter (dt=1ms)
  angleDeg = CF_ALPHA * (angleDeg + gyroDegS * 0.001f) + (1.0f - CF_ALPHA) * accDeg;

  // Safety check
  if (fabs(angleDeg) > SAFE_TILT){
    driveMotorLR(0,0);
    integ = 0; gyroFilt = 0;
    static unsigned long t=0; if (millis()-t>250){ t=millis(); Serial.println(F("SAFE_TILT")); }
    return;
  }

  // ===== PID (controlled by ERR_SIGN) =====
  float error = ERR_SIGN * angleDeg;
  if (fabs(error) < DEAD_DEG) error = 0;

  // I
  integ += error * 0.001f;
  integ = constrain(integ, -300.0f, 300.0f);

  // D (gyro-based + LPF)
  gyroFilt += D_ALPHA * (gyroDegS - gyroFilt);

  // PID sum
  float u = kP*error + kI*integ - kD*gyroFilt;

  // Output clamp
  u = constrain(u, -255, 255);
  int target = (int)u;

  // ===== Slew limiter =====
  static int lastOut = 0;
  if      (target > lastOut + SLEW_STEP) target = lastOut + SLEW_STEP;
  else if (target < lastOut - SLEW_STEP) target = lastOut - SLEW_STEP;
  lastOut = target;

  // Drive motors
  driveMotorLR(target, target);

  // Debug (20 Hz)
  static unsigned long lastLog = 0, nowMs = 0;
  nowMs = millis();
  if (nowMs - lastLog > 50){
    lastLog = nowMs;
    Serial.print("ang="); Serial.print(angleDeg,2);
    Serial.print(" err="); Serial.print(error,2);
    Serial.print(" u=");   Serial.print((int)u);
    Serial.print(" out="); Serial.println(target);
  }
}

void loop(){
  unsigned long now = micros();
  if ((long)(now - nextTick) >= 0) {
    nextTick += CONTROL_US;
    fastControlStep();
  }
}
