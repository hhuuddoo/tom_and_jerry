## TOM AND JERRY CONSOLE GAME

In this game you take on the role as the mouse Jerry.

The goal is to collect cheese (C) while avoiding Tom (T) and the mousetraps that he places (M).
Once 5 pieces of cheese have been collected, a door (X) will open that allows you to progress to the next level.
It's game over if you run into Tom, or hit a mousetrap, 5 times.

# The header bar displays: 
* Your score
* Remaining lives
* Time played
* Number of 
	* cheese on screen
	* traps on screen
	* fireworks on screen
* Current level

Complete all the levels to win.

# Controls:
* W - Move Up
* A - Move Left
* S - Move Down
* D - Move Right
* F - Shoot fireworks (Only avaliable after 1st level)
* P - Toggle Pause
* L - Skip Level
* Q - Quit

# To run the game
1. Download the 'tom_and_jerry' executable & the 'levels' folder
2. Open cmd or terminal and run the following command:
```./tom_and_jerry levels/1.txt```

Note that several level text files can be entered into command(10 are provided).

To play all 10 levels, run the following command:
```./tom_and_jerry levels/1.txt levels/2.txt levels/3.txt levels/4.txt levels/5.txt levels/6.txt levels/7.txt levels/8.txt levels/9.txt levels/10.txt```

# Building Source Code
If you want build the game after modifying the source code, run the following command in the same directory as the 'tom_and_jerry.c' file, and the ZDK folder.
```gcc tom_and_jerry.c -std=gnu99 -IZDK -LZDK -lzdk -lncurses -o tom_and_jerry```
