#include <EEPROM.h>
#include <LiquidCrystal.h>

// LCD Pins, initializing screen
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Button Pins
const int buttonP3 = 8; //Pause game or move to next setting
const int buttonP2 = 9; //Switch to player 2 or increase setting
const int buttonP1 = 10; //Switch to player 1 or decrease setting
const int buzzer = 7;

int setupPlayer = 0;  // Controls setup stages from 0 to 2
int setupNumber = 0;  // Controls from 0 to 1
int currentPlayer = 0; // 0 for Player 1 (White), 1 for Player 2 (Black)
bool gameRunning = false, gamePaused = false, whiteWon = false, blackWon = false;
int player1Minutes = 0, player1Seconds = 0, player2Minutes = 0, player2Seconds = 0;
int increment = 0;
int centiCounter1 = 0, centiCounter2 = 0;

bool buttonP1pressed = false, buttonP2pressed = false, buttonP3pressed = false;

unsigned long lastTimeUpdate;

void setup() {
  pinMode(buttonP3, INPUT);
  pinMode(buttonP2, INPUT);
  pinMode(buttonP1, INPUT);
  pinMode(buzzer, OUTPUT);
  lcd.begin(16, 2);
  promptForSettings();
  if (EEPROM.read(0) != 255)
  {
    player1Minutes = EEPROM.read(0);
  }
  if (EEPROM.read(1) != 255)
  {
    player1Seconds = EEPROM.read(1);
  }
  if (EEPROM.read(2) != 255)
  {
    player2Minutes = EEPROM.read(2);
  }
  if (EEPROM.read(3) != 255)
  {
    player2Seconds = EEPROM.read(3);
  }
  if (EEPROM.read(4) != 255)
  {
    increment = EEPROM.read(4);
  }
}

void loop() {
  if (gameRunning) {
    if (digitalRead(buttonP1) == HIGH) {
      if (currentPlayer != 1)
      {
      	player1Seconds += increment;
      	while (player1Seconds >= 60)
      	{
        	player1Minutes++;
        	player1Seconds -= 60;
      	}
      }
      currentPlayer = 1;
    }
    else if (digitalRead(buttonP2) == HIGH) {
      if (currentPlayer != 0)
      {
      	player2Seconds += increment;
      	while (player2Seconds >= 60)
      	{
        	player2Minutes++;
        	player2Seconds -= 60;
      	}
      }
      currentPlayer = 0;
    }
    else if (digitalRead(buttonP3) == HIGH) {
      buttonP3pressed = true;
    }
    else {
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }
    advanceTime();
    if (whiteWon == true) {
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Black timeout");
    }
    else if (blackWon == true) {
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("White timeout");
      whiteWon = true;
    }
    delay(100); // Debounce delay
    while (gamePaused) {
      if (whiteWon == true)
      {
        while (whiteWon == true)
        {
          noTone(buzzer);
          delay(500);
          tone(buzzer, 523);
          delay(500);
        }
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Game Paused");
      delay(500); // Debounce delay
      if ((digitalRead(buttonP3) == HIGH) && whiteWon == false) {
        gamePaused = false;
        buttonP3pressed = false;
        lcd.clear();
        delay(200); // Debounce delay
      }
    }
  }
  else {
    checkSetupButtons();
    promptForSettings();
    if (digitalRead(buttonP1) == HIGH) {
      buttonP1pressed = true;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }
    else if (digitalRead(buttonP2) == HIGH) {
      buttonP1pressed = false;
      buttonP2pressed = true;
      buttonP3pressed = false;
    }
    else if (digitalRead(buttonP3) == HIGH) {
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = true;
    }
    else {
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }
    delay(200); // Debounce delay
  }
}

void checkSetupButtons() {
  if (buttonP3pressed) {
    lcd.clear();
    if (setupNumber < 1) {
      setupNumber++;
    }
    else if (setupPlayer < 1) {
      setupNumber = 0;
      setupPlayer++;
    }
    else if (setupPlayer == 1) {
      if (player2Minutes == 0 && player2Seconds == 0)
      {
        player2Minutes = player1Minutes;
        player2Seconds = player1Seconds;
      }
      setupPlayer++;
      setupNumber = 2;
    }
    else {
      gameRunning = true;
      EEPROM.write(0, player1Minutes);
  	  EEPROM.write(1, player1Seconds);
  	  EEPROM.write(2, player2Minutes);
  	  EEPROM.write(3, player2Seconds);
  	  EEPROM.write(4, increment);
    }
    delay(500);
  }
  if (buttonP2pressed) {
    lcd.clear();
    if (setupPlayer == 0) {
      if (setupNumber == 0) {
        player1Minutes++;
      } else if (setupNumber == 1) {
        player1Seconds++;
        if (player1Seconds >= 60)
        {
          player1Seconds = 59;
        }
      }
    }
    else if (setupPlayer == 1) {
      if (setupNumber == 0) {
        player2Minutes++;
      } else if (setupNumber == 1) {
        player2Seconds++;
        if (player2Seconds >= 60)
        {
          player2Seconds = 59;
        }
      }
    }
    else if (setupNumber == 2) {
      increment++;
    }
  }
  if (buttonP1pressed) {
    lcd.clear();
    if (setupPlayer == 0) {
      if (setupNumber == 0) {
        player1Minutes--;
        if (player1Minutes < 0) {
          player1Minutes = 0;
        }
      } else if (setupNumber == 1) {
        player1Seconds--;
        if (player1Seconds < 0) {
          player1Seconds = 0;
        }
      }
    }
    else if (setupPlayer == 1) {
      if (setupNumber == 0) {
        player2Minutes--;
        if (player2Minutes < 0) {
          player2Minutes = 0;
        }
      } else if (setupNumber == 1) {
        player2Seconds--;
        if (player2Seconds < 0) {
          player2Seconds = 0;
        }
      }
    }
    else if (setupNumber == 2) {
      increment--;
      if (increment < 0) {
        increment = 0;
      }
    }
  }
}

void promptForSettings() {
  const char* labels[] = {"Minutes: ", "Seconds: ", "Increment: "};
  const char* player[] = {"Player 1: ", "Player 2: ", "Both: "};
  lcd.setCursor(0,0);
  lcd.print(player[setupPlayer]);
  lcd.setCursor(0,1);
  lcd.print(labels[setupNumber]);
  if (setupPlayer == 0) {
    if (setupNumber == 0)
    {
      lcd.print(player1Minutes);
    }
    else {
      lcd.print(player1Seconds);
    }
  }
  else if (setupPlayer == 1) {
    if (setupNumber == 0)
    {
      lcd.print(player2Minutes);
    }
    else {
      lcd.print(player2Seconds);
    }
  }
  else
  {
    lcd.print(increment);
  }
}

void advanceTime() {
  unsigned long currentTime = millis();
  if (buttonP3pressed) {
    gamePaused = true;
    return;
  }
  if (currentPlayer == 0) {
    if (centiCounter1 < 10) {
      centiCounter1++;
      return;
    }
    else if (player1Seconds > 0) {
      centiCounter1 = 0;
      player1Seconds--;
    }
    else if (player1Minutes > 0) {
      centiCounter1 = 0;
      player1Minutes--;
      player1Seconds = 59;
    }
    else {
      blackWon = true;
    }
  }
  else if (currentPlayer == 1) {
    if (centiCounter2 < 10) {
      centiCounter2++;
      return;
    }
    else if (player2Seconds > 0) {
      centiCounter2 = 0;
      player2Seconds--;
    }
    else if (player2Minutes > 0) {
      centiCounter2 = 0;
      player2Minutes--;
      player2Seconds = 59;
    }
    else {
      whiteWon = true;
    }
  }
  displayCurrentTime();
}

void displayCurrentTime() {
  if (player1Seconds == 59 || player2Seconds == 59)
  {
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print("W:");
  lcd.setCursor(0, 1);
  if (player1Minutes < 10) {
    lcd.print("0");
  }
  lcd.print(player1Minutes);
  lcd.print(":");
  if (player1Seconds < 10) {
    lcd.print("0");
  }
  lcd.print(player1Seconds);

  lcd.setCursor(9, 0); // Display on the right side of the same row
  lcd.print("B:");
  lcd.setCursor(9, 1);
  if (player2Minutes < 10) {
    lcd.print("0");
  }
  lcd.print(player2Minutes);
  lcd.print(":");
  if (player2Seconds < 10) {
    lcd.print("0");
  }
  lcd.print(player2Seconds);
}
