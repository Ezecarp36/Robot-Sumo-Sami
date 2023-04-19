#include <EngineController.h>

#define PIN_RIGHT_ENGINE_DIR 19
#define PIN_RIGHT_ENGINE_PWM 18
#define PIN_LEFT_ENGINE_DIR 22
#define PIN_LEFT_ENGINE_PWM 21
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12

int rightSpeed = 255;
int leftSpeed = 255;
#define TICK_TURN_FRONT 59
#define TICK_TURN_SIDE 95
#define TICK_SHORT_BACK_TURN 115
#define TICK_LONG_BACK_TURN 120

IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);


void setup() {
}

void loop() {
  Sami->Right(rightSpeed, leftSpeed);
    delay(TICK_TURN_FRONT);
  Sami->Stop();
  delay(3000);
  Sami->Right(rightSpeed, leftSpeed);
  delay(TICK_TURN_SIDE);
  Sami->Stop();
  delay(3000);
  Sami->Right(rightSpeed, leftSpeed);
  delay(TICK_SHORT_BACK_TURN);
  Sami->Stop();
  delay(3000);
  Sami->Right(rightSpeed, leftSpeed);
  delay(TICK_LONG_BACK_TURN);
  Sami->Stop();
  delay(3000);
}
