#include <ESP_Color.h>

ESP_Color::Color color(0.0f, 0.0f, 0.5f);
ESP_Color::HSVf hsvColor = color.ToHsv();

#define LEDR 8
#define LEDG 9
#define LEDB 7

const int NUM_SLIDERS = 5;
const int analogInputs[NUM_SLIDERS] = {0, 1, 2, 3, 4};

int analogSliderValues[NUM_SLIDERS];

// Settings mode
// Uses the last slider to control the LED colors
// LED settings mode is triggered when the slider goes to max, min 3 times in a row (3 seconds)
// - LED intensity is controlled by the fiirst slider
// - LED color is controlled by the second slider

// Led settings variables
// int ledIntensity = 0;                       // 0-255
// int ledColor = 0;                           // HSV color (0-360)
int ledSettingsCounter = 0;                 // Counter for the number of times the slider goes to max, min
int ledSettingsCounterMax = 3;              // Number of times the slider needs to go to max, min to trigger settings mode
int ledSettingsCounterThreshold = 3000;     // Number of seconds to wait for the next max, min
int ledSettingsCounterLastTime = 0;         // Last time the slider was at max, min
bool ledSettingsLastIsMax = false;          // Last toggle of the slider
bool isSettingsMode = 0;                    // Flag to check if we are in settings mode

int ledSettingsEdditingLastTime = 0;        // Last time the settings were edited
int ledSettingsEdditingThreshold = 5000;    // Number of milliseconds to wait before exiting settings mode
int ledSettingsEdditingLastValue[3];        // Last values of the sliders [value, color, saturation]
const int TOLERANCE = 10;                   // Settings measures tolerance


void setup() { 
  analogReadResolution(10);       // Set ADC resolution to 10 bits - 0-1023
  analogSetAttenuation(ADC_11db); // Set ADC input attenuation to 0dB

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, LOW);      // Turn off all LEDs
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  Serial.begin(9600);
}

void loop() {
  updateSliderValues();
  checkLedSettingsTrigger();    // Check if we need to enter settings mode
  sendSliderValues();           // Actually send data (all the time)
  // printSliderValues();       // For debug
  delay(10);
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
     analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}

// LED Settings
void checkLedSettingsTrigger() {
  // Check if the last slider is at max, min
  bool isAtMax = abs(analogSliderValues[0] - 0) <= TOLERANCE;
  bool isAtMin = abs(1023 - analogSliderValues[0]) <= TOLERANCE;

  // Check 
  // - if the slider is at max, min
  // - if the last state was different
  if ((isAtMax || isAtMin) && ledSettingsLastIsMax != isAtMax) {
    ledSettingsCounter++;
    ledSettingsLastIsMax = isAtMax;
    ledSettingsCounterLastTime = millis();
    // Serial.println("Settings: Toggling counter");
  }

  // Reset the counter if the time is greater than the threshold
  if (ledSettingsCounter > 0 && millis() - ledSettingsCounterLastTime > ledSettingsCounterThreshold) {
    ledSettingsCounter = 0;
    ledSettingsLastIsMax = false;
    ledSettingsCounterLastTime = 0;
    // Serial.println("Settings: Resetting counter");
  }

  // Check if the counter is greater than the max, then trigger settings mode
  if (ledSettingsCounter >= ledSettingsCounterMax) {
    // Trigger LED settings mode
    ledSettingsCounter = 0;
    ledSettingsCounterLastTime = 0;
    // Enter settings mode
    updateSettingsLoop();
  }
}

void updateSettingsLoop() {
  Serial.println("Settings: Entering settings mode");
  lightEnteringSettingsMode();

  isSettingsMode = true;

  ledSettingsEdditingLastTime = millis();           // Set the last time the settings were edited, to avoid auto-exiting right away

  while (isSettingsMode) {
    const int sliderIntexColor      = NUM_SLIDERS - 1;
    const int sliderIntexSaturation = NUM_SLIDERS - 2;
    const int sliderIntexValue      = NUM_SLIDERS - 3;
    
    // Check if the settings were edited
    updateSliderValues();
    bool editedSliiderColor      = abs(analogSliderValues[sliderIntexColor]      - ledSettingsEdditingLastValue[0]) > TOLERANCE;
    bool editedSliiderSaturation = abs(analogSliderValues[sliderIntexSaturation] - ledSettingsEdditingLastValue[1]) > TOLERANCE;
    bool editedSliiderLight      = abs(analogSliderValues[sliderIntexValue]      - ledSettingsEdditingLastValue[2]) > TOLERANCE;
    
    if (editedSliiderLight || editedSliiderColor || editedSliiderSaturation) {
      // Serial.println("Settings: Settings edited");
      ledSettingsEdditingLastValue[0] = analogSliderValues[sliderIntexColor];
      ledSettingsEdditingLastValue[1] = analogSliderValues[sliderIntexSaturation];
      ledSettingsEdditingLastValue[2] = analogSliderValues[sliderIntexValue];
      ledSettingsEdditingLastTime = millis();
      
    } else if (millis() - ledSettingsEdditingLastTime > ledSettingsEdditingThreshold) {
      // Serial.println("Settings: Auto-exiting settings mode");
      isSettingsMode = false;
      lightExitingSettingsMode();
      return;
    }

    // Set LED color
    hsvColor.H = map(analogSliderValues[sliderIntexColor], 0, 1023, 0, 1000) / 1000.0f;
    hsvColor.S = map(analogSliderValues[sliderIntexSaturation], 0, 1023, 0, 1000) / 1000.0f;
    hsvColor.V = map(analogSliderValues[sliderIntexValue], 0, 1023, 0, 1000) / 1000.0f;

    // Set LED color
    setLedsToCurrent();

    delay(10);
  }
}

/** Set the LED color to the current HSL color */
void setLedsToCurrent() {
  color = ESP_Color::Color::FromHsv(hsvColor);
  setLedsToColor(color);
}

void setLedsToColor(ESP_Color::Color color) {
  analogWrite(LEDR, (int) (color.R * 255));
  analogWrite(LEDG, (int) (color.G * 255));
  analogWrite(LEDB, (int) (color.B * 255));
}

void setFadeLedsToColor(ESP_Color::Color color, int delayTime) {
  ESP_Color::Color currentColor = ESP_Color::Color::FromHsv(hsvColor);
  ESP_Color::Color diffColor = color - currentColor;

  for (int i = 0; i < 100; i++) {
    ESP_Color::Color newColor = currentColor + (diffColor * (i / 100.0f));
    setLedsToColor(newColor);
    delay(delayTime);
  }
}

// Effects
void lightPulsating(int intensity, int times, int delayTimeIn, int delayTimeOut) {
  // Pulsate
  for (int i = 0; i < times; i++) {
    for (int i = 0; i < intensity; i++) {
      analogWrite(LEDR, i);
      analogWrite(LEDG, i);
      analogWrite(LEDB, i);
      delay(delayTimeIn);
    }

    for (int i = intensity; i >= 0; i--) {
      analogWrite(LEDR, i);
      analogWrite(LEDG, i);
      analogWrite(LEDB, i);
      delay(delayTimeOut);
    }
  }

  // Fade to configured color
  setFadeLedsToColor(color, 10);
}

void lightEnteringSettingsMode() {
  // Pulse the LEDs fading in and out
  lightPulsating(150, 3, 2, 1);
}

void lightExitingSettingsMode() {
  // Pulse the LEDs fading in and out
  lightPulsating(150, 3, 1, 1);
}
