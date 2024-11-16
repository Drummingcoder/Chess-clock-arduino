// List of functions: 
// setup(); and loop(); are the main functions, setup() runs once at the beginning to initialize pins, while loop() loops over and over after setup() runs
// editTime(); works during setup, and is in charge of changing time controls or advancing settings based on button pressed
// updateScreen(); works during setup, in charge of updating the screen as the time controls are set
// advanceTime(); works while the game is running, and is in charge of managing the timers and checking for timeout
// displayCurrentTime(); works while the game is running, and is in charge of updating the screen as the timers count down

#include <LiquidCrystal.h> // To control the LCD display
#include <EEPROM.h> // Use the EEPROM memory to write time controls used for next time

// LCD Pins, initializing screen
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Button Pins
const int buttonP3 = 8; // Pause game or move to next setting
const int buttonP2 = 9; // Switch to player 2 or increase setting
const int buttonP1 = 10; // Switch to player 1 or decrease setting

// Buzzer pin
const int buzzer = 7;

int setupPlayer = 0;  // Controls setup stages from 0 to 2
int setupNumber = 0;  // Controls from 0 to 1
int currentPlayer = 0; // 0 for Player 1 (White), 1 for Player 2 (Black)
bool gameRunning = false, gamePaused = false, whiteWon = false, blackWon = false; // Various check variables to check condition of the game
int player1Minutes = 0, player1Seconds = 0, player2Minutes = 0, player2Seconds = 0, increment = 0; // Store the time controls
int centiCounter1 = 0, centiCounter2 = 0; // Allows the clock to count in centiseconds for more accurate timing

// Variables to check on the buttons
bool buttonP1pressed = false, buttonP2pressed = false, buttonP3pressed = false;

// Runs once after the chess clock starts up, sets up variables and reads from EEPROM
void setup() {
  // Initialize buttons and speaker
  pinMode(buttonP3, INPUT);
  pinMode(buttonP2, INPUT);
  pinMode(buttonP1, INPUT);
  pinMode(buzzer, OUTPUT);

  // Initialize LcD screen
  lcd.begin(16, 2);

  // Read EEPROM and initialize variables if EEPROM is not blank
  if (EEPROM.read(0) != 255) player1Minutes = EEPROM.read(0);
  if (EEPROM.read(1) != 255) player1Seconds = EEPROM.read(1);
  if (EEPROM.read(2) != 255) player2Minutes = EEPROM.read(2);
  if (EEPROM.read(3) != 255) player2Seconds = EEPROM.read(3);
  if (EEPROM.read(4) != 255) increment = EEPROM.read(4);

  // Display initial starting screen
  updateScreen();
}

// Continues to loop throughout the entire time the chess clock is on (after the setup function runs)
void loop() {
  // Big if-else statement checks if the game is running or not and executes appropriate code based on the result
  if (gameRunning) {
    // If the game is running, execute this section of the code

    // Checks the states of the buttons, and executes the corresponding code to switch current player, or pause the game
    // Also, thanks to this if-else statement, when multiple buttons are pressed, player 1 button has 1st priority, and
    // player 2 button has 2nd priority
    if (digitalRead(buttonP1) == HIGH && currentPlayer != 1) { // Player 1 button is pressed and the current player is player 1
      player1Seconds += increment;
      while (player1Seconds >= 60) { // In case the increment increases the # of seconds to be greater than 59
        player1Minutes++;
        player1Seconds -= 60;
      }
      currentPlayer = 1; // Change current player to black (player 2)
    } else if (digitalRead(buttonP2) == HIGH && currentPlayer != 0) { // Player 2 button is pressed and the current player is player 2
      player2Seconds += increment;
      while (player2Seconds >= 60) {
        player2Minutes++;
        player2Seconds -= 60;
      }
      currentPlayer = 0; // Change current player to white (player 1)
    } else if (digitalRead(buttonP3) == HIGH) { // Pause button is pressed
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

    // Checks if the game is paused or over, and keeps execution in this loop until the Pause button is pressed again if game is paused,
    // otherwise just pauses execution infinitely (which means that the game is over)
    while (gamePaused) {
      while (whiteWon || blackWon) { // Infinitely makes the buzzer beep until chess clock is reset or turned off
        noTone(buzzer);
        delay(500);
        tone(buzzer, 523);
        delay(500);
      }

      // Clear the screen and display "Game Paused" message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Game Paused");
      
      delay(200); // Debounce delay
      if (digitalRead(buttonP3) == HIGH) { // Pause button is pressed to unpause the game
        gamePaused = false;
        buttonP3pressed = false; // Ensure that the game isn't paused again in the advanceTime() function
        lcd.clear();
        delay(200); // Debounce delay
      }
    }
    delay(100); // Debounce delay, serves dual purpose of ensuring that advanceTime() function is only run every 0.01s, allowing
                // the centiseconds counter to only increment every centisecond and therefore count correctly
  }
  else {
    // If the game is not running (meaning that the time controls are being set up), execute this section of the code

    // Calls editTime() to change the values of the time control variables or advance the setting stage
    editTime();

    // Calls updateScreen() to update the screen as the time variables change
    updateScreen();

    // Checks each button to see if it's pressed, and sets the button variables to the appropriate values
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
    } else if (digitalRead(buttonP3) == HIGH) { // SET button is pressed
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = true;
    } else { // No buttons are pressed
      buttonP1pressed = false;
      buttonP2pressed = false;
      buttonP3pressed = false;
    }
    
    delay(200); // Debounce delay
  }
}

// This function is in charge of checking the state of each button variable and changing the appropriate variable (or advancing the
// setting stage).
void editTime() {
  if (buttonP3pressed) { // SET button is pressed
    lcd.clear();
    if (setupNumber < 1) { // Minutes (of either player) were being set, move on to seconds
      setupNumber++;
    } else if (setupPlayer < 1) { // If execution moved here, seconds are done being set. If player 1's time was the one being set, move on to player 2.
      setupNumber = 0;
      setupPlayer++;
    } else if (setupPlayer == 1) { // If player 2's time is done being set, move on to the increment time control
      if (player2Minutes == 0 && player2Seconds == 0) { // Sets player 2's time to player 1's if it's left blank
        player2Minutes = player1Minutes;
        player2Seconds = player1Seconds;
      }
      setupPlayer++;
      setupNumber = 2; // To ensure that the first if block (of this if-else tree) isn't triggered and pass a check later on
    } else { // Increment time control is done being set, start the game
      gameRunning = true;
      
      // Write variables to memory only if they have changed
      if (player1Minutes != EEPROM.read(0)) EEPROM.write(0, player1Minutes);
      if (player1Seconds != EEPROM.read(1)) EEPROM.write(1, player1Seconds);
      if (player2Minutes != EEPROM.read(2)) EEPROM.write(2, player2Minutes);
      if (player2Seconds != EEPROM.read(3)) EEPROM.write(3, player2Seconds);
      if (increment != EEPROM.read(4)) EEPROM.write(4, increment);
    }
    
    delay(500); // Small delay before the game begins
  }
  if (buttonP2pressed) { // Increment button is pressed
    lcd.clear();
    if (setupPlayer == 0) { // Player 1 (white)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
        player1Minutes++;
        if (player1Minutes > 999) { // If minutes are over 999, bring it back to 999
          player1Minutes = 999;
        }
      } else if (setupNumber == 1) { // Seconds are being set
        player1Seconds++;
        if (player1Seconds >= 60) { // If seconds are over 59, bring it back to 59
          player1Seconds = 59;
        }
      }
    } else if (setupPlayer == 1) { // Player 2 (Black)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
        player2Minutes++;
        if (player2Minutes > 999) // If minutes are over 999, bring it back to 999
        {
          player2Minutes = 999;
        }
      } else if (setupNumber == 1) { // Seconds are being set
        player2Seconds++;
        if (player2Seconds >= 60) // If seconds are over 59, bring it back to 59
        {
          player2Seconds = 59;
        }
      }
    }
    else if (setupNumber == 2) { // Increment time control is being set
      increment++;
      if (increment > 60) { // If increment is over 60, bring it back to 60
        increment = 60;
      }
    }
  }
  if (buttonP1pressed) { // Decrement button was pressed
    lcd.clear();
    if (setupPlayer == 0) { // Player 1 (white)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
        player1Minutes--;
        if (player1Minutes < 0) { // If minutes are negative, bring it back to 0
          player1Minutes = 0;
        }
      } else if (setupNumber == 1) { // Seconds are being set
        player1Seconds--;
        if (player1Seconds < 0) { // If seconds are negative, bring it back to 0
          player1Seconds = 0;
        }
      }
    }
    else if (setupPlayer == 1) { // Player 2 (Black)'s time is being set
      if (setupNumber == 0) { // Minutes are being set
        player2Minutes--;
        if (player2Minutes < 0) { // If minutes are negative, bring it back to 0
          player2Minutes = 0;
        }
      } else if (setupNumber == 1) { // Minutes are being set
        player2Seconds--;
        if (player2Seconds < 0) { // If seconds are negative, bring it back to 0
          player2Seconds = 0;
        }
      }
    }
    else if (setupNumber == 2) { // Increment time control is being set
      increment--;
      if (increment < 0) { // If increment is negative, bring it back to 0
        increment = 0;
      }
    }
  }
}

// During the setup process, display the labels accordingly to the stage of setup it's in
void updateScreen() {
  // Initialize arrays of labels
  const char* labels[] = {"Minutes: ", "Seconds: ", "Increment: "};
  const char* player[] = {"Player 1: ", "Player 2: ", "Both: "};

  // Display appropriate labels
  lcd.setCursor(0,0);
  lcd.print(player[setupPlayer]);
  lcd.setCursor(0,1);
  lcd.print(labels[setupNumber]);

  // Displays appropriate label based on setting stage
  if (setupPlayer == 0) { // Player 1's time is being set
    if (setupNumber == 0) { // Minutes is being set
      lcd.print(player1Minutes);
    }
    else { // Seconds is being set
      lcd.print(player1Seconds);
    }
  }
  else if (setupPlayer == 1) { // Player 2's time is being set
    if (setupNumber == 0) { // Minutes is being set
      lcd.print(player2Minutes);
    }
    else { // Seconds is being set
      lcd.print(player2Seconds);
    }
  }
  else { // Increment time control is being set
    lcd.print(increment);
  }
}

// While the game is running, based on whose turn it is, this function will update the corresponding timer and call
// displayCurrentTime() to update the screen
void advanceTime() {
  if (buttonP3pressed) { // Pause button is pressed
    gamePaused = true;
    return; // Return execution to loop()
  }

  // Update timer based on whose turn it is
  if (currentPlayer == 0) { // White's turn
    if (centiCounter1 < 10) { // As this function only runs every 0.01s, this centisecond counter will increment at the correct time
      centiCounter1++;
    }
    else if (player1Seconds > 0) { // If there are seconds left to reduce
      centiCounter1 = 0;
      player1Seconds--;
    }
    else if (player1Minutes > 0) { // If there are minutes left to reduce
      centiCounter1 = 0;
      player1Minutes--;
      player1Seconds = 59;
    }
    else { // White's time has run out, so Black wins
      blackWon = true;
    }
  }
  else if (currentPlayer == 1) { // Black's turn
    if (centiCounter2 < 10) {
      centiCounter2++;
    }
    else if (player2Seconds > 0) { // If there are seconds left to reduce
      centiCounter2 = 0;
      player2Seconds--;
    }
    else if (player2Minutes > 0) { // If there are minutes left to reduce
      centiCounter2 = 0;
      player2Minutes--;
      player2Seconds = 59;
    }
    else { // Black's time has run out, so White wins
      whiteWon = true;
    }
  }

  // Call displayCurrentTime() to update the screen
  displayCurrentTime();
}

// This function is responsible for updating the screen while the game is running, called by advanceTime()
void displayCurrentTime() {
  if (player1Seconds == 59 || player2Seconds == 59) { // Only blink the screen once either time reaches 59 seconds (minutes don't matter)
    lcd.clear();
  }

  // Player 1 (White) display
  lcd.setCursor(0, 0);
  lcd.print("W:");
  lcd.setCursor(0, 1);
  if (player1Minutes < 10) { //Add zeros before minutes to ensure that formatting is uniform
    lcd.print("0");
  }
  lcd.print(player1Minutes);
  lcd.print(":");
  if (player1Seconds < 10) {
    lcd.print("0");
  }
  lcd.print(player1Seconds);

  // Player 2 (Black) display
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
