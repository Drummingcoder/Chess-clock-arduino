# Arduino Uno Chess Clock

The first version (v1.0) of my code (code.ino) is made for a chess clock with the following design: 
https://www.tinkercad.com/things/izdj3lSTSRr-my-chess-clock

The second version (v2.0) of my code (revised_code.ino) is made for a different chess clock with the following design:
https://www.tinkercad.com/things/9GNWRFxF56N-my-chess-clock-v20

TLDR Features: 
- Separate times for Players 1 and 2
- Increment time control
- Can store previous time setting
- The difference between v1.0 and v2.0 is that v2.0 has two additional displays to show Player 1 and 2 times separately

Things to add:
- Add a centisecond display under 20 secs
- Blinking minutes or seconds when setting up
- Ending buzzer melody?
- Better time handling using millis(), using delay() for now

Added: 
- Option to use previous time settings or not when setting up (Player 1 (white) or left button for YES, Player 2 (Black) or right button for NO)
  - Option to use previous game score or not is separate. If not used and new chess game is started and ended, previous game scores will be overwritten.
- When using previous time settings, automatically skip to start game screen
- Hexadecimal minutes up to 12, for 120 minutes (v2.0 only)
- Start the timer (when the game starts) only when player 1 presses their button
- Ability to reset the chess clock after timing out and also the option to play again with the same settings (just use the Reset button)
- A beep every time a turn is made
- Better colon blinking during game
- Games counter (EEPROM storage)
- Increment minutes by 5 after 15 mins, by 10 after 30, by 5 after 40, and by 10 after 50
- Increment the increment by 5 after 15 secs
- Option to turn beep off (Button 1 for YES to turn beep off, Button 2 for NO to leave beep on)
- Welcome Screen
- 1 minute and 10 seconds warning
- Time difference on LED display (v2.0 only)
- Ability to add or subtract time during a game while in the Pause menu (Press Button 1 while the game is paused)
- End game early using Pause menu and increment game counter accordingly (Press Button 2 while the game is paused) (in progress)

This chess clock allows you to set separate times for players 1 and 2 (although you can leave the times for player 2 blank if you want to have players 1 and 2 the same). Leaving player 1's time blank will set the time control to be 10 minutes (player 2's time can be set separately or not).
When starting the chess clock and setting the time controls, use the player 1 button to decrement, the player 2 button to increment, and the Set button to set that time setting. You can also set the increment time (in seconds) 
after setting player 1 and player 2's time. When the game is running, the Player 1 and Player 2 buttons will swap turns from black to white and vice versa, and the Set
button will pause and unpause the game. There's also a Reset button (not coded, but included in both designs) that will reset the chess clock back from the start. The clock counts in centiseconds for high accuracy, but centiseconds are not displayed.
After a player times out, the display will change to show who has timed out and the piezo buzzer will beep until the chess clock is reset or disconnected from power.
This chess clock also can store the previous game setting and will restore those settings the next time you play (the first time you use it, though, will simply show zeros for all settings). The difference between v1.0 and v2.0 is that on v2.0,
two additional 4-digit 7-segment displays will show the times for Player 1 and 2 separately, while this information is put onto one 16 X 2 LCD on v1.0.

Limits: On v1.0, there is a limit of 999 minutes and 59 seconds, while on v2.0, there is a limit of 120 minutes and 59 seconds. The increment limit (in seconds)
for both versions is 60 seconds.

When multiple buttons are pressed, the player 1 button has first priority, then the player 2 button, then the Pause/Set button.
