#include <Bounce2.h>
#include <FastLED.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>

#include "bird.hpp"
#include "world.hpp"

#define LED_PIN 11
#define START_PIN 2 // Reset Button
#define IN_PIN 3    // Vogel Button
#define NUM_LEDS 256
#define BRIGHTNESS 50
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define FLAP_TIMER_INTERVAL 150000 // Timer: 0.15s
#define MIN_BIRD_POSITION 16
#define MAX_BIRD_POSITION 31
#define STARTING_BIRD_POSITION 24
#define DEFAULT_WORLD_SPEED 1000 // Timer: 1s
#define WORLD_GAP 5
#define WORLD_DISTANCE 4

CRGB leds[NUM_LEDS];
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Bounce2::Button button = Bounce2::Button();
World world(WORLD_GAP, WORLD_DISTANCE);
Bird bird(MIN_BIRD_POSITION, MAX_BIRD_POSITION, STARTING_BIRD_POSITION);

static unsigned long previousMillis = 0;
volatile int score = 0;
volatile int world_timer_interval = DEFAULT_WORLD_SPEED;
volatile bool is_game_over = true;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(IN_PIN, INPUT_PULLUP); // set digital pin 3 as input
  pinMode(START_PIN, INPUT_PULLUP);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Timer1.initialize(FLAP_TIMER_INTERVAL);
  Timer1.attachInterrupt(flap);

  // Setup Button
  button.attach(IN_PIN, INPUT_PULLUP);
  button.interval(100); // Entprellzeit
  button.setPressedState(LOW);

  // Setup Interrupt für Button
  attachInterrupt(digitalPinToInterrupt(IN_PIN), flyUp,
                  CHANGE); // Vogelsteuerung
  attachInterrupt(digitalPinToInterrupt(START_PIN), reset, FALLING); // Reset
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop() {
  // Button entprellen
  button.update();

  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("SCORE: ");
  lcd.print(score);

  if (!is_game_over && millis() - previousMillis > world_timer_interval) {
    previousMillis = millis();
    shift_world();
  }

  world.print(leds);

  // Game over wenn der Vogel die Balken berührt
  if (leds[bird.getPosition()] != CRGB::Black) {
    lcd.setCursor(0, 1);
    lcd.print("GAME OVER!");
    is_game_over = true;
  }

  bird.print(leds);
  FastLED.show();
  delay(10);
}

void reset() {
  if (is_game_over) {
    world.reset();
    bird.reset();
    world_timer_interval = DEFAULT_WORLD_SPEED;
    is_game_over = false;
    score = 0;
  }
}

void flyUp() { bird.flyUp(digitalRead(IN_PIN) == HIGH); }

void flap() {
  if (is_game_over) {
    return;
  }
  bird.hide(leds);
  bird.flap();
}

void shift_world() {
  if (is_game_over) {
    return;
  }

  if (leds[0] != CRGB::Black || leds[15] != CRGB::Black) {
    score++;

    // Geschwindigkeitserhöhung der Balken
    if ((score % 4) == 0 && score < 9 * 4) {
      world_timer_interval -= 100;
    }
  }

  world.shift();
}
