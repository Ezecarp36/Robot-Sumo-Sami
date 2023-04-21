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

int Speed = 255;

int tickForwardOrBackward;
#define TICK_SHORT 10
#define TICK_MEDIUM 20
#define TICK_LONG 30

enum forwardOrBackward
{
  FORWARD,
  BACKWARD
};
int forwardOrBackward;

#define PIN_SDA 16
#define PIN_SCL 17

SSD1306 display (0x3C,PIN_SDA, PIN_SCL);
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_buttonStart);
IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);


enum positioningMenu
{
  MAIN_MENU,
  SELECT_FORWARD_OR_BACKWARD,
  SELECT_FORWARD,
  SELECT_BACKWARD,
  NO_FORWARD_OR_BACKWARD,
  ADVANCE_SHORT,
  ADVANCE_MEDIUM,
  ADVANCE_LONG,
  REVERSE_SHORT,
  REVERSE_MEDIUM,
  REVERSE_LONG,
  TEST
};

int positioningMenu = MAIN_MENU;

void PositioningMenu()
{
  switch(positioningMenu)
  {
    case MAIN_MENU:
    {
      display.clear();   
      display.drawString(4, 0, "Avanzar o Retroceder"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "No moverse"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = SELECT_FORWARD;
      if(buttonStart->GetIsPress())  positioningMenu = NO_FORWARD_OR_BACKWARD;
      break;
    }

    case NO_FORWARD_OR_BACKWARD:
    {
      display.clear();   
      display.drawString(0, 26, "NO AVANZO"); 
      display.display();
      delay(3000);
      break;
    }

    case SELECT_FORWARD:
    {
      display.clear();   
      display.drawString(4, 0, "Avanzar o Retroceder"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Avance"); 
      display.display();
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
      display.clear();   
      display.drawString(4, 0, "Avanzar o Retroceder"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Retroceso"); 
      display.display();
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
      display.clear();   
      display.drawString(31, 0, "Menu Avance"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Avance corto"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_MEDIUM;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_SHORT;
        positioningMenu = TEST;
      }
      break;
    }

    case ADVANCE_MEDIUM:
    {
      display.clear();   
      display.drawString(31, 0, "Menu Avance"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Avance medio"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_LONG;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_MEDIUM;
        positioningMenu = TEST;
      }
      break;
    }

    case ADVANCE_LONG:
    {
      display.clear();   
      display.drawString(31, 0, "Menu Avance"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Avance largo"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = ADVANCE_SHORT;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_LONG;
        positioningMenu = TEST;
      }
      break;
    }

  case REVERSE_SHORT:
    {
      display.clear();   
      display.drawString(22, 0, "Menu Retroceso"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Retroceso corto"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_MEDIUM;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_SHORT;
        positioningMenu = TEST;
      }
      break;
    }

    case REVERSE_MEDIUM:
    {
      display.clear();   
      display.drawString(22, 0, "Menu Retroceso"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Retroceso medio"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_LONG;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_MEDIUM;
        positioningMenu = TEST;
      }
      break;
    }

    case REVERSE_LONG:
    {
      display.clear();   
      display.drawString(22, 0, "Menu Retroceso"); 
      display.drawString(0, 9, "---------------------"); 
      display.drawString(0,26, "Retroceso largo"); 
      display.display();
      if(buttonStrategy->GetIsPress()) positioningMenu = REVERSE_SHORT;
      if(buttonStart->GetIsPress())
      {
        tickForwardOrBackward = TICK_LONG;
        positioningMenu = TEST;
      }
      break;
    }

  case TEST:
  {
    if(forwardOrBackward == FORWARD)
    {
      Sami->Forward(Speed);
      delay(tickForwardOrBackward);
    }

    if(forwardOrBackward == BACKWARD)
    {
      Sami->Backward(Speed);
      delay(tickForwardOrBackward);
    }
    positioningMenu = MAIN_MENU;
    break;
  }    
  }
}

void setup() 
{
  display.init();
}

void loop() 
{
  PositioningMenu();
}
