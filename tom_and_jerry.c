#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

//------------------ Control ------------------//
bool gameOver;	// Set this to true when the game is over
bool nextLevel; // Set this to true when the next level has been entered
bool quitGame;
int currLevel = 1; //  Current level in play

//------------------ Timers ------------------//
bool paused = false;			 // Set to true when game is paused
double startTime;					 // Games start time
double pauseStartTime = 0; // Time that the game was paused
double pauseDuration = 0;	// Duration of the pause
double currentTime;				 // Time when the current loop started

timer_id nextCheeseCountdown;		// Timer to determine when next cheese should be placed
timer_id nextTrapCountdown;			// Timer to determine when next trap should be placed
timer_id nextFireWorkCountdown; // Timer to  when next firework should be fired

#define DELAY 10 // Loop delay

//------------------ Players ------------------//
#define MAX_HEALTH 5 // Maximum player health
char currPlayer;		 // Holds the current player being used

//---- Tom ----//
double xTomInitial, yTomInitial, xTom, yTom, dxTom, dyTom;
int tHealth = MAX_HEALTH;
int tScore, tTotalScore;
#define TOM_IMG 'T'

//---- Jerry ----//
double xJerryInitial, yJerryInitial, xJerry, yJerry, dxJerry, dyJerry;
int jHealth = MAX_HEALTH;
int jScore;
#define JERRY_IMG 'J'

//------------------ Objects ------------------//

//---- Cheese ----//
typedef struct
{
	double xCheese;
	double yCheese;
} cheese;									 // Struct to hold the position of the cheese
cheese cheeseLocations[5]; // Holds all the cheese coordinates
int numberOfCheese;				 // Current number of cheese on the screen
int cheeseCollected;
#define CHEESE_IMG 'C'

//---- Door ----//
int xDoor, yDoor; // Door coordiantes
bool doorOpen;		// True is the door is open
#define DOOR_IMG 'X'

//---- Traps ----//
typedef struct
{
	double xMouse;
	double yMouse;
} mouseTrap;								// Struct to hold the position of the traps
mouseTrap trapLocations[5]; // Holds all the trap coordinates
int numberOfTraps;					// Current number of traps on the scrren
#define MOUSETRAP_IMG 'M'

typedef struct
{
	double x;
	double y;
	double dx;
	double dy;
} firework;											// Struct to hold position and direction of fireworks
firework fireworkLocations[10]; // Hold all fireworks
int numberOfFireworks;					// Current number of fireworks on the screen
#define FIREWORKS_IMG 'F'

//------------------ Settings ------------------//

//---- Screen ----//
int W, H; // Width and Height of the screen
const int statusBarHeight = 4;

//---- Wall ----//
typedef struct
{
	double x1;
	double x2;
	double y1;
	double y2;
} wall;						 // Struct to hold a walls coordiantes
wall walls[50];		 // Holds all the walls
int numberOfWalls; // Counter for the number of wall in the level
#define WALL_CH '*'

/**
 * Reads input of specified format from a given file
 */
void readData(FILE *stream)
{
	int currentWall = 0; // Counter for current wall being read

	// While stream has not reached the end of input
	while (!feof(stream))
	{
		char command;					 // Indicator which will determine what the rest of the line represents
		double x1, y1, x2, y2; // Position coordinates

		fscanf(stream, "%c", &command); // Get command from current line

		// If current line corresponds to Jerrys starting coordinates, store the values
		if (command == 'J')
			fscanf(stream, "%c %lf %lf", &command, &xJerryInitial, &yJerryInitial);
		// If current line corresponds to Toms starting coordinates, store the values
		else if (command == 'T')
			fscanf(stream, "%c %lf %lf", &command, &xTomInitial, &yTomInitial);
		// If current line corresponds to Wall coordinates, store the values
		else if (command == 'W')
		{
			fscanf(stream, "%c %lf %lf %lf %lf", &command, &x1, &y1, &x2, &y2);

			// Store walls coordinates in walls array
			walls[currentWall].x1 = x1;
			walls[currentWall].x2 = x2;
			walls[currentWall].y1 = y1;
			walls[currentWall].y2 = y2;

			++currentWall;
		}
	}
	numberOfWalls = currentWall; // Store number of walls
}

/**
 * Check whether two pairs of coordinates have collided
 * doubles - x1, y1, x2, y2
 * returns - bool true if a collision has happened
 */
bool collided(double x1, double y1, double x2, double y2)
{
	return round(x1) == round(x2) && round(y1) == round(y2);
}

/**
 * collidedWithCheese returns true if x and y coordinates collided with cheese
 */
bool collidedWithCheese(double x, double y)
{
	bool collision = false;
	for (int i = 0; i < numberOfCheese; ++i)
	{
		if (collided(round(x), round(y), cheeseLocations[i].xCheese, cheeseLocations[i].yCheese))
			collision = true;
	}
	return collision;
}

/**
 * collidedWithFirework returns true if x and y coordinates collided with a firework
 */
bool collidedWithFirework(double x, double y)
{
	bool collision = false;
	for (int i = 0; i < numberOfFireworks; ++i)
	{
		if (collided(round(x), round(y), fireworkLocations[i].x, fireworkLocations[i].y))
			collision = true;
	}
	return collision;
}

/**
 * collidedWithTrap returns true if x and y coordinates collided with trap
 */
bool collidedWithTrap(double x, double y)
{
	bool collision = false;
	for (int i = 0; i < numberOfTraps; ++i)
	{
		if (collided(round(x), round(y), trapLocations[i].xMouse, trapLocations[i].yMouse))
			collision = true;
	}
	return collision;
}

/**
 * Returns a random value between a specified min and max
 */
double randomRange(double min, double max)
{
	return (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

/**
 * sgn function takes a number and returns its sign
 */
int sgn(double num)
{
	if (num > 0)
		return 1;
	if (num < 0)
		return -1;
	return 0;
}

/**
 * Calculates a random direction
 */
void calculateDirection(double *dx, double *dy)
{
	double dir = rand() * M_PI * 2 / RAND_MAX; // Random value between 0 and 2pi

	// Determine random step size
	double max = 0.1;
	double min = 0.05;
	double step = randomRange(min, max);

	// Calculate direction
	*dx = step * cos(dir);
	*dy = step * sin(dir);
}

/**
 * totalDifference calculates the total difference between two points
 */
double totalDifference(double x1, double y1, double x2, double y2)
{
	double diff = fabs(x1 - x2) + fabs(y1 - y2);
	return diff;
}

/**
 * Calculates direction from a point to a destination
 */
void calculateDirectionToTarget(double currentX, double currentY, double targetX, double targetY, double *dx, double *dy, double step)
{
	// Calculate difference between coordiantes
	double yDiff = targetY - currentY;
	double xDiff = targetX - currentX;

	double angle = atan2(yDiff, xDiff); // Calculate angle between points

	// Calculate direction
	*dx = step * cos(angle);
	*dy = step * sin(angle);
}

/**
 * autoPlayerCollisions checks whether automatic moving players can move.
 * The function also handles players moving along walls
 */
void autoPlayerCollisions(double *x, double *y, double *dx, double *dy, double moveSpeed)
{
	int xNext = round(*x) + sgn(*dx) * ceil(fabs(*dx));
	int yNext = round(*y) + sgn(*dy) * ceil(fabs(*dy));
	char cx = scrape_char(xNext, round(*y)); // Get char in next position
	char cy = scrape_char(round(*x), yNext); // Get char in next position
	bool xHitWall = false, yHitWall = false; // Set to true if the player has hit a wall

	// If enemy is outside of bounds is heading into a wall, dont move in that direction
	if (xNext < 0 || xNext > W || cx == WALL_CH)
	{
		xHitWall = true;
		(*dx) = 0;
	}

	if (yNext < statusBarHeight || yNext > (H + statusBarHeight) || cy == WALL_CH)
	{
		yHitWall = true;
		(*dy) = 0;
	}

	// // If the next space is free, move accordingly
	if (yHitWall && !xHitWall)
		(*dx) = sgn(*dx) * moveSpeed;

	if (xHitWall && !yHitWall)
		(*dy) = sgn(*dy) * moveSpeed;
}

/**
 * calculateDirectionToClosestCheese finds the direction to the closest cheese on screen
 */
void calculateDirectionToClosestCheese(double x, double y, double *dx, double *dy, double moveSpeed)
{
	int closestCheeseIndex = 0;
	double distToCheese = __DBL_MAX__;

	for (int i = 0; i < numberOfCheese; ++i)
	{
		double tempDist = totalDifference(x, y, cheeseLocations[i].xCheese, cheeseLocations[i].yCheese);
		if (tempDist < distToCheese)
		{
			distToCheese = tempDist;
			closestCheeseIndex = i;
		}
	}
	calculateDirectionToTarget(x, y, cheeseLocations[closestCheeseIndex].xCheese, cheeseLocations[closestCheeseIndex].yCheese, dx, dy, moveSpeed);
}

/**
 * Draws the walls defined in the walls array
 */
void drawWalls()
{
	// Run for each wall in the walls array
	for (int i = 0; i < numberOfWalls; ++i)
	{
		// Scale coordinates
		int x1 = round(walls[i].x1 * W);
		int x2 = round(walls[i].x2 * W);
		int y1 = round(walls[i].y1 * H) + statusBarHeight;
		int y2 = round(walls[i].y2 * H) + statusBarHeight;

		draw_line(x1, y1, x2, y2, WALL_CH); // Draw wall
	}
}

/**
 * gameOver displays a game over screen with a variable message
 */
void gameOverScreen(char *message[])
{
	clear_screen();

	const int rows = 2;

	for (int i = 0; i < rows; i++)
	{
		// Draw message in middle of screen.
		int len = strlen(message[i]);
		int x = (W - len) / 2;
		int y = (H - rows) / 2 + i;
		draw_formatted(x, y + statusBarHeight, message[i]);
	}
	show_screen();
}

/**
 * Draws status bar in the top 3 rows of the screen
 */
void drawStatus()
{
	int grid = round(W / 5); // Grid for stats to be placed in

	// Convert time into minutes and seconds
	double currentTime = paused ? (pauseStartTime - startTime) : (get_current_time() - startTime);
	int minutes = floor(currentTime / 60);
	int seconds = currentTime - (minutes * 60);

	int score = currPlayer == 'J' ? jScore : tTotalScore;
	int lives = currPlayer == 'J' ? jHealth : tHealth;

	// Draw first row
	draw_string(0, 0, "n10219641");
	draw_formatted(grid, 0, "Score: %d", score);
	draw_formatted(grid * 2, 0, "Lives: %d", lives);
	draw_formatted(grid * 3, 0, "Player: %c", currPlayer);
	draw_formatted(grid * 4, 0, "Time: %02d:%02d", minutes, seconds);

	// Draw second row
	draw_formatted(0, 2, "Cheese: %d", numberOfCheese);
	draw_formatted(grid, 2, "Traps: %d", numberOfTraps);
	draw_formatted(grid * 2, 2, "Fireworks: %d", numberOfFireworks);
	draw_formatted(grid * 3, 2, "Level: %d", currLevel);

	// Draw seperator
	draw_line(0, 3, W, 3, '-');
}

/**
 * Draws the door
 */
void drawDoor()
{
	draw_char(round(xDoor), round(yDoor), DOOR_IMG);
}

/**
 * Draws the current positions of generated cheese
 */
void drawCheese()
{
	for (int i = 0; i < numberOfCheese; ++i)
	{
		draw_char(round(cheeseLocations[i].xCheese), round(cheeseLocations[i].yCheese), CHEESE_IMG);
	}
}

/**
 * Draws the current positions of generated traps
 */
void drawTrap()
{
	for (int i = 0; i < numberOfTraps; ++i)
	{
		draw_char(round(trapLocations[i].xMouse), round(trapLocations[i].yMouse), MOUSETRAP_IMG);
	}
}

/**
 * Draws the current positions of generated fireworks
 */
void drawFireworks()
{
	for (int i = 0; i < numberOfFireworks; ++i)
	{
		draw_char(round(fireworkLocations[i].x), round(fireworkLocations[i].y), FIREWORKS_IMG);
	}
}

/**
 * Draws the current position of Tom and Jerry
 */
void drawPlayers()
{
	draw_char(round(xTom), round(yTom), TOM_IMG);
	draw_char(round(xJerry), round(yJerry), JERRY_IMG);
}

/**
 * Setup jerry's variables
 */
void setupJerry()
{
	xJerry = (xJerryInitial * W);
	yJerry = (yJerryInitial * H) + statusBarHeight;
}

/**
 * Setup Tom's variables and calculate direction of travel
 */
void setupTom()
{
	xTom = (xTomInitial * W);
	yTom = (yTomInitial * H) + statusBarHeight;

	calculateDirection(&dxTom, &dyTom);
}

/**
 * Moves current player based on keyboard input
 */
void movePlayer(int key, double *x, double *y)
{
	// Store rounded coordiantes
	int xPos = round(*x);
	int yPos = round(*y);

	// If a is pressed and player can move left, move left
	if (key == 'a' && xPos > 0)
	{
		char c = scrape_char(xPos - 1, yPos);
		if (c != WALL_CH)
			(*x)--;
	}
	// If d is pressed and player can move right, move right
	else if (key == 'd' && xPos < W)
	{
		char c = scrape_char(xPos + 1, yPos);
		if (c != WALL_CH)
			(*x)++;
	}
	// If s is pressed and player can move down, move down
	else if (key == 's' && yPos < (H + statusBarHeight))
	{
		char c = scrape_char(xPos, yPos + 1);
		if (c != WALL_CH)
			(*y)++;
	}
	// If w is pressed and player can move up, move up
	else if (key == 'w' && yPos > statusBarHeight)
	{
		char c = scrape_char(xPos, yPos - 1);
		if (c != WALL_CH)
			(*y)--;
	}
}

/**
 * getFurthestPositionIndex finds the best direction for character1 to move to get away from character2
 * this function returns the index of the best way to move
 * 0 - up
 * 1 - left
 * 2 - down
 * 3 - right
 */
int getFurthestPositionIndex(double x1, double y1, double x2, double y2, double *bestPos)
{
	(*bestPos) = -__DBL_MAX__;
	int bestPosIndex;

	// Find distance from coordinates when coord 1 moves in all directions
	double upDist = totalDifference(x1, y1 - 1, x2, y2);
	double downDist = totalDifference(x1, y1 + 1, x2, y2);
	double leftDist = totalDifference(x1 - 1, y1, x2, y2);
	double rightDist = totalDifference(x1 + 1, y1, x2, y2);

	// Check if any moves have left the screen or hit a wall
	bool uOut = y1 - 1 < statusBarHeight || scrape_char(x1, y1 - 1) == WALL_CH;
	bool dOut = y1 + 1 > (H + statusBarHeight) || scrape_char(x1, y1 + 1) == WALL_CH;
	bool lOut = x1 - 1 < 0 || scrape_char(x1 - 1, y1) == WALL_CH;
	bool rOut = x1 + 1 > W || scrape_char(x1 + 1, y1) == WALL_CH;

	// Store values in arrays
	double moveDists[4] = {upDist, leftDist, downDist, rightDist};
	bool moveChars[4] = {uOut, lOut, dOut, rOut};

	// Get largest distance from Tom and store index in bestPosIndex
	for (int i = 0; i < 4; ++i)
	{
		double tempPos = moveDists[i];
		bool isOut = moveChars[i];
		if (tempPos > (*bestPos) && !isOut) // If current position is greater than the best position and is on screen
		{
			(*bestPos) = tempPos; // Update best position
			bestPosIndex = i;
		}
	}
	return bestPosIndex;
}

/**
 * evadeTom calculates and moves Jerry away from Tom and returns false is jerry will not evade tom
 */
bool evadeTom(double *dx, double *dy, double jerryMoveSpeed)
{
	double bestPos; // Holds the distance of the furthest position

	int bestPosIndex = getFurthestPositionIndex(xJerry, yJerry, xTom, yTom, &bestPos); // Get best direction to move in

	// If Tom is close
	if (bestPos < 10)
	{
		// Move Up
		if (bestPosIndex == 0)
			(*dy) = -jerryMoveSpeed;
		// Move Left
		else if (bestPosIndex == 1)
			(*dx) = -jerryMoveSpeed;
		// Move Down
		else if (bestPosIndex == 2)
			(*dy) = jerryMoveSpeed;
		// Move Right
		else if (bestPosIndex == 3)
			(*dx) = jerryMoveSpeed;
		return true; // Jerry will evade
	}
	else
	{
		return false; // Jerry will not evade
	}
}

/**
 * jerryAutoMove controls jerrys automatic movement, and determines whether to run from tom or to seek cheese
 */
void jerryAutoMove(double *x, double *y)
{
	// Set move speeds
	double jerryCheeseSeekSpeed = 0.05;
	double jerryEvadeSpeed = 0.07;
	bool evade; // Set to true if Jerry will run from Tom

	evade = evadeTom(&dxJerry, &dyJerry, jerryEvadeSpeed);

	// Get direction to closest cheese if Tom is far away
	if (!evade)
	{
		calculateDirectionToClosestCheese(xJerry, yJerry, &dxJerry, &dyJerry, jerryCheeseSeekSpeed);
	}

	// Check collisions
	autoPlayerCollisions(x, y, &dxJerry, &dyJerry, (evade ? jerryEvadeSpeed : jerryCheeseSeekSpeed));

	// Move Jerry
	(*x) += dxJerry;
	(*y) += dyJerry;
}

/**
 * chaseJerry calculates and moves Tom towards Jerry
 */
void chaseJerry(double *x, double *y, double *dx, double *dy)
{
	double tomMoveSpeed = 0.05;																										// Move speed
	calculateDirectionToTarget((*x), (*y), xJerry, yJerry, dx, dy, tomMoveSpeed); // Get direction to jerry
	autoPlayerCollisions(x, y, dx, dy, tomMoveSpeed);															// Check for collisions

	// Move Tom
	(*x) += (*dx);
	(*y) += (*dy);
}

/**
 * createDoor creates a door if appropriate score is met and one doesnt exist
 */
void createDoor()
{
	// Get random x and y position within bounds and character at that position
	int x = round(randomRange(0, W));
	int y = round(randomRange(statusBarHeight, statusBarHeight + H));
	char c = scrape_char(x, y);

	// While x and y collide with another game object
	while (c != ' ')
	{
		// Regenerate x and y coords, and get character at the position
		x = round(randomRange(0, W));
		y = round(randomRange(statusBarHeight, statusBarHeight + H));
		c = scrape_char(x, y);
	}
	xDoor = x;
	yDoor = y;
	doorOpen = true;
}

/**
 * updateDoor places the door and handles collisions between objects and doors
 */
void updateDoor()
{
	// Create door if one does not exist
	if (!doorOpen)
		createDoor();

	// Check if current player touches door
	if (currPlayer == 'J')
		nextLevel = collided(xDoor, yDoor, xJerry, yJerry);
	else
		nextLevel = collided(xDoor, yDoor, xTom, yTom);
}

/**
 * autoCheesePlacement handles automated cheese placement
 */
void autoCheesePlacement()
{
	// Check if timer has finished, place cheese
	if (timer_expired(nextCheeseCountdown))
	{
		// Get random x and y position within bounds and character at that position
		int x = round(randomRange(0, W));
		int y = round(randomRange(statusBarHeight, statusBarHeight + H));
		char c = scrape_char(x, y);

		// While x and y collide with another game object
		while (c != ' ')
		{
			// Regenerate x and y coords, and get character at the position
			x = round(randomRange(0, W));
			y = round(randomRange(statusBarHeight, statusBarHeight + H));
			c = scrape_char(x, y);
		}

		// Add cheese coords into cheese location array
		cheeseLocations[numberOfCheese].xCheese = x;
		cheeseLocations[numberOfCheese].yCheese = y;

		++numberOfCheese;
		timer_reset(nextCheeseCountdown);
	}
}

/**
 * manualCheesePlacement handles automated cheese placement
 */
void manualCheesePlacement()
{
	// Place cheese
	int x = round(xTom);
	int y = round(yTom);
	bool canPlaceCheese = true;

	// Check collisions with objects
	canPlaceCheese = !(collidedWithCheese(x, y) || !canPlaceCheese);
	canPlaceCheese = !(collidedWithTrap(x, y) || !canPlaceCheese);
	canPlaceCheese = !(collided(x, y, xJerry, yJerry) || !canPlaceCheese);

	if (canPlaceCheese)
	{
		// Add cheese coords into cheese location array
		cheeseLocations[numberOfCheese].xCheese = x;
		cheeseLocations[numberOfCheese].yCheese = y;
		++numberOfCheese;
	}
}

/**
 * Function that handles manual and automatic placement of cheese
 */
void updateCheese(int key)
{
	// Check if Jerry has collided with cheese, and if so, increment score
	for (int i = 0; i < numberOfCheese; ++i)
	{
		if (collided(xJerry, yJerry, cheeseLocations[i].xCheese, cheeseLocations[i].yCheese))
		{
			// If a collision has occured, remove cheese location from array
			for (int j = i; j < numberOfCheese - 1; ++j)
			{
				cheeseLocations[j] = cheeseLocations[j + 1];
			}

			if (numberOfCheese == 5)
				timer_reset(nextCheeseCountdown);
			--numberOfCheese;
			if (currPlayer == 'J')
			{
				++cheeseCollected;
				++jScore;
			}
		}
	}

	// If there is less than 5 cheese and the game isn't paused, place cheese
	if (numberOfCheese < 5 && !paused && currPlayer != 'T')
		autoCheesePlacement();
	if (numberOfCheese < 5 && currPlayer == 'T' && key == 'c')
		manualCheesePlacement();
}

/**
 * autoTrapPlacement handles automated cheese placement
 */
void autoTrapPlacement()
{
	// Check if timer has finished
	if (timer_expired(nextTrapCountdown))
	{
		int x = round(xTom);
		int y = round(yTom);
		bool canPlaceTrap = true;

		// Check collisions with objects
		canPlaceTrap = !(collidedWithCheese(x, y) || !canPlaceTrap);
		canPlaceTrap = !(collidedWithTrap(x, y) || !canPlaceTrap);
		canPlaceTrap = !(collided(x, y, xJerry, yJerry) || !canPlaceTrap);

		// If there is a free space, place trap
		if (canPlaceTrap)
		{
			trapLocations[numberOfTraps].xMouse = x;
			trapLocations[numberOfTraps].yMouse = y;

			++numberOfTraps;
			timer_reset(nextTrapCountdown);
		}
	}
}

/**
 * manualTrapPlacement handles automated trap placement
 */
void manualTrapPlacement()
{
	// Place cheese
	int x = round(xTom);
	int y = round(yTom);
	bool canPlaceTrap = true;

	// Check collisions with objects
	canPlaceTrap = !(collidedWithCheese(x, y) || !canPlaceTrap);
	canPlaceTrap = !(collidedWithTrap(x, y) || !canPlaceTrap);
	canPlaceTrap = !(collided(x, y, xJerry, yJerry) || !canPlaceTrap);

	if (canPlaceTrap)
	{
		// Add cheese coords into cheese location array
		trapLocations[numberOfTraps].xMouse = x;
		trapLocations[numberOfTraps].yMouse = y;
		++numberOfTraps;
	}
}

/**
 * Function that handles manual and automatic placement of mouse traps
 */
void updateTrap(char key)
{
	// Check if Jerry has collided with trap, and if so, decrease life or increase toms score
	for (int i = 0; i < numberOfTraps; ++i)
	{
		if (collided(xJerry, yJerry, trapLocations[i].xMouse, trapLocations[i].yMouse))
		{
			// If a collision has occured, remove trap location from array
			for (int j = i; j < numberOfTraps - 1; ++j)
			{
				trapLocations[j] = trapLocations[j + 1];
			}
			if (numberOfTraps == 5)
				timer_reset(nextTrapCountdown);
			--numberOfTraps;
			if (currPlayer == 'J')
				--jHealth;
			else
			{
				++tScore;
				++tTotalScore;
			}
		}
	}

	// If there is less than 5 traps and the game isn't paused, place trap
	if (numberOfTraps < 5 && !paused && currPlayer != 'T')
		autoTrapPlacement();
	if (numberOfTraps < 5 && currPlayer == 'T' && key == 'm')
		manualTrapPlacement();
}

/**
 * createFireworks creates fire works
 */
void createFireworks()
{
	if (!collidedWithFirework(xJerry, yJerry))
	{
		// Create firework at jerry position
		fireworkLocations[numberOfFireworks].x = xJerry;
		fireworkLocations[numberOfFireworks].y = yJerry;

		// Calculate direction for firework
		calculateDirectionToTarget(fireworkLocations[numberOfFireworks].x, fireworkLocations[numberOfFireworks].y, xTom, yTom, &fireworkLocations[numberOfFireworks].dx, &fireworkLocations[numberOfFireworks].dy, 0.11);
		++numberOfFireworks;
	}
}

/**
 * moveFireworks moves existing fireworks in direction
 * i - firework index
 */
void moveFireworks(int i)
{
	// Initialse firework speeds
	double autoFireworkSpeed = 0.04;
	double manualFireworkSpeed = 0.11;

	// Change firework speed if Jerry is not being controlled
	double moveSpeed = currPlayer == 'J' ? manualFireworkSpeed : autoFireworkSpeed;

	// Move each firework based on calculated direction and recalculate direction to tom
	fireworkLocations[i].x += fireworkLocations[i].dx;
	fireworkLocations[i].y += fireworkLocations[i].dy;
	calculateDirectionToTarget(fireworkLocations[i].x, fireworkLocations[i].y, xTom, yTom, &fireworkLocations[i].dx, &fireworkLocations[i].dy, moveSpeed);
}

/**
 * updateFireworks handles collisions, and movement of fireworks
 */
void updateFireworks(char key)
{
	for (int i = 0; i < numberOfFireworks; ++i)
	{
		// Next position for current firework
		int xNext = round(fireworkLocations[i].x + fireworkLocations[i].dx);
		int yNext = round(fireworkLocations[i].y + fireworkLocations[i].dy);

		char c = scrape_char(xNext, yNext); // Get char in next position

		if (c == WALL_CH)
		{
			// Remove from array
			for (int j = i; j < numberOfFireworks - 1; ++j)
			{
				fireworkLocations[j] = fireworkLocations[j + 1];
			}

			if (numberOfFireworks == 10)
				timer_reset(nextFireWorkCountdown);

			--numberOfFireworks;
		}
		else if (c == TOM_IMG)
		{
			// Remove from array
			for (int j = i; j < numberOfFireworks - 1; ++j)
			{
				fireworkLocations[j] = fireworkLocations[j + 1];
			}

			if (numberOfFireworks == 10)
				timer_reset(nextFireWorkCountdown);
			--numberOfFireworks;

			// Reset Toms position
			setupTom();

			if (currPlayer == 'J')
				++jScore;
			else
				--tHealth;
		}
		else
		{
			// Move firework if there is no collision
			moveFireworks(i);
		}
	}

	// If there is less than 5 traps and the game isn't paused, place trap
	if (numberOfFireworks < 10 && !paused && currPlayer != 'J' && timer_expired(nextFireWorkCountdown))
	{
		createFireworks();
		timer_reset(nextFireWorkCountdown);
	}
	if (numberOfFireworks < 10 && currPlayer == 'J' && key == 'f')
		createFireworks();
}

/**
 * Controls all process which occur when a key has been pressed
 */
void checkControlKeyPresses(char key)
{
	// If q is pressed, quit the game
	if (key == 'q')
	{
		quitGame = true;
		return;
	}
	// Else if z is pressed, switch characters
	else if (key == 'z' && currLevel > 1)
	{
		currPlayer = currPlayer == 'J' ? 'T' : 'J';
	}
	// Else if p is pressed, pause the game
	else if (key == 'p')
	{
		paused = !paused;										 // Toggle pause
		pauseStartTime = get_current_time(); // Set pause time
		// If the game has been unpaused, add the pause duration to the startTime
		if (!paused)
		{
			// Compensate for pause time
			startTime += pauseDuration;
			(*nextCheeseCountdown).reset_time += pauseDuration;
			(*nextTrapCountdown).reset_time += pauseDuration;
			(*nextFireWorkCountdown).reset_time += pauseDuration;
		}
	}
	// Else if l is pressed, skip to next level
	else if (key == 'l')
	{
		nextLevel = true;
		return;
	}
}

/**
 * Updates lives and scores when Tom and Jerry collide
 */
void collisionBetweenPlayers()
{
	// If there's a collision between tom and jerry, reset coords and modify appropriate stats
	if (collided(xJerry, yJerry, xTom, yTom))
	{
		setupJerry();
		setupTom();
		if (currPlayer == 'J')
			--jHealth;
		else
		{
			tScore += 5;
			tTotalScore += 5;
		}
	}
}

/**
 * Updates the players position which is currently being played as
 */
void moveCurrentPlayer(char key)
{
	// If Tom is being played, move Tom
	if (currPlayer == 'T')
	{
		movePlayer(key, &xTom, &yTom);
	}
	// Else if Jerry is being played, move Jerry
	else if (currPlayer == 'J')
	{
		movePlayer(key, &xJerry, &yJerry);
	}
}

/**
 * Handles all process which occur when the game is paused xor unpaused
 */
void gamePausedControl()
{
	// Run processes which depend on the game being paused
	if (paused)
	{
		pauseDuration = (get_current_time() - pauseStartTime); // Store paused duration
	}
	// Run processes which depend on the game being unpaused
	else
	{
		if (currPlayer != 'T')
			chaseJerry(&xTom, &yTom, &dxTom, &dyTom); // Move unplayed character
		else
			jerryAutoMove(&xJerry, &yJerry);
	}
}

/**
 * Renders all objects drawn to the screen
 */
void drawAll()
{
	clear_screen();
	drawWalls();
	drawTrap();
	drawCheese();
	drawFireworks();
	drawPlayers();
	if (doorOpen)
		drawDoor();
	drawStatus();
	show_screen();
}

/**
 * startSetup similar to game setup however, should only be run once at the start of the game.
 */
void startSetup()
{
	// Initialise health
	tHealth = MAX_HEALTH;
	jHealth = MAX_HEALTH;

	// Initialise Score
	jScore = 0;
	tTotalScore = 0;

	// Initialise pausing
	pauseDuration = 0;

	// Initialise start time
	startTime = get_current_time();
}

/**
 * Game setup
 */
void setup()
{
	srand(get_current_time()); // Initialise randomisation

	// Set screen dimentions
	W = screen_width() - 1;
	H = screen_height() - (1 + statusBarHeight);

	// Initialise control variables
	gameOver = false;
	nextLevel = false;
	quitGame = false;
	doorOpen = false;
	paused = false;

	// Initialise counter variables
	numberOfCheese = 0;
	numberOfTraps = 0;
	numberOfFireworks = 0;
	cheeseCollected = 0;
	tScore = 0;

	// Initialise timers
	nextCheeseCountdown = create_timer(2000);
	nextTrapCountdown = create_timer(3000);
	nextFireWorkCountdown = create_timer(5000);

	// Setup players
	currPlayer = 'J';

	setupJerry();
	setupTom();
}

/**
 * Function which constantly runs
 */
void loop()
{
	int key = get_char(); // Get user keypress

	// Updated processes
	checkControlKeyPresses(key);
	collisionBetweenPlayers();
	moveCurrentPlayer(key);
	gamePausedControl();
	updateTrap(key);
	updateCheese(key);

	if (currLevel > 1)
		updateFireworks(key); // Move fireworks

	// If either Tom or Jerry have lost their health, end the game
	if (jHealth <= 0 || tHealth <= 0)
	{
		gameOver = true;
		return;
	}

	// If Jerry has collected five or more cheese, open the door
	if (cheeseCollected >= 5 || tScore >= 5)
		updateDoor();
}

/** 
 * Opens and reads a file
 */
void openFile(char *args[], int i)
{
	FILE *stream = fopen(args[i], "r"); // Open file
	currLevel = i;

	// If the file exists, read data
	if (stream != NULL)
	{
		readData(stream);
		fclose(stream);
	}
}

/**
 * Checks keypresses when on game over screen and applies accordingly
 */
void handleEndScreenKeypress(int *i, int argc)
{
	// Wait for keypress
	char c = wait_char();

	while (1)
	{
		// If r is pressed, restart game and reset variables
		if (c == 'r')
		{
			(*i) = 0;
			startSetup();
			break;
		}
		// Else if r is pressed, quit game
		else if (c == 'q')
		{
			(*i) = argc;
			break;
		}
		// Else wait for valid keypress
		else
		{
			c = wait_char();
		}
	}
}

/**
 * Function runs at beginning of execution
 * int argc - number of arguments
 * char **args - string value of entered arguments
 */
int main(int argc, char *args[])
{
	// Run initial setup
	startSetup();

	// For loop for each entered level argument
	for (int i = 1; i < argc; ++i)
	{
		openFile(args, i);

		setup_screen(); // Initialise game screen
		setup();

		while (!gameOver && !nextLevel && !quitGame)
		{
			currentTime = get_current_time();
			drawAll();
			loop();
			timer_pause(DELAY);
		}

		// Reset screen
		clear_screen();
		show_screen();

		// If the player has died or quit
		if (gameOver)
		{
			// Show losing game over screen
			char *message[] = {"GAME OVER! YOU LOSE!", "Press R to restart or Q to quit."};
			gameOverScreen(message);
			handleEndScreenKeypress(&i, argc);
		}

		// Quit game if q was pressed
		if (quitGame)
		{
			i = argc;
			break;
		}

		// If the end of the game has been reached
		if (nextLevel && currLevel >= (argc - 1))
		{
			// Show winning game over screen
			char *message[] = {"CONGRATULATIONS! YOU WIN!", "Press R to restart or Q to quit."};
			gameOverScreen(message);
			handleEndScreenKeypress(&i, argc);
		}
	}
	return 0;
}
