# Tic-Tac-Toe

Written in embedded C for the microcontroller PIC16LF1937 with the help of the program Proteus 8, all comments on project are in romanian however.

The .c file is found in "PIC16LF1937" folder.

Functioning: At start-up you will be prompted with a menu. Your current choice is highlighted by blinking, upon choosing PvC, to fight vs the AI, you will choose to start with either X or 0. When it's the computer's turn to move, the screen might fill with X and 0s, this is not a glitch, the computer is just considering possibilities, when it's done computing, it will just pick a single spot and put its assigned symbol there. When an end state has been reached, the table will blink the line that won for several seconds, then it will return the game to the main menu.

Note: Due to the limited amount of computation of the microcontroller, the AI is also limited to a low difficulty

Layout is visible when launching the program in proteus however I will include a screenshot of it: "Layout.jpg".

Component list:

1   PIC16LF1937

9   14 segment LED

13  100 ohm resistances

12  10k ohm resistances

9   PNP transistors

3   NPN transistors

3   Buttons
