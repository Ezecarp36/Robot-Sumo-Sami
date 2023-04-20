#include <EngineController.h>
#include <SSD1306.h>
#include <Button.h>

#define PIN_RIGHT_ENGINE_DIR 19
#define PIN_RIGHT_ENGINE_PWM 18
#define PIN_LEFT_ENGINE_DIR 22
#define PIN_LEFT_ENGINE_PWM 21
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12
#define PIN_BUTTON_buttonStart 34
#define PIN_BUTTON_STRATEGY 35

int rightSpeed = 255;
int leftSpeed = 255;

#define TICK_TURN_FRONT 59
#define TICK_TURN_SIDE 95
#define TICK_SHORT_BACK_TURN 115
#define TICK_LONG_BACK_TURN 120
int tickTurn;

enum turnSide
{
  LEFT,
  RIGHT
};
int turnSide = LEFT;

#define PIN_SDA 16
#define PIN_SCL 17

SSD1306 display (0x3C,PIN_SDA, PIN_SCL);
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_buttonStart);
IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);

enum turnMenu
{
  MAIN_MENU,
  LEFT_TURN,
  RIGHT_TURN,
  NO_TURN,
  TURN_FRONT,
  TURN_SIDE,
  SHORT_BACK_TURN,
  LONG_BACK_TURN,
  TURN_TEST
};
int turnMenu = MAIN_MENU;

void TurnMenu()
{
  switch (turnMenu)
  {
  case MAIN_MENU:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion de lado del giro:"); 
    display.drawString(0, 9, "Presione Start para no girar"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_FRONT;
    if(buttonStart->GetIsPress())  turnMenu = NO_TURN;
    break;
  }

  case LEFT_TURN:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion de lado del giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "Giro hacia la izquierda"); 
    display.display();
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
    display.clear();   
    display.drawString(19, 0, "Seleccion de lado del giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "Giro hacia la derecha"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = LEFT_TURN;
    if(buttonStart->GetIsPress())
    {
      turnMenu = TURN_FRONT;
      turnSide = RIGHT;
    }  
    break;
  }

  case TURN_FRONT:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion del angulo de giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "el Rival esta adelante"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_SIDE;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_TURN_FRONT;
      turnMenu = TURN_TEST;
    }
    break;
  }
  case TURN_SIDE:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion del angulo de giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "El rival esta al lado"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = SHORT_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_TURN_SIDE;
      turnMenu = TURN_TEST;
    }
    break;
  }
  case SHORT_BACK_TURN:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion del angulo de giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "El rival esta atras pero cerca"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = LONG_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_SHORT_BACK_TURN;
      turnMenu = TURN_TEST;
    }
    break;
  }
  case LONG_BACK_TURN:
  {
    display.clear();   
    display.drawString(19, 0, "Seleccion del angulo de giro"); 
    display.drawString(0, 9, "---------------------"); 
    display.drawString(0,37, "El rival esta atras y lejos"); 
    display.display();
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_FRONT;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_LONG_BACK_TURN;
      turnMenu = TURN_TEST;
    }
    break;
  }

  case NO_TURN:
  {
    tickTurn = 0;
    break;
  }
  case TURN_TEST:
  {
    if(turnSide == LEFT)
    {
      Sami->Left(255);
      delay(tickTurn);
    }
    if(turnSide == RIGHT)
    {
      Sami->Right(255);
      delay(tickTurn);
    }
    turnMenu = MAIN_MENU;
    break;
  }
  }
}

void setup() 
{

}

void loop() 
{
  TurnMenu();
}
