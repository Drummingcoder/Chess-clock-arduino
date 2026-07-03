// List of functions: 
// setup(); and loop(); are the main functions, setup() runs once at the beginning to initialize pins, while loop() loops over and over after setup() runs
// editTime(bool notPaused); is in charge of changing time controls or advancing settings based on button pressed, works during setup or pause menu
// updateScreen(); works during setup, in charge of updating the screen as the time controls are set
// advanceTime(); works while the game is running, and is in charge of managing the timers and checking for timeout
// displayCurrentTime(); works while the game is running, and is in charge of updating the screen as the timers count down
// setTime(int timeSetting, bool player1, bool minutes); works during both stages, and is in charge of changing the arrays to fit the variables
// menuPause(); works while the game is paused, and gives the user the option to add or subtract time, or end the game early (also checks to unpause the game)
// get_ans(bool& var); stops execution and waits for a response from Button 1 or 2 (turns var to false if Button 1 is pressed, leaves it alone otherwise)
// checkButtons(); checks the state of the 3 buttons and adjusts the button states buttonP1pressed, buttonP2pressed, and buttonP3pressed accordingly
// startingGame(); starts the game, sets defaults, and reads time controls to EEPROM

#include <TM1637Display.h> // For controlling the 4-digit 7-segment LED displays
#include <LiquidCrystal.h> // To control the LCD display
#include <EEPROM.h> // Use the EEPROM memory to write time controls used for next time

// LCD Pins, initializing screen
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Initializing LED displays
TM1637Display display1(A4, A5);
TM1637Display display2(A2, A3);

// Button Pins
const int buttonP3 = 8; // Pause game or move to next setting
const int buttonP2 = 9; // Switch to player 2 or increase setting
const int buttonP1 = 10; // Switch to player 1 or decrease setting
const int buzzer = 7; // Buzzer pin

// Game variables
int setupPlayer = 0;  // Controls setup stages from 0 to 2
int setupNumber = 0;  // Controls from 0 to 1
int currentPlayer = 0; // 0 for Player 1 (White), 1 for Player 2 (Black)
bool gameRunning = false, gamePaused = false, whiteWon = false, blackWon = false; // Various check variables to check condition of the game
int player1Minutes = 0, player1Seconds = 0, player2Minutes = 0, player2Seconds = 0, increment = 0; // Store the time controls
int clone1, clone1s, clone2, clone2s;
char player1Time[5] = "0000", player2Time[5] = "0000"; // Variables to control what is printed to the LED display
int centiCounter1 = 0, centiCounter2 = 0, centiBeepCounter = 0; // Allows the clock to count in centiseconds for more accurate timing
int whiteGames = 0, blackGames = 0; // Games counter
bool gameStarted = true, beepOn = true, beeping = false, pauseMenu = false, casual = true; // Various checks

// Button states
bool buttonP1pressed = false, buttonP2pressed = false, buttonP3pressed = false;

// millis()-based timing and redraw state
const unsigned long CLOCK_TICK_MS = 100;      // 10 ticks at 100 ms each = 1 displayed second.
const unsigned long DEBOUNCE_MS = 200;
const unsigned long SET_DEBOUNCE_MS = 500;

unsigned long lastClockTickMillis = 0;
unsigned long lastSetupButtonMillis = 0;
unsigned long lastPauseButtonMillis = 0;
unsigned long beepStopMillis = 0;

int lastDisplay1Value = -1;
int lastDisplay2Value = -1;
uint8_t lastDisplay1Colon = 255;
uint8_t lastDisplay2Colon = 255;
bool display1Blanked = false;
bool display2Blanked = false;

int lastSetupPlayer = -1;
int lastSetupNumber = -1;
int lastSetupP1Minutes = -1;
int lastSetupP1Seconds = -1;
int lastSetupP2Minutes = -1;
int lastSetupP2Seconds = -1;
int lastSetupIncrement = -1;

bool lastGameLcdInitialized = false;
int lastDisplayedWhiteGames = -1;
int lastDisplayedBlackGames = -1;
int lastDisplayedDiffMinutes = 999;
int lastDisplayedDiffSeconds = 999;
bool lastDisplayedDiffPositive = false;
bool lastDisplayedCasual = false;
int lastDisplayedP1LowTick = -1;
int lastDisplayedP2LowTick = -1;

void startBeep(unsigned int durationMs) {
  if (beepOn) {
    tone(buzzer, 523);
    beepStopMillis = millis() + durationMs;
    beeping = true;
  }
}

void serviceBeep() {
  if (beeping && (long)(millis() - beepStopMillis) >= 0) {
    noTone(buzzer);
    beeping = false;
    centiBeepCounter = 0;
  }
}

void forceDisplayRefresh() {
  lastDisplay1Value = -1;
  lastDisplay2Value = -1;
  lastDisplay1Colon = 255;
  lastDisplay2Colon = 255;
  display1Blanked = false;
  display2Blanked = false;
}

void resetSetupDisplayCache() {
  lastSetupPlayer = -1;
  lastSetupNumber = -1;
  lastSetupP1Minutes = -1;
  lastSetupP1Seconds = -1;
  lastSetupP2Minutes = -1;
  lastSetupP2Seconds = -1;
  lastSetupIncrement = -1;
  forceDisplayRefresh();
}

void resetGameLcdCache() {
  lastGameLcdInitialized = false;
  lastDisplayedWhiteGames = -1;
  lastDisplayedBlackGames = -1;
  lastDisplayedDiffMinutes = 999;
  lastDisplayedDiffSeconds = 999;
  lastDisplayedDiffPositive = false;
  lastDisplayedCasual = false;
  lastDisplayedP1LowTick = -1;
  lastDisplayedP2LowTick = -1;
}

void showDisplayIfChanged(TM1637Display& display, int value, uint8_t colonMask, bool& blanked, int& lastValue, uint8_t& lastColon) {
  if (blanked || value != lastValue || colonMask != lastColon) {
    display.showNumberDecEx(value, colonMask, false);
    lastValue = value;
    lastColon = colonMask;
    blanked = false;
  }
}

void blankDisplayIfNeeded(TM1637Display& display, bool& blanked, int& lastValue, uint8_t& lastColon) {
  if (!blanked) {
    display.clear();
    blanked = true;
    lastValue = -1;
    lastColon = 255;
  }
}

// Function prototypes
void editTime(bool notPaused);
void updateScreen();
int convertArrtoInt(char arr[]);
void advanceTime();
void displayCurrentTime();
void setTime(int timeSetting, bool player1, bool minutes);
void menuPause();
void get_ans(bool& var);
void checkButtons();
void startingGame();
void handleTimeoutEnd();
void resetForReplay();
void resetAfterGameToSetup();
void startBeep(unsigned int durationMs);
void serviceBeep();
void forceDisplayRefresh();
void resetSetupDisplayCache();
void resetGameLcdCache();
void showDisplayIfChanged(TM1637Display& display, int value, uint8_t colonMask, bool& blanked, int& lastValue, uint8_t& lastColon);
void blankDisplayIfNeeded(TM1637Display& display, bool& blanked, int& lastValue, uint8_t& lastColon);

// For buzzer melody
const int G3 = 196, Arl = 220, B3 = 247, C4 = 262, C45 = 278, D4 = 294, E4 = 330, F4 = 349, F45 = 372, G4 = 392, Aru = 440, B4 = 494, C5 = 523, D5 = 566;

int melody[] = {
  Aru, Aru, Aru, Aru, C5, D5, Aru, G4, G4, F4, F4, E4, F4, D4
};

float noteDurations[] = {
  2.4, 4.8, 2.88, 2.88, 2.88, 2.88, 1.9, 2.4, 4.8, 2.88, 2.88, 2.88, 2.88, 2.88
};

// Runs once after the chess clock starts up, sets up variables and reads from EEPROM
void setup() {
  // Setup pin modes
  pinMode(buttonP3, INPUT_PULLUP);
  pinMode(buttonP2, INPUT);
  pinMode(buttonP1, INPUT);
  pinMode(buzzer, OUTPUT);

  // Initialize LED displays
  display1.setBrightness(0x04);
  display2.setBrightness(0x04);

  // Initialize LCD
  lcd.begin(16, 2);

	// Welcome message
  lcd.setCursor(0, 0);
  lcd.print("Welcome! Have");
  lcd.setCursor(0, 1);
  lcd.print("a good game!");
  delay(2000);

	// Turn beep off or on
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Turn beep off?");

  get_ans(beepOn);

	// Set casual or competitive mode
  delay(200);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Competitive or");
  lcd.setCursor(0, 1);
  lcd.print("casual?");

	get_ans(casual);
	bool checker2 = true;
	if (EEPROM.read(7) != casual) {
		EEPROM.write(7, casual);
		checker2 = false; // Skip "Use previous time?" settings if this setting doesn't match previous setting
	}

	if (checker2) {
	  // Check to use previous settings or not
	  delay(200);
	  lcd.clear();
	  lcd.setCursor(0, 0);
	  lcd.print("Use previous");
	  lcd.setCursor(0, 1);
	  lcd.print("time?");
	  
	  get_ans(checker2);
		
		if (!checker2) {
	    // Read saved settings from EEPROM
	    if (EEPROM.read(0) != 255) player1Minutes = EEPROM.read(0);
	    if (EEPROM.read(1) != 255) player1Seconds = EEPROM.read(1);
	    if (EEPROM.read(2) != 255) player2Minutes = EEPROM.read(2);
	    if (EEPROM.read(3) != 255) player2Seconds = EEPROM.read(3);
	    if (EEPROM.read(4) != 255) increment = EEPROM.read(4);
			// Skip setup
			setupPlayer = 2;
	    setupNumber = 2;
	    buttonP3pressed = true;
	  }
	}

	// Check to use previous score
  delay(200);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Use previous");
  lcd.setCursor(0, 1);
  lcd.print("score?");
  
  checker2 = true;
  get_ans(checker2);

  if (!checker2) {
    if (EEPROM.read(5) != 255) whiteGames = EEPROM.read(5);
    if (EEPROM.read(6) != 255) blackGames = EEPROM.read(6);
  }
  
  lcd.clear();
  // Display initial starting screen
  updateScreen();
}

void loop() {
  serviceBeep();

  if (gameRunning) {
    // Start the game only when player 1 presses the button
    while (gameStarted) {
      serviceBeep();
      if (digitalRead(buttonP1) == HIGH) {
        gameStarted = false;
        currentPlayer = 1;
        lastClockTickMillis = millis();
        lcd.clear();
        resetGameLcdCache();
        forceDisplayRefresh();
      }
    }

    // Handle button presses for switching players
    if (digitalRead(buttonP1) == HIGH && currentPlayer != 1) { // Player 1 button is pressed and the current player is player 1
      if (casual) {
        player1Minutes = clone1;
        player1Seconds = clone1s;
      } else {
        player1Seconds += increment;
        while (player1Seconds >= 60) { // In case the increment increases the # of seconds to be greater than 59
          player1Minutes++;
          player1Seconds -= 60;
        }
      }
      currentPlayer = 1; // Change current player to black (player 2)
      blankDisplayIfNeeded(display1, display1Blanked, lastDisplay1Value, lastDisplay1Colon);
      lastClockTickMillis = millis();
      forceDisplayRefresh();
      displayCurrentTime();
      startBeep(100); // Beep to switch players without blocking the timer
    } else if (digitalRead(buttonP2) == HIGH && currentPlayer != 0) { // Player 2 button is pressed and the current player is player 2
      if (casual) {
        player2Minutes = clone2;
        player2Seconds = clone2s;
      } else {
        player2Seconds += increment;
        while (player2Seconds >= 60) {
          player2Minutes++;
          player2Seconds -= 60;
        }
      }
      blankDisplayIfNeeded(display2, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
      currentPlayer = 0; // Change current player to white (player 1)
      lastClockTickMillis = millis();
      forceDisplayRefresh();
      displayCurrentTime();
      startBeep(100);
    } else if (digitalRead(buttonP3) == LOW) { // Pause button is pressed
      buttonP3pressed = true; // This variable is checked in advanceTime(); so no action is taken regarding this variable in this function
    } else { // No buttons are pressed
      buttonP1pressed = false; // Not used when game is running, but good to keep off in case
      buttonP2pressed = false; // Not used when game is running, but good to keep off in case
      buttonP3pressed = false;
    }

    // Call advanceTime() to advance the timer (of current player) and update the timers
    advanceTime();

    // Checks if either side has won yet (the variables whiteWon and blackWon are manipulated in advanceTime())
    // It will transition the game into a paused state, clear the screen and display timeout message
    if (whiteWon) { // Player 2 (black) has timed out
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Black timeout");
    } else if (blackWon) { // Player 1 (white) has timed out
      gamePaused = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("White timeout");
    }

    // Checks if the game is paused or over. If the game ended by timeout, offer the same replay/reset choice
    // instead of getting stuck in the timeout melody forever.
    while (gamePaused) {
      if (whiteWon || blackWon) {
        handleTimeoutEnd();
      } else {
        serviceBeep();
        menuPause();
      }
    }
  } else {
    // If the game is not running (meaning that the time controls are being set up), execute this section of the code
    unsigned long now = millis();
    if (now - lastSetupButtonMillis >= DEBOUNCE_MS) {
      // Checks each button to see if it's pressed, and sets the button variables to the appropriate values
      checkButtons();

      // Calls editTime() to change the values of the time control variables or advance the setting stage
      editTime(true);

      if (buttonP1pressed || buttonP2pressed || buttonP3pressed) {
        lastSetupButtonMillis = now;
      }
    } else {
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }

    // Calls updateScreen() to update the screen as the time variables change. It now redraws only when needed.
    updateScreen();
  }
}

// This function is in charge of checking the state of each button variable and changing the appropriate variable (or advancing the
// setting stage).
void editTime(bool notPaused) {
  if (buttonP3pressed) { // SET button is pressed
    if (setupNumber < 1) { // Minutes (of either player) were being set, move on to seconds
      setupNumber++;
    } else if (setupPlayer < 1) { // If execution moved here, seconds are done being set. If player 1's time was the one being set, move on to player 2.
      setupNumber = 0;
      setupPlayer++;
    } else if (setupPlayer == 1) { // If player 2's time is done being set, move on to the increment time control
			if (casual && notPaused) {
				startingGame();
				lcd.clear();
	      displayCurrentTime();
			} else if (casual && (!notPaused)) {
				lcd.clear();
        pauseMenu = false;
        displayCurrentTime();
        lcd.clear();
			} else {
	      setupPlayer++;
	      setupNumber = 2; // To ensure that the first if block (of this if-else tree) isn't triggered and pass a check later on
	      lcd.clear();
			}
    } else { // Increment time control is done being set, start the game
			if (notPaused) {
	      startingGame();
	      if (increment != EEPROM.read(4)) EEPROM.write(4, increment);
	      
	      lcd.clear();
	      displayCurrentTime();
			} else {
				lcd.clear();
        pauseMenu = false;
        displayCurrentTime();
        lcd.clear();
			}
    }

    lastSetupButtonMillis = millis();

    while (digitalRead(buttonP3) == LOW) {
      serviceBeep();
    }

    buttonP3pressed = false;
  }
  if (buttonP2pressed) { // Increment button is pressed
    if (setupPlayer == 0) { // Player 1 (white)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
				if (notPaused) {
	        if (player1Minutes >= 15 && player1Minutes <= 25) player1Minutes += 5;
	        else if (player1Minutes == 30) player1Minutes += 10;
	        else if (player1Minutes >= 40 && player1Minutes < 50) player1Minutes += 5;
	        else if (player1Minutes >= 50) player1Minutes += 10;
	        else player1Minutes++;
				} else {
					player1Minutes++;
				}
        if (player1Minutes > 99) player1Minutes = 99; // If minutes are over 120, bring it back to 120
      } else if (setupNumber == 1) { // Seconds are being set
        player1Seconds++;
        if (player1Seconds >= 60) player1Seconds = 59; // If seconds are over 59, bring it back to 59
      }
    } else if (setupPlayer == 1) { // Player 2 (Black)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
				if (notPaused) {
	        if (player2Minutes >= 15 && player2Minutes <= 25) player2Minutes += 5;
	        else if (player2Minutes == 30) player2Minutes += 10;
	        else if (player2Minutes >= 40 && player2Minutes < 50) player2Minutes += 5;
	        else if (player2Minutes >= 50) player2Minutes += 10;
	        else player2Minutes++;
				} else {
					player2Minutes++;
				}
        if (player2Minutes > 99) player2Minutes = 99; // If minutes are over 120, bring it back to 120
      } else if (setupNumber == 1) { // Seconds are being set
        player2Seconds++;
        if (player2Seconds >= 60) player2Seconds = 59; // If seconds are over 59, bring it back to 59
      }
    } else if (setupNumber == 2) { // Increment time control is being set
			if (notPaused) {
	      if (increment >= 15) increment += 5;
	      else increment++;
			} else {
				increment++;
			}
      if (increment > 60) increment = 60; // If increment is over 60, bring it back to 60
    }
  }
  if (buttonP1pressed) { // Decrement button was pressed
    if (setupPlayer == 0) { // Player 1 (white)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
				if (notPaused) {
	        if (player1Minutes >= 20 && player1Minutes <= 30) player1Minutes -= 5;
	        else if (player1Minutes == 40) player1Minutes -= 10;
	        else if (player1Minutes == 45 || player1Minutes == 50) player1Minutes -= 5;
	        else if (player1Minutes >= 60) player1Minutes -= 10;
	        else player1Minutes--;
				} else {
					player1Minutes--;
				}
        if (player1Minutes < 0) player1Minutes = 0; // If minutes are negative, bring it back to 0
      } else if (setupNumber == 1) { // Seconds are being set
        player1Seconds--;
        if (player1Seconds < 0) player1Seconds = 0; // If seconds are negative, bring it back to 0
      }
    } else if (setupPlayer == 1) { // Player 2 (Black)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
				if (notPaused) {
	        if (player2Minutes >= 20 && player2Minutes <= 30) player2Minutes -= 5;
	        else if (player2Minutes == 40) player2Minutes -= 10;
	        else if (player2Minutes == 45 || player2Minutes == 50) player2Minutes -= 5;
	        else if (player2Minutes >= 60) player2Minutes -= 10;
	        else player2Minutes--;
				} else {
					player2Minutes--;
				}
        if (player2Minutes < 0) player2Minutes = 0; // If minutes are negative, bring it back to 0
      } else if (setupNumber == 1) { // Seconds are being set
        player2Seconds--;
        if (player2Seconds < 0) player2Seconds = 0; // If seconds are negative, bring it back to 0
      }
    } else if (setupNumber == 2) {
			if (notPaused) {
	      if (increment >= 20) increment -= 5;
	      else increment--;
			} else {
				increment--;
			}
      if (increment < 0) increment = 0; // If increment is negative, bring it back to 0
    }
  }
}

// During the setup process, display the labels accordingly to the stage of setup it's in
void updateScreen() {
  bool labelsChanged = (setupPlayer != lastSetupPlayer || setupNumber != lastSetupNumber);
  bool displayChanged = labelsChanged ||
                        player1Minutes != lastSetupP1Minutes || player1Seconds != lastSetupP1Seconds ||
                        player2Minutes != lastSetupP2Minutes || player2Seconds != lastSetupP2Seconds ||
                        increment != lastSetupIncrement;

  if (!displayChanged) return;

  // Initialize arrays of labels
  const char* labels[] = {"Minutes: ", "Seconds: ", "Increment: "};
  const char* player[] = {"Player 1: ", "Player 2: ", "Both: "};

  // Display appropriate labels only when the setup stage changes
  if (labelsChanged) {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print(player[setupPlayer]);
    lcd.setCursor(0,1);
    lcd.print(labels[setupNumber]);
  }

  // Displays appropriate label based on setting stage
  if (setupPlayer == 0) { // Player 1's time is being set
    // Print White's time on display 1
    setTime(player1Minutes, true, true);
    setTime(player1Seconds, true, false);
    showDisplayIfChanged(display1, convertArrtoInt(player1Time), 0b01000000, display1Blanked, lastDisplay1Value, lastDisplay1Colon);
    blankDisplayIfNeeded(display2, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
  } else if (setupPlayer == 1) { // Player 2's time is being set
    // Keep White's completed setup time visible on display 1 while setting Black's time on display 2.
    setTime(player1Minutes, true, true);
    setTime(player1Seconds, true, false);
    showDisplayIfChanged(display1, convertArrtoInt(player1Time), 0b01000000, display1Blanked, lastDisplay1Value, lastDisplay1Colon);

    setTime(player2Minutes, false, true);
    setTime(player2Seconds, false, false);
    showDisplayIfChanged(display2, convertArrtoInt(player2Time), 0b01000000, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
  } else { // Increment time control is being set
    // Clear both screens only when first entering this stage, print increment on display 1
    showDisplayIfChanged(display1, increment, 0b00000000, display1Blanked, lastDisplay1Value, lastDisplay1Colon);
    blankDisplayIfNeeded(display2, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
  }

  lastSetupPlayer = setupPlayer;
  lastSetupNumber = setupNumber;
  lastSetupP1Minutes = player1Minutes;
  lastSetupP1Seconds = player1Seconds;
  lastSetupP2Minutes = player2Minutes;
  lastSetupP2Seconds = player2Seconds;
  lastSetupIncrement = increment;
}

int convertArrtoInt(char arr[]) {
  return (arr[0] - '0') * 1000 +
         (arr[1] - '0') * 100 +
         (arr[2] - '0') * 10 +
         (arr[3] - '0');
}

// While the game is running, based on whose turn it is, this function will update the corresponding timer and call
// displayCurrentTime() to update the screen
void advanceTime() {
  if (buttonP3pressed) { // Pause button is pressed
    gamePaused = true;
    lcd.clear();
    resetGameLcdCache();
    return; // Return execution to loop()
  }

  unsigned long now = millis();
  bool timerChanged = false;
  bool ticked = false;

  while (now - lastClockTickMillis >= CLOCK_TICK_MS) {
    lastClockTickMillis += CLOCK_TICK_MS;
    ticked = true;

    // Update timer based on whose turn it is
    if (currentPlayer == 0) { // White's turn
      if (centiCounter1 < 9) {
        centiCounter1++;
      } else if (player1Seconds > 0) { // If there are seconds left to reduce
        centiCounter1 = 0;
        player1Seconds--;
        timerChanged = true;
      } else if (player1Minutes > 0) { // If there are minutes left to reduce
        centiCounter1 = 0;
        player1Minutes--;
        player1Seconds = 59;
        timerChanged = true;
      } else { // White's time has run out, so Black wins
        blackWon = true;
        blackGames++;
        if (whiteGames != EEPROM.read(5)) EEPROM.write(5, whiteGames);
        if (blackGames != EEPROM.read(6)) EEPROM.write(6, blackGames);
        break;
      }
    } else if (currentPlayer == 1) { // Black's turn
      if (centiCounter2 < 9) {
        centiCounter2++;
      } else if (player2Seconds > 0) { // If there are seconds left to reduce
        centiCounter2 = 0;
        player2Seconds--;
        timerChanged = true;
      } else if (player2Minutes > 0) { // If there are minutes left to reduce
        centiCounter2 = 0;
        player2Minutes--;
        player2Seconds = 59;
        timerChanged = true;
      } else { // Black's time has run out, so White wins
        whiteWon = true;
        whiteGames++;
        if (whiteGames != EEPROM.read(5)) EEPROM.write(5, whiteGames);
        if (blackGames != EEPROM.read(6)) EEPROM.write(6, blackGames);
        break;
      }
    }
  }

  // Update the display only when a tick can change the colon/low-time indicator or a full second changed the digits.
  if (ticked || timerChanged || whiteWon || blackWon) {
    displayCurrentTime();
  }
}

// This function is responsible for updating the screen while the game is running, called by advanceTime()
void displayCurrentTime() {
  serviceBeep();

  // Warning beeps at 1:00 and 0:10, but only on the exact fresh tick so they do not retrigger all second.
  if (((player1Minutes == 1 && player1Seconds == 0 && centiCounter1 == 0) && currentPlayer == 0) ||
      ((player2Minutes == 1 && player2Seconds == 0 && centiCounter2 == 0) && currentPlayer == 1)) {
    if (!gameStarted) startBeep(1000);
  } else if (((player1Minutes == 0 && player1Seconds == 10 && centiCounter1 == 0) && currentPlayer == 0) ||
             ((player2Minutes == 0 && player2Seconds == 10 && centiCounter2 == 0) && currentPlayer == 1)) {
    if (!gameStarted) startBeep(1000);
  }

  // LCD display. Static labels are written once; changing values are updated only when their values change.
  if (!lastGameLcdInitialized) {
    lcd.setCursor(0, 0);
    lcd.print("<- W");
    lcd.setCursor(12, 0); // Display on the right side of the same row
    lcd.print("B ->");
    lastGameLcdInitialized = true;
  }

  if (whiteGames != lastDisplayedWhiteGames || blackGames != lastDisplayedBlackGames) {
    lcd.setCursor(6, 0);
    lcd.print("      ");
    lcd.setCursor(6, 0);
    lcd.print(whiteGames);
    lcd.print("- ");
    lcd.print(blackGames);
    lastDisplayedWhiteGames = whiteGames;
    lastDisplayedBlackGames = blackGames;
  }

  // Display time difference if competitive mode
  if (!casual) {
    // Calculate time difference
    int diffMinutes, diffSeconds, firstMinutes, firstSeconds, secondMinutes, secondSeconds;
    if (currentPlayer == 0) { // White's turn
      firstMinutes = player1Minutes;
      firstSeconds = player1Seconds;
      secondMinutes = player2Minutes;
      secondSeconds = player2Seconds;
    } else {
      firstMinutes = player2Minutes;
      firstSeconds = player2Seconds;
      secondMinutes = player1Minutes;
      secondSeconds = player1Seconds;
    }

    diffMinutes = firstMinutes - secondMinutes;
    diffSeconds = firstSeconds - secondSeconds;

    if (diffSeconds < 0 && diffMinutes > 0) {
      diffMinutes--;
      diffSeconds += 60;
    } else if (diffSeconds > 0 && diffMinutes < 0) {
      diffMinutes++;
      diffSeconds -= 60;
    }

    bool diffPositive = ((diffMinutes > 0) || (diffMinutes == 0 && diffSeconds >= 0));

    if (diffMinutes < 0) diffMinutes *= -1;
    if (diffSeconds < 0) diffSeconds *= -1;

    if (casual != lastDisplayedCasual || diffMinutes != lastDisplayedDiffMinutes || diffSeconds != lastDisplayedDiffSeconds || diffPositive != lastDisplayedDiffPositive) {
      lcd.setCursor(5, 1);
      lcd.print("      ");
      lcd.setCursor(5, 1);
      if (diffPositive) lcd.print("+");
      else lcd.print("-");
      if (diffMinutes < 10) lcd.print("0");
      lcd.print(diffMinutes);
      lcd.print(":");
      if (diffSeconds < 10) lcd.print("0");
      lcd.print(diffSeconds);

      lastDisplayedDiffMinutes = diffMinutes;
      lastDisplayedDiffSeconds = diffSeconds;
      lastDisplayedDiffPositive = diffPositive;
      lastDisplayedCasual = casual;
    }
  } else {
    if (casual != lastDisplayedCasual) {
      lcd.setCursor(5, 1);
      lcd.print("Casual");
      lastDisplayedCasual = casual;
    }
  }

  int p1LowTick = -1;
  if (player1Minutes == 0 && player1Seconds <= 20) p1LowTick = 10 - centiCounter1;
  if (p1LowTick != lastDisplayedP1LowTick) {
    lcd.setCursor(0, 1);
    if (p1LowTick >= 0) {
      lcd.print(".");
      lcd.print(p1LowTick);
      lcd.setCursor(2, 1);
      lcd.print(" ");
    } else {
      lcd.print("   ");
    }
    lastDisplayedP1LowTick = p1LowTick;
  }

  int p2LowTick = -1;
  if (player2Minutes == 0 && player2Seconds <= 20) p2LowTick = 10 - centiCounter2;
  if (p2LowTick != lastDisplayedP2LowTick) {
    lcd.setCursor(14, 1);
    if (p2LowTick >= 0) {
      lcd.print(".");
      lcd.print(p2LowTick);
    } else {
      lcd.print("  ");
    }
    lastDisplayedP2LowTick = p2LowTick;
  }

  bool d1colon = true, d2colon = true;
  if (centiCounter1 < 5 && currentPlayer == 0) {
    d1colon = false;
  }
  if (centiCounter2 < 5 && currentPlayer == 1) {
    d2colon = false;
  }

  // Player 1 (White) time display. No clear-before-redraw; only update if value or colon changed.
  setTime(player1Minutes, true, true);
  setTime(player1Seconds, true, false);
  showDisplayIfChanged(display1, convertArrtoInt(player1Time), d1colon ? 0b01000000 : 0b00000000,
                       display1Blanked, lastDisplay1Value, lastDisplay1Colon);

  // Player 2 (Black) time display. No clear-before-redraw; only update if value or colon changed.
  setTime(player2Minutes, false, true);
  setTime(player2Seconds, false, false);
  showDisplayIfChanged(display2, convertArrtoInt(player2Time), d2colon ? 0b01000000 : 0b00000000,
                       display2Blanked, lastDisplay2Value, lastDisplay2Colon);
}

// Sets the arrays player1Time and player2Time to the appropriate numbers
void setTime(int timeSetting, bool player1, bool minutes) {
  // Initialize variables
  int firstVar, secondVar;

  // Checking to set minutes or seconds
  if (minutes) {
    firstVar = 0;
    secondVar = 1;
  } else {
    firstVar = 2;
    secondVar = 3;
  }

  // Adjust the arrays based on timeSetting
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

void resetForReplay() {
  player1Minutes = clone1;
  player1Seconds = clone1s;
  player2Minutes = clone2;
  player2Seconds = clone2s;
  centiCounter1 = 0;
  centiCounter2 = 0;
  whiteWon = false;
  blackWon = false;
  gamePaused = false;
  gameStarted = true;
  buttonP1pressed = false;
  buttonP2pressed = false;
  buttonP3pressed = false;
  lastClockTickMillis = millis();
  lcd.clear();
  resetGameLcdCache();
  forceDisplayRefresh();
  displayCurrentTime();
}

void resetAfterGameToSetup() {
  gameRunning = false;
  gamePaused = false;
  gameStarted = true;
  whiteWon = false;
  blackWon = false;
  pauseMenu = false;
  player1Minutes = 0;
  player1Seconds = 0;
  player2Minutes = 0;
  player2Seconds = 0;
  centiCounter1 = 0;
  centiCounter2 = 0;
  setupPlayer = 0;
  setupNumber = 0;
  lcd.clear();
  buttonP1pressed = false;
  buttonP2pressed = false;
  buttonP3pressed = false;
  // Preserve the original reset behavior: clear Player 2's display and show 00:00 there.
  blankDisplayIfNeeded(display2, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
  display2.showNumberDecEx(0000, 0b01000000, true);
  display2Blanked = false;
  lastDisplay2Value = 0;
  lastDisplay2Colon = 0b01000000;
  resetSetupDisplayCache();
}

void handleTimeoutEnd() {
  // Keep the known timeout result on the LCD briefly, then play the same end melody once if enabled.
  delay(500);
  if (beepOn) {
    for (int thisNote = 0; thisNote < 14; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(buzzer, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(buzzer);
    }
    delay(500);
    noTone(buzzer);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Play again?");

  bool checker3 = true;
  get_ans(checker3);
  if (!checker3) {
    resetForReplay();
  } else {
    resetAfterGameToSetup();
  }
}

void menuPause() {
  unsigned long now = millis();

  if (!pauseMenu) {
    // Display "Game Paused" message
    lcd.setCursor(0, 0);
    lcd.print("Game Paused");
  }
  if ((digitalRead(buttonP1) == HIGH) && (!pauseMenu) && (now - lastPauseButtonMillis >= DEBOUNCE_MS)) {
    lastPauseButtonMillis = now;
    pauseMenu = true;
    setupPlayer = 0;
    setupNumber = 0;
    lcd.clear();
    resetSetupDisplayCache();
    updateScreen();
  } else if ((digitalRead(buttonP3) == LOW) && (!pauseMenu) && (now - lastPauseButtonMillis >= DEBOUNCE_MS)) { // Pause button is pressed to unpause the game
    lastPauseButtonMillis = now;
    gamePaused = false;
    buttonP3pressed = false; // Ensure that the game isn't paused again in the advanceTime() function
    lastClockTickMillis = millis();
    lcd.clear();
    resetGameLcdCache();
  } else if ((digitalRead(buttonP2) == HIGH) && (!pauseMenu) && (now - lastPauseButtonMillis >= DEBOUNCE_MS)) {
    lastPauseButtonMillis = now;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Who won?");
    bool checker3 = true;
    get_ans(checker3);
    if (!checker3) whiteGames++;
    else blackGames++;
    if (whiteGames != EEPROM.read(5)) EEPROM.write(5, whiteGames);
    if (blackGames != EEPROM.read(6)) EEPROM.write(6, blackGames);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Play again?");
    checker3 = true;
    get_ans(checker3);
    if (!checker3) {
      player1Minutes = clone1;
      player1Seconds = clone1s;
      player2Minutes = clone2;
      player2Seconds = clone2s;
      gamePaused = false;
      gameStarted = true;
      lcd.clear();
      resetGameLcdCache();
      forceDisplayRefresh();
      displayCurrentTime();
    } else {
      gameRunning = false;
      player1Minutes = 0;
      player1Seconds = 0;
      player2Minutes = 0;
      player2Seconds = 0;
      gamePaused = false;
      gameStarted = true;
      setupPlayer = 0;
      setupNumber = 0;
      lcd.clear();
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
      // Clear Player 2 Screen
      blankDisplayIfNeeded(display2, display2Blanked, lastDisplay2Value, lastDisplay2Colon);
      display2.showNumberDecEx(0000, 0b01000000, true);
      display2Blanked = false;
      lastDisplay2Value = 0;
      lastDisplay2Colon = 0b01000000;
      resetSetupDisplayCache();
    }
  }

  if (pauseMenu) {
    if (now - lastPauseButtonMillis >= DEBOUNCE_MS) {
      checkButtons();
      editTime(false);
      if (buttonP1pressed || buttonP2pressed || buttonP3pressed) {
        lastPauseButtonMillis = now;
      }
    } else {
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }

    updateScreen();
  }
}

// Repeated functions
void get_ans(bool& var) {
  bool checker = true;
  while (checker) {
    if (digitalRead(buttonP1) == HIGH) {
      checker = false;
      var = false;
    } else if (digitalRead(buttonP2) == HIGH) {
      checker = false;
    }
  }
  delay(100);
}

void checkButtons() {
	// Also, thanks to this if-else statement, when multiple buttons are pressed, player 1 button has 1st priority, and
  // player 2 button has 2nd priority
	if (digitalRead(buttonP1) == HIGH) { // Decrement button is pressed
    buttonP1pressed = true;
    buttonP2pressed = false;
    buttonP3pressed = false;
  } else if (digitalRead(buttonP2) == HIGH) { // Increment button is pressed
    buttonP1pressed = false;
    buttonP2pressed = true;
    buttonP3pressed = false;
  } else if (digitalRead(buttonP3) == LOW) { // SET button is pressed
    buttonP1pressed = false;
    buttonP2pressed = false;
    buttonP3pressed = true;
  } else { // No buttons are pressed
    buttonP1pressed = false;
    buttonP2pressed = false;
    buttonP3pressed = false;
  }
}

void startingGame() {
	gameRunning = true;
  lastClockTickMillis = millis();
  centiCounter1 = 0;
  centiCounter2 = 0;
  resetGameLcdCache();
  forceDisplayRefresh();
	      
	if (player1Minutes == 0 && player1Seconds == 0) { // Sets player 1's time to 10 minutes if it's left blank
	  player1Minutes = 10;
	}
	if (player2Minutes == 0 && player2Seconds == 0) { // Sets player 2's time to player 1's if it's left blank
	  player2Minutes = player1Minutes;
	  player2Seconds = player1Seconds;
	}
	
	clone1 = player1Minutes;
	clone1s = player1Seconds;
	clone2 = player2Minutes;
	clone2s = player2Seconds;
		      
	// Write variables to memory only if they have changed
	if (player1Minutes != EEPROM.read(0)) EEPROM.write(0, player1Minutes);
	if (player1Seconds != EEPROM.read(1)) EEPROM.write(1, player1Seconds);
	if (player2Minutes != EEPROM.read(2)) EEPROM.write(2, player2Minutes);
	if (player2Seconds != EEPROM.read(3)) EEPROM.write(3, player2Seconds);
}
