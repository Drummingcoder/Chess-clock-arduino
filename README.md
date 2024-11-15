# Arduino Uno Chess Clock

The first version of my code (code.ino) is made for a chess clock with the following design: 
https://www.tinkercad.com/things/izdj3lSTSRr-my-chess-clock

The second version of my code (revised_code.ino) is made for a different chess clock with the following design:
https://www.tinkercad.com/things/9GNWRFxF56N-my-chess-clock-v20

*As of 11/14/2024, both projects are still pending approval to become public, so the links don't work right now.*

TLDR Features: 
- Separate times for Players 1 and 2
- Increment time control
- Can store previous time setting
- The difference between v1.0 and v2.0 is v2.0 has two additional displays to show Player 1 and 2 times separately

Things to add:
- Ability to reset the chess clock after timing out (and also the option to play again with the same settings).
- A beep every time a turn is made?
- Ability to add or subtract time during a game while in Pause menu

This chess clock allows you to set separate times for players 1 and 2 (although you can leave the times for player 2 blank if you want to have players 1 and 2 the same). 
Use the player 1 button to decrement, the player 2 button to increment, and the Set button to set that time setting. You can also set the increment time (in seconds) 
after setting player 1 and player 2's time. When the game is running, the Player 1 and Player 2 buttons will swap turns from black to white and vice versa, and the Set
button will pause and unpause the game. There's also a Reset button (not coded, but included in both designs) that will reset the chess clock back from the start. 
After a player times out, the display will change to show who has timed out and the piezo buzzer will beep until the chess clock is reset or disconnected from power.
This chess clock also can store the previous game setting and will restore those settings the next time you play. The difference between v1.0 and v2.0 is that on v2.0,
two additional 4-digit 7-segment displays will show the times for Player 1 and 2 separately, while this information is put onto one 16 X 2 LCD on v1.0.

Limits: On v1.0, there is a limit of 999 minutes and 59 seconds, while on v2.0, there is a limit of 99 minutes and 59 seconds. The increment limit (in seconds)
for both versions is 60 seconds.
