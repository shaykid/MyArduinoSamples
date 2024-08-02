#include <LedControl.h>
#include <TM1637Display.h>

// Pin connections for the joystick
#define JOYSTICK_VRX_PIN A0
#define JOYSTICK_VRY_PIN A1
#define JOYSTICK_SW_PIN 13

// Pin connections for the LED matrix
#define DIN_PIN 8
#define CS_PIN 9
#define CLK_PIN 10

// Pin connections for the 4-digit 7-segment display
#define CLK_PIN_7SEG 4
#define DIO_PIN_7SEG 5

// Define the reset number of seconds
#define SECONDS 99

// Create an instance of the LedControl library for the LED matrix
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

// Create an instance of the TM1637Display library for the 7-segment display
TM1637Display display(CLK_PIN_7SEG, DIO_PIN_7SEG);

// Snake game variables
const int matrixSize = 8;
int snakeLength = 1;
int snakeX[64];
int snakeY[64];
int foodX, foodY;
int dirX = 1, dirY = 0; // Initial direction to the right
unsigned long lastMoveTime = 0;
unsigned long lastTimerUpdate = 0;
int moveDelay = 500; // Delay between moves in milliseconds
int countdown = SECONDS; // Countdown timer starting value
int score = 0;

void setup() {
  // Initialize the LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  // Initialize joystick pins
  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);

  // Initialize 7-segment display
  display.setBrightness(0x0f); // Set maximum brightness

  // Initialize snake starting position
  snakeX[0] = 4;
  snakeY[0] = 4;

  // Place the first food item
  placeFood();
}

void loop() {
  // Read joystick input
  int vrxValue = analogRead(JOYSTICK_VRX_PIN);
  int vryValue = analogRead(JOYSTICK_VRY_PIN);
  int swValue = digitalRead(JOYSTICK_SW_PIN);

  // Determine direction based on joystick input
  if (vrxValue < 300) {
    dirX = -1;
    dirY = 0;
  } else if (vrxValue > 700) {
    dirX = 1;
    dirY = 0;
  } else if (vryValue < 300) {
    dirX = 0;
    dirY = -1;
  } else if (vryValue > 700) {
    dirX = 0;
    dirY = 1;
  }

  // Move the snake
  if (millis() - lastMoveTime > moveDelay) {
    lastMoveTime = millis();
    moveSnake();
  }

  // Update countdown timer
  if (millis() - lastTimerUpdate >= 1000) {
    lastTimerUpdate = millis();
    countdown--;
    if (countdown < 0) {
      countdown = 0;
      resetGame(); // Reset game if countdown reaches zero
    }
    displayScoreAndTime(score, countdown);
  }

  // Display the snake and food
  lc.clearDisplay(0);
  for (int i = 0; i < snakeLength; i++) {
    lc.setLed(0, snakeY[i], snakeX[i], true);
  }
  lc.setLed(0, foodY, foodX, true);
}

void moveSnake() {
  // Move the snake body
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Move the snake head
  snakeX[0] += dirX;
  snakeY[0] += dirY;

  // Wrap around edges
  if (snakeX[0] < 0) snakeX[0] = matrixSize - 1;
  if (snakeX[0] >= matrixSize) snakeX[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = matrixSize - 1;
  if (snakeY[0] >= matrixSize) snakeY[0] = 0;

  // Check for collision with food
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    score++; // Increase score
    countdown = SECONDS; // Reset countdown timer
    delay(120); // Add 0.12 seconds delay before placing food
    placeFood();
    displayScoreAndTime(score, countdown); // Update display immediately
  }

  // Check for collision with self
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      resetGame();
      break;
    }
  }
}

void placeFood() {
  bool validPosition = false;
  while (!validPosition) {
    foodX = random(matrixSize);
    foodY = random(matrixSize);
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if (foodX == snakeX[i] && foodY == snakeY[i]) {
        validPosition = false;
        break;
      }
    }
  }
}

void resetGame() {
  snakeLength = 1;
  snakeX[0] = 4;
  snakeY[0] = 4;
  dirX = 1;
  dirY = 0;
  score = 0; // Reset score
  countdown = SECONDS; // Reset countdown timer
  delay(120); // Add 0.12 seconds delay before placing food
  placeFood();
  displayScoreAndTime(score, countdown); // Update display immediately
}

void displayScoreAndTime(int score, int time) {
  int displayValue;
  if (score >= 0 && score < 10) {
    // Display as "-score-time"
    displayValue = (score * 100) + time;
    display.showNumberDecEx(displayValue, 0b01001000, true); // 0b01001000 to display a dash
  } else {
    // Display as "score-time"
    displayValue = (score * 1000) + time;
    display.showNumberDec(displayValue, true);
  }
}
