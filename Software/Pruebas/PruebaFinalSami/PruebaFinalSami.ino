#include <EngineController.h>
#include <Button.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PIN_RIGHT_ENGINE_DIR 16
#define PIN_RIGHT_ENGINE_PWM 17
#define PIN_LEFT_ENGINE_DIR 18
#define PIN_LEFT_ENGINE_PWM 19
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
  NONE,
  FORWARD,
  BACKWARD
};
int forwardOrBackward = NONE;

#define TICK_TURN_FRONT 59
#define TICK_TURN_SIDE 95
#define TICK_SHORT_BACK_TURN 115
#define TICK_LONG_BACK_TURN 120
int tickTurn;

enum turnSide
{
  NONE_TURN,
  LEFT,
  RIGHT
};
int turnSide = NONE_TURN;

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_buttonStart);
IEngine *rightEngine = new Driver_G2_18V17(PIN_RIGHT_ENGINE_DIR, PIN_RIGHT_ENGINE_PWM, PWM_CHANNEL_ENGINE_RIGHT);
IEngine *leftEngine = new Driver_G2_18V17(PIN_LEFT_ENGINE_DIR, PIN_LEFT_ENGINE_PWM, PWM_CHANNEL_ENGINE_LEFT);
EngineController *Sami = new EngineController(rightEngine, leftEngine);


//declaro variables para el menu de test
enum test
{
  POSITIONING_MENU,
  TURN_MENU,
  TEST
};

int test = POSITIONING_MENU;

//Declaro variables para el menu de avance y retroceso
enum positioningMenu
{
  MAIN_MENU,
  SELECT_FORWARD_OR_BACKWARD,
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

//Declaro variables para el menu de giro
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------//
//Menu de avance o retroceso//

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
      if(buttonStart->GetIsPress())
      {
        oled.clearDisplay();  
        oled.setTextSize(1);
        oled.setTextColor(WHITE);
        oled.setCursor(0, 26);
        oled.println("Me quedo quieto"); 
        oled.display();
        delay(2000);
        test = TURN_MENU;
      }
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
        test = TURN_MENU;
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
        test = TURN_MENU;
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
        test = TURN_MENU;
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
        test = TURN_MENU;
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
        test = TURN_MENU;
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
        test = TURN_MENU;
      }
      break;
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------//
//Menu de giro//

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
    if(buttonStart->GetIsPress())
    {
      oled.clearDisplay(); 
      oled.setCursor(1, 26);
      oled.println("NO GIRO...");
      oled.display();
      delay(2000);
      test = TEST;
    }
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
      test = TEST;
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
      test = TEST;
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
    if(buttonStrategy->GetIsPress()) turnMenu = LONG_BACK_TURN;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_SHORT_BACK_TURN;
      test = TEST;
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
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_FRONT;
    if(buttonStart->GetIsPress())
    {
      tickTurn = TICK_LONG_BACK_TURN;
      test = TEST;
    }
    break;
  }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------//
//Menu//

void Test()
{
  switch(test)
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

      if(turnSide == LEFT)
      {
        Sami->Left(Speed);
        delay(tickTurn);
      }
      if(turnSide == RIGHT)
      {
        Sami->Right(Speed);
        delay(tickTurn);
      }

      Sami->Stop();
      turnMenu = MAIN_MENU_TURN;
      positioningMenu = MAIN_MENU;
      test = POSITIONING_MENU;
      tickTurn = 0;
      tickForwardOrBackward = 0;
      break;
    }
  }
}

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop() 
{
  Test();
}
