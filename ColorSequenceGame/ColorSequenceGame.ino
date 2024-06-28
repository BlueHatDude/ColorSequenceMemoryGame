/**
 * @file ColorSequenceGame.ino
 * @author BlueHatDude (https://github.com/BlueHatDude)
 * @version 1.0
 * 
 * 1. On startup, the lights will turn on one-by-one, then the game will start
 * 2. At the start of each round, a sequence of colors will be displayed
 * 3. After each round, the player must push the corresponding buttons in
 *     the correct sequence.
 * 4. Steps 2 & 3 will repeat until the game ends.
 * 
 * Note: the indication that the player has lost is all the LEDs turning on
 *          then turning off one-by-one. 
 */
#include <stdint.h>


/*
    Gameplay loop:
        1. Generate a color sequence, appending another
            color after each iteration.
        2. Display the color sequence using the LEDs
        3. Prompt the user to input the sequence
        4. Repeat steps 1-3 until user gets sequence wrong
*/


/* declaring pins */
#define RED_BUTTON 2
#define RED_LED 3
#define BLUE_BUTTON 4
#define BLUE_LED 5
#define GREEN_BUTTON 6
#define GREEN_LED 7
#define YELLOW_BUTTON 8
#define YELLOW_LED 9


/* button states */
static volatile uint8_t red_button_state = LOW;
static volatile uint8_t blue_button_state = LOW;
static volatile uint8_t green_button_state = LOW;
static volatile uint8_t yellow_button_state = LOW;


/* game state */
typedef enum : uint8_t {
    NO_COLOR,
    RED,
    BLUE,
    GREEN,
    YELLOW
} Color;


struct GameState {
    bool has_lost;
    Color sequence[25];
    uint8_t index;
};
static struct GameState g_GameState = {
    .has_lost = false,
    .sequence = { NO_COLOR },
    .index = 0,
};

/**
    This code runs on start up, 
    blinking the LEDs in sequence
*/
static void runStartup() {
    digitalWrite(RED_LED, HIGH);
    delay(500);
    digitalWrite(BLUE_LED, HIGH);
    delay(500);
    digitalWrite(GREEN_LED, HIGH);
    delay(500);
    digitalWrite(YELLOW_LED, HIGH);
    delay(1000);

    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
}


static void addColor() {
    Color color = (Color) random(RED, YELLOW + 1);
    g_GameState.sequence[g_GameState.index] = color;
    g_GameState.index++;
}


static inline void displayColor(Color c) {
    uint8_t color_pin = 0;
    switch (c) {
        case RED:
            color_pin = RED_LED;
            break;
        case BLUE:
            color_pin = BLUE_LED;
            break;
        case GREEN:
            color_pin = GREEN_LED;
            break;
        case YELLOW:
            color_pin = YELLOW_LED;
            break;
        default:
            break;
    }
    
    digitalWrite(color_pin, HIGH);
    delay(500);
    digitalWrite(color_pin, LOW);
} 


static void displaySequence() {
    for (uint8_t i = 0; i < g_GameState.index; i++) {
        displayColor(g_GameState.sequence[i]);
        delay(250);
    }
}


static void updateButtonStates() {
    red_button_state = digitalRead(RED_BUTTON);
    blue_button_state = digitalRead(BLUE_BUTTON);
    green_button_state = digitalRead(GREEN_BUTTON);
    yellow_button_state = digitalRead(YELLOW_BUTTON);
}


/**
* @brief checks to see if any of the buttons are pressed 
*/
static bool buttonIsPressed() {
    updateButtonStates();
    return (
        (red_button_state == HIGH)
        || (blue_button_state == HIGH)
        || (green_button_state == HIGH)
        || (yellow_button_state == HIGH)
    );
}


static uint8_t getButtonPressed() {
    if (red_button_state == HIGH) {
        return RED_BUTTON;
    }
    else if (blue_button_state == HIGH) {
        return BLUE_BUTTON;
    }
    else if (green_button_state == HIGH) {
        return GREEN_BUTTON;
    }
    else if (yellow_button_state == HIGH) {
        return YELLOW_BUTTON;
    }
    else {
        return 0; /* no button is pressed */
    }
}


static uint8_t colorToLED(const Color color) {
    switch (color) {
        case RED: 
            return RED_LED;
        case BLUE: 
            return BLUE_LED;
        case GREEN: 
            return GREEN_LED;
        case YELLOW: 
            return YELLOW_LED;
        case NO_COLOR: 
            return 0;
    }
}


static uint8_t buttonToLED(const uint8_t btn_pin) {
    switch (btn_pin) {
        case RED_BUTTON: return RED_LED;
        case BLUE_BUTTON: return BLUE_LED;
        case GREEN_BUTTON: return GREEN_LED;
        case YELLOW_BUTTON: return YELLOW_LED;
        default: return NO_COLOR;
    }
}


static Color buttonToColor(const uint8_t btnPin) {
    switch (btnPin) {
        case RED_BUTTON: return RED;
        case BLUE_BUTTON: return BLUE;
        case GREEN_BUTTON: return GREEN;
        case YELLOW_BUTTON: return YELLOW;
        default: return NO_COLOR;
    }
}


static Color getButtonColor() {
    /* await button input */
    while (not buttonIsPressed()) {
        delay(10); /* avoiding some redundant checks */
        continue;
    }
    /* find which button was pressed */
    uint8_t buttonPin = getButtonPressed();
    uint8_t ledPin = buttonToLED(buttonPin);
    /* light up corresponding LED */
    digitalWrite(ledPin, HIGH);
    /* check if button is being held */
    while (digitalRead(buttonPin) == HIGH) {
        delay(10);
        continue;
    }
    /* turn off corresponding LED */
    digitalWrite(ledPin, LOW);
    /* convert button to color and return color */
    return buttonToColor(buttonPin);
}


static void startNextRound() {
    delay(250);
    addColor();
    displaySequence();
    delay(1500);

    Color chosenColor;
    for (uint8_t btnsPressed = 0; btnsPressed < g_GameState.index; btnsPressed++) {
        chosenColor = getButtonColor();
        if (chosenColor != g_GameState.sequence[btnsPressed]) {
            g_GameState.has_lost = true;
            return;
        }
    }
}


void showLoss() {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, HIGH);
    delay(1000);
    digitalWrite(YELLOW_LED, LOW);
    delay(500);
    digitalWrite(GREEN_LED, LOW);
    delay(500);
    digitalWrite(BLUE_LED, LOW);
    delay(500);
    digitalWrite(RED_LED, LOW);
    delay(500);
}


void setup() {
    pinMode(RED_BUTTON, INPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BLUE_BUTTON, INPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(GREEN_BUTTON, INPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_BUTTON, INPUT);
    pinMode(YELLOW_LED, OUTPUT);
    
    randomSeed(analogRead(A0));

    runStartup();
    delay(1000);
}


void loop() {
    if (not g_GameState.has_lost) {
        startNextRound();
    } else {
        showLoss();
    }
}
 