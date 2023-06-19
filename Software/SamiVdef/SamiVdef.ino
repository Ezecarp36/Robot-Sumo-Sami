#include <EngineController.h>
#include <Button.h>
#include <DistanceSensors.h>
#include <AnalogSensor.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Motores
#define PIN_RIGHT_ENGINE_DIR 16
#define PIN_RIGHT_ENGINE_PWM 17
#define PIN_LEFT_ENGINE_DIR 18
#define PIN_LEFT_ENGINE_PWM 19
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12
//Velocidades para los motores (segun el caso)
#define SEARCH_SPEED 75
#define ATTACK_SPEED_LDR 250
#define ATTACK_SPEED 180
#define STRONG_ATTACK_SPEED 210
#define ATTACK_SPEED_AGGRESSIVE 240
#define AVERAGE_SPEED 100
//Para la estrategia SemiPasiva
int slowAttack = 40;
int lowAttackCont;
unsigned long currentTimeAttack = 0;
#define TICK_LOW_ATTACK 1600
#define TICK_ATTACK_SEARCH 1500
//Ticks para los giros
int tickTurn;
#define TICK_TURN_FRONT 59
#define TICK_TURN_SIDE 95
#define TICK_SHORT_BACK_TURN 115
#define TICK_LONG_BACK_TURN 120
//Ticks de reposicionamiento
int tickForwardOrBackward;
#define TICK_SHORT 20
#define TICK_MEDIUM 30
#define TICK_LONG 40

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

//Variables y constantes LDR
#define PIN_SENSOR_LDR 23
#define MONTADO 100
int ldr;

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

//Seleccion de reposicionamiento
enum forwardOrBackward
{
  NONE,
  FORWARD,
  BACKWARD
};
int forwardOrBackward = NONE;

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
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_BUTTON_START);
IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);
Isensor *sharpRight = new Sharp_GP2Y0A60S(PIN_SENSOR_RIGHT);
Isensor *sharpLeft = new Sharp_GP2Y0A60S(PIN_SENSOR_LEFT);
AnalogSensor *rightTatami = new AnalogSensor(PIN_SENSOR_TATAMI_RIGHT);
AnalogSensor *leftTatami = new AnalogSensor(PIN_SENSOR_TATAMI_LEFT);
AnalogSensor *ldrSensor = new AnalogSensor(PIN_SENSOR_LDR);
//<------------------------------------------------------------------------------------------------------------->//
//Funcion para la lectura de los sensores
void sensorsReading()
  {
    distSharpRight = sharpRight->SensorRead();
    distSharpLeft = sharpLeft->SensorRead();
    righTatamiRead = rightTatami->SensorRead();
    leftTatamiRead = leftTatami->SensorRead();
    ldr = ldrSensor->SensorRead();
  }
//<------------------------------------------------------------------------------------------------------------->//
//Funciones para imprimir las lecturas de los sensores por el serial Bluetooth

void printLdr()
{
  if (millis() > currentTimeLdr + TICK_DEBUG_LDR)
  {
    currentTimeLdr = millis();
    SerialBT.print("Ldr: ");
    SerialBT.println(ldr);
  }
}
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
//Declaracion de variables y constantes para cada uno de los menus (para que al hacer las funciones no alla problemas de que use variables no declaradas)

//Menu principal
enum mainMenu
{
  POSITIONING_MENU,
  TURN_MENU,
  STRATEGIES_MENU,
  PASSIVE,
  SEMI_PASSIVE,
  SEMI_AGGRESSIVE,
  AGGRESSIVE,
};
int mainMenu = POSITIONING_MENU;

//Menu de reposicionamiento
enum positioningMenu
{
  MAIN_MENU,
  SELECT_FORWARD,
  SELECT_BACKWARD,
  ADVANCE_SHORT,
  ADVANCE_MEDIUM,
  ADVANCE_LONG,
  REVERSE_SHORT,
  REVERSE_MEDIUM,
  REVERSE_LONG,
};

int positioningMenu = MAIN_MENU;

//Menu de giro
enum turnMenu
{
  MAIN_MENU_TURN,
  LEFT_TURN,
  RIGHT_TURN,
  TURN_FRONT,
  TURN_SIDE,
  SHORT_BACK_TURN,
  LONG_BACK_TURN,
};
int turnMenu = MAIN_MENU_TURN;
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
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Strategy Passive"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 28);
    oled.println("Press Star()"); 
    oled.display();
    if (buttonStart->GetIsPress())
    {
      oled.clearDisplay();  
      oled.setCursor(0, 28);
      oled.println("---------------------"); 
      oled.display();
      delay(5000);
      if(turnSide == LEFT)
      {
        Sami->Left(ATTACK_SPEED);
        delay(tickTurn);
      }
      if(turnSide == RIGHT)
      {
        Sami->Right(ATTACK_SPEED);
        delay(tickTurn);
      }
      passive = SEARCH_PASSIVE;
    } 
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
      if(ldr < MONTADO)
      {
        Sami->Forward(ATTACK_SPEED_LDR, ATTACK_SPEED_LDR);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) passive = TATAMI_LIMIT_PASSIVE;
      }

      else 
      {
        Sami->Stop();
        if(distSharpRight > RIVAL && distSharpLeft > RIVAL) passive = SEARCH_PASSIVE;
        if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) passive = TURN_RIGHT_PASSIVE;
        if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) passive = TURN_LEFT_PASSIVE;
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
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Strategy Semi_Passive"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 28);
    oled.println("Press Star()"); 
    oled.display();
    if (buttonStart->GetIsPress())
    {
      oled.clearDisplay();  
      oled.setCursor(0, 28);
      oled.println("---------------------"); 
      oled.display();
      delay(5000);
      if(turnSide == LEFT)
      {
        Sami->Left(ATTACK_SPEED);
        delay(tickTurn);
      }
      if(turnSide == RIGHT)
      {
        Sami->Right(ATTACK_SPEED);
        delay(tickTurn);
      }
      passive = SEARCH_PASSIVE;
    } 
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
      if(ldr < MONTADO)
      {
        Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      }
      break;    
    }

    case TURN_RIGHT_PASSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiPassive = TURN_LEFT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiPassive = ATTACK_SEMI_PASSIVE;
      if(ldr < MONTADO)
      {
        Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      }
      break;
    }

    case TURN_LEFT_PASSIVE:
    {
      Sami->Left(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiPassive = TURN_RIGHT_SEMI_PASSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiPassive = ATTACK_SEMI_PASSIVE;
      if(ldr < MONTADO)
      {
        Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      }
      break;
    }

    case ATTACK_SEMI_PASSIVE:
    {
      if(ldr < MONTADO)
      {
        Sami->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_PASSIVE;
      }

      else 
      {
        Sami->Stop();
        if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiPassive = SEARCH_SEMI_PASSIVE;
        if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiPassive = TURN_RIGHT_SEMI_PASSIVE;
        if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiPassive = TURN_LEFT_SEMI_PASSIVE;
        if (millis() > currentTimeAttack + TICK_LOW_ATTACK)
        {
          semiPassive = LOW_ATTACK_SEMI_PASSIVE;
        }
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
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Strategy Semi_Agressive"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 28);
    oled.println("Press Star()"); 
    oled.display();
    if (buttonStart->GetIsPress())
    {
      oled.clearDisplay();  
      oled.setCursor(0, 28);
      oled.println("---------------------"); 
      oled.display();
      delay(5000);
      if(turnSide == LEFT)
      {
        Sami->Left(ATTACK_SPEED);
        delay(tickTurn);
      }
      if(turnSide == RIGHT)
      {
        Sami->Right(ATTACK_SPEED);
        delay(tickTurn);
      }
      passive = SEARCH_PASSIVE;
    } 
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
      if(ldr < MONTADO) 
      {
        Sami->Forward(ATTACK_SPEED_LDR, ATTACK_SPEED_LDR);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiPassive = TATAMI_LIMIT_SEMI_AGGRESSIVE;
      }
      if(distSharpRight > RIVAL_NEAR && distSharpLeft > RIVAL_NEAR)
      {
        Sami->Forward(STRONG_ATTACK_SPEED, STRONG_ATTACK_SPEED);
      }
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
//Maquina de estados para la estrategia Agressive
enum aggressive
{
  STANDBY_AGGRESSIVE,
  SEARCH_AGGRESSIVE,
  TURN_RIGHT_AGGRESSIVE,
  TURN_LEFT_AGGRESSIVE,
  TATAMI_LIMIT_AGGRESSIVE,
  ATTACK_AGGRESSIVE
}; 
int aggressive = STANDBY_AGGRESSIVE;
//Estrategia que va a buscar al rival de forma moderada
void Aggressive()
{
  switch (aggressive)
  {
    case STANDBY_AGGRESSIVE:
    {
    Sami->Stop();
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Strategy Agressive"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 28);
    oled.println("Press Star()"); 
    oled.display();
    if (buttonStart->GetIsPress())
    {
      oled.clearDisplay();  
      oled.setCursor(0, 28);
      oled.println("---------------------"); 
      oled.display();
      delay(5000);
      if(turnSide == LEFT)
      {
        Sami->Left(ATTACK_SPEED);
        delay(tickTurn);
      }
      if(turnSide == RIGHT)
      {
        Sami->Right(ATTACK_SPEED);
        delay(tickTurn);
      }
      passive = SEARCH_PASSIVE;
    } 
    break;
    }

    case SEARCH_AGGRESSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiAggressive = TURN_RIGHT_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiAggressive = TURN_LEFT_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_AGGRESSIVE; 
    }

    case TURN_RIGHT_AGGRESSIVE:
    {
      Sami->Right(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiAggressive = SEARCH_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft <= RIVAL) semiAggressive = TURN_LEFT_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_AGGRESSIVE;
      break;
    }

    TURN_LEFT_AGGRESSIVE:
    {
      Sami->Left(SEARCH_SPEED, SEARCH_SPEED);
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) semiAggressive = TATAMI_LIMIT_AGGRESSIVE;
      if(distSharpRight > RIVAL && distSharpLeft > RIVAL) semiAggressive = SEARCH_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft > RIVAL) semiAggressive = TURN_RIGHT_AGGRESSIVE;
      if(distSharpRight <= RIVAL && distSharpLeft <= RIVAL) semiAggressive = ATTACK_AGGRESSIVE;
      break;
    }


    case ATTACK_AGGRESSIVE:
    {
      Sami->Forward(ATTACK_SPEED_AGGRESSIVE, ATTACK_SPEED_AGGRESSIVE);
      if(distSharpRight > RIVAL || distSharpLeft > RIVAL) aggressive = SEARCH_AGGRESSIVE;
      if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) aggressive = TATAMI_LIMIT_AGGRESSIVE;
      if(ldr < MONTADO) 
      {
        Sami->Forward(ATTACK_SPEED_LDR, ATTACK_SPEED_LDR);
        if(leftTatamiRead < BORDE_TATAMI || righTatamiRead < BORDE_TATAMI) aggressive = TATAMI_LIMIT_AGGRESSIVE;
      }
      break;
    }

    case TATAMI_LIMIT_AGGRESSIVE: 
    {
    Sami->Backward(AVERAGE_SPEED, AVERAGE_SPEED);
    delay(300);
    if(leftTatamiRead > BORDE_TATAMI && righTatamiRead > BORDE_TATAMI) aggressive = SEARCH_AGGRESSIVE;
    break;
    }
  }
}


//<------------------------------------------------------------------------------------------------------------->//
//Menu de seleccion de reposicionamiento
void PositioningMenu()
{
  switch(positioningMenu)
  {
    case MAIN_MENU:
    {
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Avanzar o Retroceder"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("No moverse"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = SELECT_FORWARD;
      if(buttonStart->GetIsPress()) mainMenu = TURN_MENU;
      break;
    }

    case SELECT_FORWARD:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Avanzar o Retroceder"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Avance"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = SELECT_BACKWARD;
      if(buttonStart->GetIsPress())
      {
        forwardOrBackward = FORWARD;
        positioningMenu = ADVANCE_SHORT;        
      }
      break;
    }

    case SELECT_BACKWARD:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Avanzar o Retroceder"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Retroceso"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = MAIN_MENU;
      if(buttonStart->GetIsPress())
      {
        forwardOrBackward = BACKWARD;
        positioningMenu = REVERSE_SHORT;
      }
      break;
    }

    case ADVANCE_SHORT:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Selec largo de avance"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Avance corto"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_MEDIUM;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_SHORT;
        mainMenu = TURN_MENU;
      }
      break;
    }

    case ADVANCE_MEDIUM:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Selec largo de avance"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Avance medio"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_LONG;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_MEDIUM;
        mainMenu = TURN_MENU;
      }
      break;
    }

    case ADVANCE_LONG:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(4, 0);
      oled.println("Selec largo de avance"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Avance largo"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_SHORT;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_LONG;
        mainMenu = TURN_MENU;
      }
      break;
    }

  case REVERSE_SHORT:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(1, 0);
      oled.println("Selec largo de retroceso"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Retroceso corto"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_MEDIUM;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_SHORT;
        mainMenu = TURN_MENU;
      }
      break;
    }

    case REVERSE_MEDIUM:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(1, 0);
      oled.println("Selec largo de retroceso"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Retroceso medio"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_LONG;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_MEDIUM;
        mainMenu = TURN_MENU;
      }
      break;
    }

    case REVERSE_LONG:
    {
      oled.clearDisplay(); 
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(1, 0);
      oled.println("Selec largo de retroceso"); 
      oled.setCursor(0, 9);
      oled.println("---------------------"); 
      oled.setCursor(0, 26);
      oled.println("Retroceso largo"); 
      oled.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_SHORT;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_LONG;
        mainMenu = TURN_MENU;
      }
      break;
    }
  }
}

//<------------------------------------------------------------------------------------------------------------->//
//Menu de seleccion de giro
void TurnMenu()
{
  switch (turnMenu)
  {
  case MAIN_MENU_TURN:
  {
    oled.clearDisplay(); 
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(4, 0);
    oled.println("Selec. lado del giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("No girar"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = LEFT_TURN;
    if(buttonStart->GetIsPress()) mainMenu = STRATEGIES_MENU;
    break;
  }

  case LEFT_TURN:
  {
    oled.clearDisplay(); 
    oled.setCursor(4, 0);
    oled.println("Selec. lado del giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Girar a la izquierda"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = RIGHT_TURN;
    if(buttonStart->GetIsPress())
    {
      turnMenu = TURN_FRONT;
      turnSide = LEFT;
    }  
    break;
  }

  case RIGHT_TURN:
  {
    oled.clearDisplay(); 
    oled.setCursor(4, 0);
    oled.println("Selec. lado del giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Girar a la derecha"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = MAIN_MENU_TURN;
    if(buttonStart->GetIsPress())
    {
      turnMenu = TURN_FRONT;
      turnSide = RIGHT;
    }  
    break;
  }

  case TURN_FRONT:
  {
    oled.clearDisplay();
    oled.setCursor(1, 0);
    oled.println("Selec. angulo de giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Giro de 45 grados"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_SIDE;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_TURN_FRONT;
      mainMenu = STRATEGIES_MENU;
    }
    break;
  }
  case TURN_SIDE:
  {
    oled.clearDisplay();
    oled.setCursor(1, 0);
    oled.println("Selec. angulo de giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Giro de 90 grados"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = SHORT_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_TURN_SIDE;
      mainMenu = STRATEGIES_MENU;
    }
    break;
  }
  case SHORT_BACK_TURN:
  {
    oled.clearDisplay(); 
    oled.setCursor(1, 0);
    oled.println("Selec. angulo de giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Giro de 120 grados"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TICK_SHORT_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_SHORT_BACK_TURN;
      mainMenu = STRATEGIES_MENU;
    }
    break;
  }
  case LONG_BACK_TURN:
  {
    oled.clearDisplay();  
    oled.setCursor(1, 0);
    oled.println("Selec. angulo de giro"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 26);
    oled.println("Giro de 150 grados"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TICK_LONG_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_LONG_BACK_TURN;
      mainMenu = STRATEGIES_MENU;
    }
    break;
  }
  }
}

//<------------------------------------------------------------------------------------------------------------->//
//Menu de seleccion de estrategia

enum strategiesMenu
{
  STRATEGY_MAIN_MENU,
  PASSIVE_MENU,
  SEMI_PASSIVE_MENU,
  SEMI_AGGRESSIVE_MENU,
  AGGRESSIVE_MENU,
};
int menu = MAIN_MENU;

void StrategiesMenu()
{
  switch (mainMenu)
  {
  case STRATEGY_MAIN_MENU:
  {
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Select strategy"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 19);
    oled.println("Passive"); 
    oled.setCursor(0, 28);
    oled.println("Semi Passive"); 
    oled.setCursor(0, 37);
    oled.println("Semi Aggressive"); 
    oled.setCursor(0, 46);
    oled.println("Aggressive"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) menu = PASSIVE_MENU;
    break;
  }
  
  case PASSIVE_MENU:
  { 
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Select strategy"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 19);
    oled.println("Passive"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) menu = SEMI_PASSIVE_MENU;
    if(buttonStart->GetIsPress()) mainMenu = PASSIVE;
    break;
  }

  case SEMI_PASSIVE_MENU:
  {  
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Select strategy"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 28);
    oled.println("Semi-Passive"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) menu = SEMI_AGGRESSIVE_MENU;
    if(buttonStart->GetIsPress()) mainMenu = SEMI_PASSIVE;
    break;
  }

  case SEMI_AGGRESSIVE_MENU:
  { 
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Select strategy"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 37);
    oled.println("Semi-Aggressive"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) menu = AGGRESSIVE_MENU;
    if(buttonStart->GetIsPress()) mainMenu = SEMI_AGGRESSIVE;
    break;
  }

  case AGGRESSIVE_MENU:
  {
    oled.clearDisplay();  
    oled.setCursor(19, 0);
    oled.println("Select strategy"); 
    oled.setCursor(0, 9);
    oled.println("---------------------"); 
    oled.setCursor(0, 46);
    oled.println("Aggressive"); 
    oled.display();
    if(buttonStrategy->GetIsPress()) menu = PASSIVE_MENU;
    if(buttonStart->GetIsPress()) mainMenu = AGGRESSIVE;
    break;
  }
}
}
//<------------------------------------------------------------------------------------------------------------->//
//Maquina de casos principal del robot
void mainProgram()
{
  switch (mainMenu)
  {
  case POSITIONING_MENU:
  {
    PositioningMenu();
    break;
  }
  case TURN_MENU:
  {
    TurnMenu();
    break;
  }
  case STRATEGIES_MENU:
  {
    StrategiesMenu();
    break;
  }
  case PASSIVE:
  {
    Passive();
    break;
  }

  case SEMI_PASSIVE:
  {
    SemiPassive();
    break;
  }

  case SEMI_AGGRESSIVE:
  {
    SemiAggressive();
    break;
  }

  case AGGRESSIVE:
  {
    Aggressive();
    break;
  }
  }
}
//<------------------------------------------------------------------------------------------------------------->//

void setup() 
{
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop() 
{
  sensorsReading();
  mainProgram();
}