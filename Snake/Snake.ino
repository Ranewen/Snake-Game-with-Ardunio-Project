#include "LedControl.h"

struct Point     // This struct holds the row and column of any LED
{   
	int row = 0, col = 0;   // Default row and column are 0
  int x = 0, y = 0;
	Point(int row = 0, int col = 0, int x = 0, int y = 0): row(row), col(col), x(x), y(y) {}
};

const short potentiometer = A0; // Connects the potentiometer for snake speed to pin A0 
const short JoystickX_axis = A1;  // Connects the joystick's x-axis to pin A1
const short JoystickY_axis = A2;  // Connects the joystick's x-axis to pin A2

const short CLK = 10;   // connects the clock for LED matrix to 10
const short CS  = 11;  // connects the CS for LED matrix to 11
const short DIN = 12; // connects the DIN for LED matrix to 12
LedControl matrix(DIN,CLK,CS,1);

int snakeLen = 3; // Holds the length of the snake. Initial length of the snake is 3.
int snakeSpeed = 1; // Holds the speed of the snake. Initial speed of the snake is 1.
int snakeDirection = 0; // Holds the direction of the snake. Initial direction none.

bool isGameOver = false;  // Keeps track of whether the game is over or not.

int Board[8][8] = {};   // Represents 8x8 LED
Point Snake;
Point Food(-1, -1);
Point Joystick(0, 0, 500, 500);

void setup()
{
	Serial.begin(115200);
  
  // Sets initial values for led and snake
	matrix.shutdown(0, false);
	matrix.setIntensity(0,8);
	matrix.clearDisplay(0);
	randomSeed(analogRead(A5));
	Snake.row = random(8);
	Snake.col = random(8);

  // Here calibrates the joystick
	int x = 0, y = 0;

	for (int i = 0; i < 10; i++)
  {
		x += analogRead(JoystickX_axis);
		y += analogRead(JoystickY_axis);
	}

	Joystick.x = x / 10;
	Joystick.y = y / 10;
}

void loop()
{
	ProduceFood();    // This function produces food
	ScanJoystick();    // This function scans the movement of the joystick
	GetNewSnake();     // Updates the new snake after receiving the command from the joystick
	IsTheGameOver();   // Checks if the game is over
}

void IsTheGameOver()
{
	if (isGameOver)   // If the game is over
  {
		matrix.setLed(0, Food.row, Food.col, 0);   // Resets food on led

    // Resets snake on led
    for (int i = 1; i <= snakeLen; i++)
    {
      for (int row = 0; row < 8; row++)
      {
        for (int col = 0; col < 8; col++)
        {
          if (Board[row][col] == i)
          {
            matrix.setLed(0, row, col, 0);
            delay(100);
          }
        }
      }
    }

    // Prints a cross on the screen to indicate to the user that they have lost the game
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
          if (i == j || i == 7 - j)
              matrix.setLed(0, i, j, 1);
          else
              matrix.setLed(0, i, j, 0);
      }
    }
    delay(1000);

    ScorePoint(snakeLen - 3);    // Prints the user's score on the screen

    // Resets the game
		isGameOver = false;
		Snake.row = random(8);
		Snake.col = random(8);
		Food.row = -1;
		Food.col = -1;
		snakeLen = 3;
		snakeDirection = 0;
		memset(Board, 0, sizeof(Board[0][0]) * 8 * 8);
		matrix.clearDisplay(0);
	}
}

void GetNewSnake()
{
	switch (snakeDirection)
  {
    case 1:   // If direction is 1, i.e. up
			Snake.row--;
			FixOvershoots();    // This function fixes the snake on the 8x8 LED exceeding the LED.
			matrix.setLed(0, Snake.row, Snake.col, 1);
			break;

		case 2:   // If direction is 2, i.e. right
			Snake.col++;
			FixOvershoots();
			matrix.setLed(0, Snake.row, Snake.col, 1);
			break;

		case 3:   // If direction is 3, i.e. down
			Snake.row++;
			FixOvershoots();
			matrix.setLed(0, Snake.row, Snake.col, 1);
			break;

		case 4:   // If direction is 4, i.e. left
			Snake.col--;  
			FixOvershoots();
			matrix.setLed(0, Snake.row, Snake.col, 1);
			break;

		default:  // if the snake is not moving
			return;
	}

	if (Board[Snake.row][Snake.col] > 1 && snakeDirection != 0)     // If the snake hits itself, the game is over.
  {
		isGameOver = true;
		return;
	}

	if (Snake.row == Food.row && Snake.col == Food.col)   // If the snake eats the food
  {
    // Resets food
		Food.row = -1; 
		Food.col = -1;

		// Increments snake length
		snakeLen++;

		// increment all the snake body segments
		for (int row = 0; row < 8; row++)
    {
			for (int col = 0; col < 8; col++)
      {
				if (Board[row][col] > 0 )
        {
					Board[row][col]++;
				}
			}
		}
	}

	Board[Snake.row][Snake.col] = snakeLen + 1;

	// Decrements all the snake body segments
	for (int row = 0; row < 8; row++)
  {
		for (int col = 0; col < 8; col++)
    {
			if (Board[row][col] > 0 )
      {
				Board[row][col]--;
			}
			matrix.setLed(0, row, col, Board[row][col] == 0 ? 0 : 1);
		}
	}
}

// This function fixes the snake on the 8x8 LED exceeding the LED.
void FixOvershoots()
{
  if (Snake.col < 0)
  {
    Snake.col += 8;
  }
  if (Snake.col > 7)
  {
    Snake.col -= 8;
  }
  if (Snake.row < 0)
  {
    Snake.row += 8;
  }
  if (Snake.row > 7)
  {
    Snake.row -= 8;
  }
}

void ProduceFood()
{
	if (Food.row == -1 || Food.col == -1)   // If there is no food on the board, it produces a food
  {
		if (snakeLen >= 64)   // If the length of the snake is greater than 64, that is, if all 8x8 LEDs are filled, it will not produce food because the game is already over.
    {
			return;
		}

		do   // It produces food until it finds an empty spot on the board.
    {
			Food.col = random(8);
			Food.row = random(8);
		} while (Board[Food.row][Food.col] > 0);
	}
}

void ScanJoystick()
{
	int prevDirection = snakeDirection;  // Saves the last direction.
	long timestamp = millis();

	while (millis() < timestamp + snakeSpeed)
  {
		// Calculates snake speed.
		float temp = CalculateSpeed(analogRead(potentiometer), 0, 1023, 0, 1);
		snakeSpeed = CalculateSpeed(pow(temp, 3.5), 0, 1, 10, 1000); 
		if (snakeSpeed == 0) snakeSpeed = 1;

		// Adjusts the direction of the snake according to the input from the joystick
    if (analogRead(JoystickY_axis) < (Joystick.y - 160))
    {
      snakeDirection = 1;
    }
    if (analogRead(JoystickX_axis) > (Joystick.x + 160))
    {
      snakeDirection = 2;
    }
    if (analogRead(JoystickY_axis) > (Joystick.y + 160))
    {
      snakeDirection = 3;
    }
    if (analogRead(JoystickX_axis) < (Joystick.x - 160))
    {
      snakeDirection = 4;
    }
		
		// Prevents the snake from changing direction 180 degrees
    if ((prevDirection == (snakeDirection -2) || prevDirection == (snakeDirection + 2)) && prevDirection != 0)
    {
      snakeDirection = prevDirection;
    } 

		// Makes food blink continuously
		matrix.setLed(0, Food.row, Food.col, millis() % 100 < 50 ? 1 : 0);
	}
}

float CalculateSpeed(float x, float input_min, float input_max, float output_min, float output_max) 
{
	return (x - input_min) * (output_max - output_min) / (input_max - input_min) + output_min;
}

const PROGMEM bool Points[][8][8] =
{
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 1, 1, 1, 0},
		{0, 1, 1, 1, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 1, 1, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 1, 0, 0},
		{0, 0, 1, 0, 1, 1, 0, 0},
		{0, 1, 0, 0, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 0, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0}
	}
};

const PROGMEM bool scorePoint[8][58] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Prints the player's score on the LED screen
void ScorePoint(int score)
{
  // If there is no score, it does not print the score on the screen.
	if (score < 0 || score > 99) return;    

	int sec = score % 10;
	int fir = (score / 10) % 10;

	[&] {
		for (int i = 0; i < sizeof(scorePoint[0]) + 2 * sizeof(Points[0][0]); i++) 
    {
      // Rotates as much as the width of the LED screen
			for (int column = 0; column < 8; column++)
      {
				delay(5);
        // Rotates as much as the height of the LED screen
				for (int row = 0; row < 8; row++)
        {
					if (i <= sizeof(scorePoint[0]) - 8)
          {
						matrix.setLed(0, row, column, pgm_read_byte(&(scorePoint[row][column + i])));
					}
          // move 6 px in front of the previous message
					int temp = column + i - sizeof(scorePoint[0]) + 6; 

					// if the score is < 10, shift out the first digit
					if (score < 10) temp += 8;

          // Shows only if score is >= 10
					if (temp >= 0 && temp < 8)
          {
						if (fir > 0) matrix.setLed(0, row, column, pgm_read_byte(&(Points[fir][row][temp]))); 
					}
          else
          {
						temp -= 8;
						if (temp >= 0 && temp < 8)
            {
							matrix.setLed(0, row, column, pgm_read_byte(&(Points[sec][row][temp])));
						}
					}
				}
			}

		}
	}();

	matrix.clearDisplay(0);
}