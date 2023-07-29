#include <EngineController.h>
#include <Button.h>
#include <DistanceSensors.h>
#include <AnalogSensor.h>
#include "BluetoothSerial.h"

//Motores
#define PIN_RIGHT_ENGINE_DIR 16
#define PIN_RIGHT_ENGINE_PWM 17
#define PIN_LEFT_ENGINE_DIR 18
#define PIN_LEFT_ENGINE_PWM 19
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12
//Velocidades para los motores (segun el caso)
#define SEARCH_SPEED 80
#define ATTACK_SPEED_LDR 255
#define ATTACK_SPEED 200
#define STRONG_ATTACK_SPEED 210
#define ATTACK_SPEED_AGGRESSIVE 230
#define AVERAGE_SPEED 100
//Para la estrategia SemiPasiva
int slowAttack = 40;
int lowAttackCont;
unsigned long currentTimeAttack = 0;
#define TICK_LOW_ATTACK 1600
#define TICK_ATTACK_SEARCH 1500

//Botones
#define PIN_BUTTON_BUTTON_START 34
#define PIN_BUTTON_STRATEGY 35

//Sensores de Distancia
#define PIN_SENSOR_RIGHT 26
#define PIN_SENSOR_LEFT 27
int distSharpRight;
int distSharpLeft;
#define RIVAL 35
#define RIVAL_NEAR 15

//Sensores de Tatami
#define PIN_SENSOR_TATAMI_RIGHT 25
#define PIN_SENSOR_TATAMI_LEFT 33
int leftTatamiRead;
int righTatamiRead;
#define BORDE_TATAMI 300

//Variables y constantes para el debug por bloutooth
#define DEBUG_SHARP 1
#define DEBUG_TATAMI 1
#define DEBUG_LDR 1
#define TICK_DEBUG_SHARP 500
#define TICK_DEBUG_TATAMI 500
#define TICK_DEBUG_LDR 500
unsigned long currentTimeSharp = 0;
unsigned long currentTimeTatami = 0;
unsigned long currentTimeLdr = 0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

//Seleccion de lado de giro
enum turnSide
{
  NONE_TURN,
  LEFT,
  RIGHT
};
int turnSide = NONE_TURN;

//Oled
#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels


//<------------------------------------------------------------------------------------------------------------->//
//Instancia de objetos para cada periferico del Robot
IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_BUTTON_START);
Isensor *sharpRight = new Sharp_GP2Y0A60S(PIN_SENSOR_RIGHT);
Isensor *sharpLeft = new Sharp_GP2Y0A60S(PIN_SENSOR_LEFT);
AnalogSensor *rightTatami = new AnalogSensor(PIN_SENSOR_TATAMI_RIGHT);
AnalogSensor *leftTatami = new AnalogSensor(PIN_SENSOR_TATAMI_LEFT);
//<------------------------------------------------------------------------------------------------------------->//
//Funcion para la lectura de los sensores
void sensorsReading()
  {
    distSharpRight = sharpRight->SensorRead();
    distSharpLeft = sharpLeft->SensorRead();
    righTatamiRead = rightTatami->SensorRead();
    leftTatamiRead = leftTatami->SensorRead();
  }
//<------------------------------------------------------------------------------------------------------------->//
//Funciones para imprimir las lecturas de los sensores por el serial Bluetooth
void printSharp()
{
  if (millis() > currentTimeSharp + TICK_DEBUG_SHARP)
  {
    currentTimeSharp = millis();
    SerialBT.print("Right dist: ");
    SerialBT.print(distSharpRight);
    SerialBT.print("  //  ");
    SerialBT.print("Left dist: ");
    SerialBT.println(distSharpLeft);
  }
}

void printTatami()
{
  if (millis() > currentTimeTatami + TICK_DEBUG_TATAMI)
  {
    currentTimeTatami = millis();
    SerialBT.print("Right tatami: ");
    SerialBT.print(righTatamiRead);
    SerialBT.print("  //  ");
    SerialBT.print("Left tatami: ");
    SerialBT.println(leftTatamiRead);
  }
}
//<------------------------------------------------------------------------------------------------------------->//
//Maquina de estados para la estrategia PASSIVE
enum passive
{
  STANDBY_PASSIVE,
  SEARCH_PASSIVE,
  TURN_RIGHT_PASSIVE,
  TURN_LEFT_PASSIVE,
  TATAMI_LIMIT_PASSIVE,
  ATTACK_PASSIVE
}; 
int passive = STANDBY_PASSIVE;
//Estrategia que espera al rival y cuando este se monta ataca
void Passive()
{
  switch (passive)
  {
    case STANDBY_PASSIVE:
    {
    Sami->Stop();
    if (buttonStart->GetIsPress()) passive = SEARCH_PASSIVE;
    break;
    }

    case SEARCH_PASSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) passive = TATAMI_LIMIT_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) passive = TURN_RIGHT_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) passive = TURN_LEFT_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) passive = ATTACK_PASSIVE;
      break;    
    }

    case TURN_RIGHT_PASSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) passive = TATAMI_LIMIT_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) passive = SEARCH_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) passive = TURN_LEFT_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) passive = ATTACK_PASSIVE;
      break;
    }

    case TURN_LEFT_PASSIVE:
    {
      Sami->Left(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) passive = TATAMI_LIMIT_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) passive = SEARCH_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) passive = TURN_RIGHT_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) passive = ATTACK_PASSIVE;
      break;
    }

    case ATTACK_PASSIVE:
    {
      Sami->Stop();
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) passive = SEARCH_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) passive = TURN_RIGHT_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) passive = TURN_LEFT_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL)
      {
        Sami->Forward(200);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) passive = TATAMI_LIMIT_PASSIVE;
      }
      break;
    }

    case TATAMI_LIMIT_PASSIVE: 
    {
    Sami->Backward(AVERAGE_SPEED, AVERAGE_SPEED);
    delay(300);
    if(leftTatamiRead > BORDE_TATAMI && righTatamiRead > BORDE_TATAMI) passive = SEARCH_PASSIVE;
    break;
    }
  }
}

//<------------------------------------------------------------------------------------------------------------->//
//Maquina de estados para la estrategia SemiPassive
enum semiPassive
{
  STANDBY_SEMI_PASSIVE,
  SEARCH_SEMI_PASSIVE,
  TURN_RIGHT_SEMI_PASSIVE,
  TURN_LEFT_SEMI_PASSIVE,
  LOW_ATTACK_SEMI_PASSIVE,
  TATAMI_LIMIT_SEMI_PASSIVE,
  ATTACK_SEMI_PASSIVE
}; 
int semiPassive = STANDBY_SEMI_PASSIVE;
//Estrategia que espera al rival y avanza de a poco buscando el rival
void SemiPassive()
{
  switch (semiPassive)
  {
    case STANDBY_SEMI_PASSIVE:
    {
    Sami->Stop();
    if (buttonStart->GetIsPress()) semiPassive = SEARCH_PASSIVE;
    break;
    }

    case SEARCH_SEMI_PASSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiPassive = TURN_RIGHT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiPassive = TURN_LEFT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiPassive = ATTACK_SEMI_PASSIVE;
      if (millis() > currentTimeAttack + TICK_ATTACK_SEARCH)
      {
        semiPassive = LOW_ATTACK_SEMI_PASSIVE;
      }
      break;    
    }

    case TURN_RIGHT_SEMI_PASSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiPassive = TURN_LEFT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiPassive = ATTACK_SEMI_PASSIVE;
      break;
    }

    case TURN_LEFT_SEMI_PASSIVE:
    {
      Sami->Left(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiPassive = TURN_RIGHT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiPassive = ATTACK_SEMI_PASSIVE;
      break;
    }

    case ATTACK_SEMI_PASSIVE:
    {
      Sami->Stop();
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiPassive = TURN_RIGHT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiPassive = TURN_LEFT_SEMI_PASSIVE;
      if (millis() > currentTimeAttack + TICK_LOW_ATTACK)
      {
        semiPassive = LOW_ATTACK_SEMI_PASSIVE;
      }
      if(distSharpRight <= 10 && distSharpLeft <= 10)
      {
        Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      }
      break;
    }

    case LOW_ATTACK_SEMI_PASSIVE:
    {
      lowAttackCont++;
      slowAttack = slowAttack + (lowAttackCont*10);
      Sami->Forward(slowAttack, slowAttack);
      delay(388);
      currentTimeAttack = millis();
      semiPassive = ATTACK_SEMI_PASSIVE;
      break;
    }

    case TATAMI_LIMIT_SEMI_PASSIVE: 
    {
      Sami->Backward(AVERAGE_SPEED, AVERAGE_SPEED);
      delay(300);
      if(leftTatamiRead > BORDE_TATAMI && righTatamiRead > BORDE_TATAMI) semiPassive = SEARCH_SEMI_PASSIVE;
      break;
    }
  }
}

//<------------------------------------------------------------------------------------------------------------->//
//Maquina de estados para la estrategia Semi-Agressive
enum semiAggressive
{
  STANDBY_SEMI_AGGRESSIVE,
  SEARCH_SEMI_AGGRESSIVE,
  TURN_RIGHT_SEMI_AGGRESSIVE,
  TURN_LEFT_SEMI_AGGRESSIVE,
  TATAMI_LIMIT_SEMI_AGGRESSIVE,
  ATTACK_SEMI_AGGRESSIVE
}; 
int semiAggressive = STANDBY_SEMI_AGGRESSIVE;
//Estrategia que va a buscar al rival de forma moderada
void SemiAggressive()
{
  switch (semiAggressive)
  {
    case STANDBY_SEMI_AGGRESSIVE:
    {
    Sami->Stop();
    if (buttonStart->GetIsPress()) passive = SEARCH_PASSIVE;
    break;
    }

    case SEARCH_SEMI_AGGRESSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiAggressive = TURN_RIGHT_SEMI_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiAggressive = TURN_LEFT_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_SEMI_AGGRESSIVE; 
    }

    case TURN_RIGHT_SEMI_AGGRESSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_SEMI_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiAggressive = SEARCH_SEMI_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiAggressive = TURN_LEFT_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_SEMI_AGGRESSIVE;
      break;
    }

    case TURN_LEFT_SEMI_AGGRESSIVE:
    {
      Sami->Left(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_SEMI_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiAggressive = SEARCH_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiAggressive = TURN_RIGHT_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_SEMI_AGGRESSIVE;
      break;
    }

    case ATTACK_SEMI_AGGRESSIVE:
    {
      Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiAggressive = SEARCH_SEMI_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiAggressive = TURN_RIGHT_SEMI_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiAggressive = TURN_LEFT_SEMI_AGGRESSIVE;
      if(leftTatamiRead < 250 || righTatamiRead < 250) semiPassive = TATAMI_LIMIT_SEMI_AGGRESSIVE;
      break;
    }

    case TATAMI_LIMIT_SEMI_AGGRESSIVE: 
    {
      Sami->Backward(AVERAGE_SPEED, AVERAGE_SPEED);
      delay(300);
    if(leftTatamiRead > BORDE_TATAMI && righTatamiRead > BORDE_TATAMI) semiAggressive = SEARCH_SEMI_AGGRESSIVE;
    break;
    }
  }
}
//<------------------------------------------------------------------------------------------------------------->//

void setup() 
{
  SerialBT.begin("sami");
  
}


void loop() 
{
  sensorsReading();

  //descomentar la estrategia a usar
  Passive();
  //SemiAggressive();
  //SemiPassive();
  
  printSharp();
  printTatami();
}