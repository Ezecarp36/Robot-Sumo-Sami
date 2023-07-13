#include <EngineController.h>
#include <Button.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PIN_RIGHT_ENGINE_DIR 18
#define PIN_RIGHT_ENGINE_PWM 19
#define PIN_LEFT_ENGINE_DIR 16
#define PIN_LEFT_ENGINE_PWM 17
#define PWM_CHANNEL_ENGINE_RIGHT 11
#define PWM_CHANNEL_ENGINE_LEFT 12
#define PIN_BUTTON_buttonStart 34
#define PIN_BUTTON_STRATEGY 35

int Speed = 255;

int tickForwardOrBackward;
#define TICK_SHORT 100
#define TICK_MEDIUM 150
#define TICK_LONG 200

enum forwardOrBackward
{
  FORWARD,
  BACKWARD
};
int forwardOrBackward;

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
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
      if(buttonStart->GetIsPress())  positioningMenu = NO_FORWARD_OR_BACKWARD;
      break;
    }

    case NO_FORWARD_OR_BACKWARD:
    {
      oled.clearDisplay();  
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(0, 26);
      oled.println("Me quedo quieto"); 
      oled.display();
      delay(2000);
      positioningMenu = MAIN_MENU;
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
        positioningMenu = TEST;
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
        positioningMenu = TEST;
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
        positioningMenu = TEST;
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
        positioningMenu = TEST;
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
        positioningMenu = TEST;
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
        positioningMenu = TEST;
      }
      break;
    }

  case TEST:
  {
    delay(3000);
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
    Sami->Stop();
    tickForwardOrBackward = 0;
    positioningMenu = MAIN_MENU;
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
  PositioningMenu();
}
