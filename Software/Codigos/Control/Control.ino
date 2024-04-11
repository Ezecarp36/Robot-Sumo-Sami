#include <Button.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

//Oled
#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Botones
#define PIN_BUTTON_BUTTON_START 34
#define PIN_BUTTON_STRATEGY 35
Button *buttonStrategy = new Button(PIN_BUTTON_STRATEGY);
Button *buttonStart = new Button(PIN_BUTTON_BUTTON_START);

//Seleccion de lado de giro
enum turnSide
{
  NONE_TURN,
  LEFT,
  RIGHT
};
int turnSide = NONE_TURN;

//Ticks para los giros
int tickTurn;
#define TICK_TURN_FRONT 59
#define TICK_TURN_SIDE 95
#define TICK_SHORT_BACK_TURN 115
#define TICK_LONG_BACK_TURN 120

//strategias
enum Strategy
{
  PASSIVE,
  SEMI_PASSIVE,
  SEMI_AGGRESSIVE,
  AGGRESSIVE,
};
int Strategy = PASSIVE;

//Declaración de variable para el Programa Principal
enum mainMenu
{
  TURN_MENU,
  STRATEGIES_MENU,
  SEND_DATA
};

int mainMenu = TURN_MENU;

//<------------------------------------------------------------------------------------------------------------->//
//Menu de seleccion de giro

enum turnMenu
{
  MAIN_MENU_TURN,
  LEFT_TURN,
  RIGHT_TURN,
  TURN_FRONT,
  TURN_SIDE,
  SHORT_BACK_TURN,
  LONG_BACK_TURN
};
int turnMenu = MAIN_MENU_TURN;

void TurnMenu()
{
  switch (turnMenu)
  {
  case MAIN_MENU_TURN:
  {
    oled.clearDisplay(); 
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
    if(buttonStrategy->GetIsPress()) turnMenu = LONG_BACK_TURN;
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
    if(buttonStrategy->GetIsPress()) turnMenu = TURN_FRONT;
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
int strategiesMenu = STRATEGY_MAIN_MENU;

void StrategiesMenu()
{
  switch (strategiesMenu)
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
    if(buttonStrategy->GetIsPress()) strategiesMenu = PASSIVE_MENU;
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
    if(buttonStrategy->GetIsPress()) strategiesMenu = SEMI_PASSIVE_MENU;
    if(buttonStart->GetIsPress())
    {
      Strategy = PASSIVE;
      mainMenu = SEND_DATA;
    }
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
    if(buttonStrategy->GetIsPress()) strategiesMenu = SEMI_AGGRESSIVE_MENU;
    if(buttonStart->GetIsPress())
    {
      Strategy = SEMI_PASSIVE;
      mainMenu = SEND_DATA;
    }
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
    if(buttonStrategy->GetIsPress()) strategiesMenu = AGGRESSIVE_MENU;
    if(buttonStart->GetIsPress())
    {
      Strategy = SEMI_AGGRESSIVE;
      mainMenu = SEND_DATA;
    }
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
    if(buttonStrategy->GetIsPress()) strategiesMenu = PASSIVE_MENU;
    if(buttonStart->GetIsPress())
    {
      Strategy = AGGRESSIVE;
      mainMenu = SEND_DATA;
    }
    break;
  }
}
}
//<------------------------------------------------------------------------------------------------------------->//
//Pograma Principal que ejecuta los menus de selección y luego envia los datos

void mainProgram()
{
  switch (mainMenu)
  {
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

    case SEND_DATA:
    {
      break;
    }

  }
}

void setup() 
{
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  SerialBT.begin("sami");
}

void loop() 
{
}
