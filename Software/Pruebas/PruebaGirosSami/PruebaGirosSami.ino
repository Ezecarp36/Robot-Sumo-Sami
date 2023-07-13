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

int speed = 255;

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

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
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
      delay(3000);
      turnMenu = MAIN_MENU;
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
    if(buttonStrategy->GetIsPress()) turnMenu = MAIN_MENU;
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
      turnMenu = TURN_TEST;
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
      turnMenu = TURN_TEST;
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
      turnMenu = TURN_TEST;
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
      turnMenu = TURN_TEST;
    }
    break;
  }

  case TURN_TEST:
  {
    delay(3000);
    if(turnSide == LEFT)
    {
      Sami->Left(speed);
      delay(tickTurn);
    }
    if(turnSide == RIGHT)
    {
      Sami->Right(speed);
      delay(tickTurn);
    }
    Sami->Stop();
    tickTurn = 0;
    turnMenu = MAIN_MENU;
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
  TurnMenu();
}
