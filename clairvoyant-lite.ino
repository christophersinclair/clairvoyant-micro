/**************************************************************************
Written by Chris Sinclair

# Clairvoyant
Clairvoyant is a seemingly simple game of memory.

### Game Rules
See README.md for how the game works.

### Hardware
1 x Arduino Uno
1 x SSD13306 OLED display connected to the A4 and A5 analog pins on the Arduino
5 x LEDs of colors red, blue, and yellow
5 x Buttons with colored caps of red, blue, yellow, green, and black

### Software
Adafruit_BusIO
Adafruit_GFX_Library
Adafruit_SSD1306
**************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int x, minX;

// Set GPIO pin locations for LEDs
const int RED_LED = 0;
const int BLUE_LED = 1;
const int YELLOW_LED = 2;

// Set GPIO pin locations for buttons
const int RED_BTN = 3;
const int BLUE_BTN = 4;
const int YELLOW_BTN = 5;

const int ENTER_GREEN = 6;
const int MULTI_BLACK = 7;

// Clairvoyant Lite is set to 3 players only
const int NUM_PLAYERS = 3;

struct guess {
  int turnItIs;
  int turnItWillBe;
  int turnsToGo;
  const char *target;
  const char *guesser;
};

// Only 100 guesses allowed until game runs out
guess guesses[100];

void(* resetFunc) (void) = 0;

void setup() {
  // LEDs are output
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  // Buttons are input
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(YELLOW_BTN, INPUT_PULLUP);

  pinMode(ENTER_GREEN, INPUT_PULLUP);
  pinMode(MULTI_BLACK, INPUT_PULLUP);

  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
}

int numDigits(int num) {
    int digits = 0;
    if (num == 0) {
        return 1;
    }
    while (num != 0) {
        num /= 10;
        digits++;
    }
    return digits;
}

void displayMessage(char *message) {
  x = display.width();
  minX = -12 * strlen(message);
  for(;;) {
   display.clearDisplay();
   display.setCursor(0,0);
   display.setCursor(x,10);
   display.print(message);
   display.display();
   x=x-5; // scroll speed, make more positive to slow down the scroll
   if(x < minX) x= display.width();
  }
}

void displayInt(int value) {
  char message[20];
  itoa(value, message, 10);

  displayMessage(message);
}

void loop() {
  // Display welcome message
  char welcomeMessage[] = "Welcome to Clairvoyant Lite!";
  displayMessage(welcomeMessage);

  // Display select number of players
  char liteMessage[] = "This version of the game is set to three players only (Blue, Red, and Yellow).";
  displayMessage(liteMessage);
  delay(5000);

  char colorChoiceMessage[] = "Please choose which color player you will be, then press the Green button to get started!";
  displayMessage(colorChoiceMessage);
  for(;;) {
    if (digitalRead(ENTER_GREEN) == 0) {
      digitalWrite(ENTER_GREEN, HIGH);
      break;
    }
  }

  // Randomly choose who goes first
  long randNumber = random(NUM_PLAYERS);
  char *currentPlayer;
  if (randNumber == 0) {
    char blueFirstMessage[] = "Blue player goes first! Please hand the controller to them, then have them press the Blue button.";
    displayMessage(blueFirstMessage);
    for(;;) {
      if (digitalRead(BLUE_BTN) == 0) {
        digitalWrite(BLUE_LED, HIGH);
        currentPlayer = "BLUE";
        digitalWrite(BLUE_BTN, HIGH);

        delay(1000);
        digitalWrite(BLUE_LED, LOW);
        break;
      }
    }
  } else if (randNumber == 1) {
    char redFirstMessage[] = "Red player goes first! Please hand the controller to them, then have them press the Red button.";
    displayMessage(redFirstMessage);
    for(;;) {
      if (digitalRead(RED_BTN) == 0) {
        digitalWrite(RED_LED, HIGH);
        currentPlayer = "RED";
        digitalWrite(RED_BTN, HIGH);

        delay(1000);
        digitalWrite(RED_LED, LOW);
        break;
      }
    }
  } else {
    char yellowFirstMessage[] = "Yellow player goes first! Please hand the controller to them, then have them press the Yellow button.";
    displayMessage(yellowFirstMessage);
    for(;;) {
      if (digitalRead(YELLOW_BTN) == 0) {
        digitalWrite(YELLOW_LED, HIGH);
        currentPlayer = "YELLOW";
        digitalWrite(YELLOW_BTN, HIGH);

        delay(1000);
        digitalWrite(YELLOW_LED, LOW);
        break;
      }
    }
  }

  int TURN = 0;
  char playerIdentificationMessage[] = "Which player are YOU? Press your colored button.";
  char nextPlayerMessage[] = "Now pick whoever should guess next, and hand the controller to them. Once ready for the next turn, press the Green button.";

  for(;;) {
    if (TURN == 99) {
      char tieGameMessage[] = "No more guesses! Its a tie!";
      displayMessage(tieGameMessage);
      delay(10000);

      // Reboot device
      resetFunc();
    }
    if (TURN > 0) {
      displayMessage(playerIdentificationMessage);
      delay(3000);
      if (digitalRead(RED_BTN) == 0) {
        digitalWrite(RED_LED, HIGH);
        currentPlayer = "RED";
        digitalWrite(RED_BTN, HIGH);

        delay(1000);
        digitalWrite(RED_LED, LOW);
      } else if (digitalRead(BLUE_BTN) == 0) {
        digitalWrite(BLUE_LED, HIGH);
        currentPlayer = "BLUE";
        digitalWrite(BLUE_BTN, HIGH);

        delay(1000);
        digitalWrite(BLUE_LED, LOW);
      } else if (digitalRead(YELLOW_BTN) == 0) {
        digitalWrite(YELLOW_LED, HIGH);
        currentPlayer = "YELLOW";
        digitalWrite(YELLOW_BTN, HIGH);

        delay(1000);
        digitalWrite(YELLOW_LED, LOW);
      }

      for (int i = 0; i < TURN; i++) {
        guess candidate = guesses[i];
        if (candidate.turnItWillBe == TURN && candidate.target == currentPlayer) {
          int messageLength = strlen("CONGRATULATIONS TO PLAYER ") + strlen(candidate.guesser) +
                        strlen("! They guessed that player ") + strlen(candidate.target) +
                        strlen(" would have the controller in ") + 2 * numDigits(candidate.turnsToGo) +
                        strlen(", ") + 2 * numDigits(candidate.turnsToGo) + strlen(" turns ago!");

          char *congratsMessage = new char[messageLength + 1];
          sprintf(congratsMessage, "CONGRATULATIONS TO PLAYER %s! They guessed that player %s would have the controller in %d, %d turns ago!",
            candidate.guesser, candidate.target, candidate.turnsToGo, candidate.turnsToGo);
          
          displayMessage(congratsMessage);
          delay(30000);

          delete[] congratsMessage;

          // Reboot device
          resetFunc();
        }
      }
    }

    gameTurn(currentPlayer, TURN);

    displayMessage(nextPlayerMessage);
    for(;;) {
      if (digitalRead(ENTER_GREEN) == 0) {
        digitalWrite(ENTER_GREEN, HIGH);
        break;
      }
    }

    TURN++;
  }
}

void gameTurn(char *currentPlayer, int turn) {
  int turnsToGo = 2;
  char *target;
  char turnGuesserMessage[] = "Hello, player! How far into the future can you see? (Use the Black button to increment the number of turns you want to guess for, then the Green button to lock in)";
  char playerGuesserMessage[] = "When you look to the future, who do you see in your mind? (Press the button corresponding to your guessed target)";

  displayMessage(turnGuesserMessage);
  delay(5000);

  for (;;) {
    if (digitalRead(MULTI_BLACK) == 0) {
      // Can only guess up to 10 turns ahead until it cycles back to 2
      if (turnsToGo < 11) {
        turnsToGo++;
        digitalWrite(MULTI_BLACK, HIGH);
      } else {
        turnsToGo = 2;
        digitalWrite(MULTI_BLACK, HIGH);
      }
      displayInt(turnsToGo);
    }

    if (digitalRead(ENTER_GREEN) == 0) {
      // Lock in the choice
      digitalWrite(ENTER_GREEN, HIGH);
      break;
    }
  }

  displayMessage(playerGuesserMessage);
  delay(5000);

  for(;;) {
    if (digitalRead(RED_BTN) == 0) {
      digitalWrite(RED_LED, HIGH);
      target = "RED";
      digitalWrite(RED_BTN, HIGH);

      delay(1000);
      digitalWrite(RED_LED, LOW);
      break;
    } else if (digitalRead(BLUE_BTN) == 0) {
      digitalWrite(BLUE_LED, HIGH);
      target = "BLUE";
      digitalWrite(BLUE_BTN, HIGH);

      delay(1000);
      digitalWrite(BLUE_LED, LOW);
      break;
    } else if (digitalRead(YELLOW_BTN) == 0) {
      digitalWrite(YELLOW_LED, HIGH);
      target = "YELLOW";
      digitalWrite(YELLOW_BTN, HIGH);

      delay(1000);
      digitalWrite(YELLOW_LED, LOW);
      break;
    }
  }

  guess g;
  g.guesser = currentPlayer;
  g.target = target;
  g.turnsToGo = turnsToGo;
  g.turnItIs = turn;
  g.turnItWillBe = turn + turnsToGo;

  guesses[turn] = g;
}
