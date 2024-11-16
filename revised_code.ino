#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// LCD Pins, initializing screen
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Initializing LED displays
Adafruit_7segment led_display1 = Adafruit_7segment();
Adafruit_7segment led_display2 = Adafruit_7segment();

// Button Pins
const int buttonP3 = 8; // Pause game or move to next setting
const int buttonP2 = 9; // Switch to player 2 or increase setting
const int buttonP1 = 10; // Switch to player 1 or decrease setting
const int buzzer = 7; // Buzzer pin

// Game variables
int setupPlayer = 0;  // Controls setup stages from 0 to 2
int setupNumber = 0;  // Controls from 0 to 1
int currentPlayer = 0; // 0 for Player 1 (White), 1 for Player 2 (Black)
bool gameRunning = false, gamePaused = false, whiteWon = false, blackWon = false;
int player1Minutes = 0, player1Seconds = 0, player2Minutes = 0, player2Seconds = 0;
char player1Time[5] = "0000", player2Time[5] = "0000";
int increment = 0;
int centiCounter1 = 0, centiCounter2 = 0;

// Button states
bool buttonP1pressed = false, buttonP2pressed = false, buttonP3pressed = false;

// Last time update
unsigned long lastTimeUpdate;

void setup() {
  // Setup pin modes
  pinMode(buttonP3, INPUT);
  pinMode(buttonP2, INPUT);
  pinMode(buttonP1, INPUT);
  pinMode(buzzer, OUTPUT);

  // Initialize LED displays
  led_display1.begin(112);
  led_display2.begin(113);
  led_display1.writeDisplay();
  led_display2.writeDisplay();

  // Initialize LCD
  lcd.begin(16, 2);

  // Prompt for settings
  promptForSettings();

  // Read saved settings from EEPROM
  if (EEPROM.read(0) != 255) player1Minutes = EEPROM.read(0);
  if (EEPROM.read(1) != 255) player1Seconds = EEPROM.read(1);
  if (EEPROM.read(2) != 255) player2Minutes = EEPROM.read(2);
  if (EEPROM.read(3) != 255) player2Seconds = EEPROM.read(3);
  if (EEPROM.read(4) != 255) increment = EEPROM.read(4);
}

void loop() {
  if (gameRunning) {
    // Handle button presses for switching players
    if (digitalRead(buttonP1) == HIGH && currentPlayer != 1) {
      player1Seconds += increment;
      while (player1Seconds >= 60) {
        player1Minutes++;
        player1Seconds -= 60;
      }
      currentPlayer = 1;
    } else if (digitalRead(buttonP2) == HIGH && currentPlayer != 0) {
      player2Seconds += increment;
      while (player2Seconds >= 60) {
        player2Minutes++;
        player2Seconds -= 60;
      }
      currentPlayer = 0;
    } else if (digitalRead(buttonP3) == HIGH) {
      buttonP3pressed = true;
    } else {
      buttonP1pressed = buttonP2pressed = buttonP3pressed = false;
    }

    // Advance time and check for win conditions
    advanceTime();
    if (whiteWon) {
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Black timeout");
    } else if (blackWon) {
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("White timeout");
      whiteWon = true;
    }

    delay(100); // Debounce delay

    // Handle game pause
    while (gamePaused) {
      if (whiteWon) {
        while (whiteWon) {
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
      if (digitalRead(buttonP3) == HIGH && !whiteWon) {
        gamePaused = false;
        buttonP3pressed = false;
        lcd.clear();
        delay(200); // Debounce delay
      }
    }
  } else {
    // Handle button presses during setup
    checkSetupButtons();
    promptForSettings();
    if (digitalRead(buttonP1) == HIGH) {
      buttonP1pressed = true;
      buttonP2pressed = buttonP3pressed = false;
    } else if (digitalRead(buttonP2) == HIGH) {
      buttonP1pressed = false;
      buttonP2pressed = true;
      buttonP3pressed = false;
    } else if (digitalRead(buttonP3) == HIGH) {
      buttonP1pressed = buttonP2pressed = false;
      buttonP3pressed = true;
    } else {
      buttonP1pressed = buttonP2pressed = buttonP3pressed = false;
    }
    delay(200); // Debounce delay
  }
}

void checkSetupButtons() {
  if (buttonP3pressed) {
    lcd.clear();
    if (setupNumber < 1) {
      setupNumber++;
    } else if (setupPlayer < 1) {
      setupNumber = 0;
      setupPlayer++;
    } else if (setupPlayer == 1) {
      if (player2Minutes == 0 && player2Seconds == 0) {
        player2Minutes = player1Minutes;
        player2Seconds = player1Seconds;
      }
      setupPlayer++;
      setupNumber = 2;
    } else {
      gameRunning = true;
      // Save settings to EEPROM
      if (player1Minutes != EEPROM.read(0)) EEPROM.write(0, player1Minutes);
      if (player1Seconds != EEPROM.read(1)) EEPROM.write(1, player1Seconds);
      if (player2Minutes != EEPROM.read(2)) EEPROM.write(2, player2Minutes);
      if (player2Seconds != EEPROM.read(3)) EEPROM.write(3, player2Seconds);
      if (increment != EEPROM.read(4)) EEPROM.write(4, increment);
      lcd.clear();
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
        if (player1Seconds >= 60) player1Seconds = 59;
      }
    } else if (setupPlayer == 1) {
      if (setupNumber == 0) {
        player2Minutes++;
      } else if (setupNumber == 1) {
        player2Seconds++;
        if (player2Seconds >= 60) player2Seconds = 59;
      }
    } else if (setupNumber == 2) {
      increment++;
    }
  }
  if (buttonP1pressed) {
    lcd.clear();
    if (setupPlayer == 0) {
      if (setupNumber == 0) {
        player1Minutes--;
        if (player1Minutes < 0) player1Minutes = 0;
      } else if (setupNumber == 1) {
        player1Seconds--;
        if (player1Seconds < 0) player1Seconds = 0;
      }
    } else if (setupPlayer == 1) {
      if (setupNumber == 0) {
        player2Minutes--;
        if (player2Minutes < 0) player2Minutes = 0;
      } else if (setupNumber == 1) {
        player2Seconds--;
        if (player2Seconds < 0) player2Seconds = 0;
      }
    } else if (setupNumber == 2) {
      increment--;
      if (increment < 0) increment = 0;
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
    led_display1.clear();
    setTime(player1Minutes, true, true);
    setTime(player1Seconds, true, false);
    led_display1.println(player1Time);
    led_display1.drawColon(true);
    led_display1.writeDisplay();
  } else if (setupPlayer == 1) {
    led_display2.clear();
    setTime(player2Minutes, false, true);
    setTime(player2Seconds, false, false);
    led_display2.println(player2Time);
    led_display2.drawColon(true);
    led_display2.writeDisplay();
  } else {
    led_display1.clear();
    led_display2.clear();
    led_display1.println(increment);
    led_display1.drawColon(false);
    led_display2.drawColon(false);
    led_display1.writeDisplay();
    led_display2.writeDisplay();
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
    } else if (player1Seconds > 0) {
      centiCounter1 = 0;
      player1Seconds--;
    } else if (player1Minutes > 0) {
      centiCounter1 = 0;
      player1Minutes--;
      player1Seconds = 59;
    } else {
      blackWon = true;
    }
  } else if (currentPlayer == 1) {
    if (centiCounter2 < 10) {
      centiCounter2++;
      return;
    } else if (player2Seconds > 0) {
      centiCounter2 = 0;
      player2Seconds--;
    } else if (player2Minutes > 0) {
      centiCounter2 = 0;
      player2Minutes--;
      player2Seconds = 59;
    } else {
      whiteWon = true;
    }
  }
  displayCurrentTime();
}

void displayCurrentTime() {
  if (player1Seconds == 59 || player2Seconds == 59) lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<- W");
  led_display1.clear();
  setTime(player1Minutes, true, true);
  setTime(player1Seconds, true, false);
  led_display1.println(player1Time);
  led_display1.drawColon(true);
  led_display1.writeDisplay();

  lcd.setCursor(12, 0); // Display on the right side of the same row
  lcd.print("B ->");
  led_display2.clear();
  setTime(player2Minutes, false, true);
  setTime(player2Seconds, false, false);
  led_display2.println(player2Time);
  led_display2.drawColon(true);
  led_display2.writeDisplay();
}

void setTime(int timeSetting, bool player1, bool minutes) {
  int firstVar, secondVar;
  if (minutes) {
    firstVar = 0;
    secondVar = 1;
  } else {
    firstVar = 2;
    secondVar = 3;
  }
  if (player1) {
    if (timeSetting < 10) {
      player1Time[firstVar] = '0';
      player1Time[secondVar] = timeSetting + '0';
    } else {
      player1Time[firstVar] = timeSetting / 10 + '0';
      player1Time[secondVar] = timeSetting % 10 + '0';
    }
  } else {
    if (timeSetting < 10) {
      player2Time[firstVar] = '0';
      player2Time[secondVar] = timeSetting + '0';
    } else {
      player2Time[firstVar] = timeSetting / 10 + '0';
      player2Time[secondVar] = timeSetting % 10 + '0';
    }
  }
}
