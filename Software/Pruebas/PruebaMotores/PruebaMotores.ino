#include <EngineController.h>

#define PIN_RIGHT_ENGINE_DIR 18
#define PIN_RIGHT_ENGINE_PWM 19
#define PIN_LEFT_ENGINE_DIR 16
#define PIN_LEFT_ENGINE_PWM 17
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12

IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);
void setup()
{
}

void loop()
{
    Sami->Forward(200);
    delay(3000);
    Sami->Backward(200);
    delay(3000);
    Sami->Left(200);
    delay(3000);
    Sami->Right(200);
    delay(3000);
    Sami->Stop();
    delay(3000);
}