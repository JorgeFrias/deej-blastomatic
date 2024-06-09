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
int ledIntensity = 0;         // 0-255
int ledColor = 0;             // HSV color (0-360)
int ledSettingsCounter = 0;           // Counter for the number of times the slider goes to max, min
int ledSettingsCounterMax = 3;        // Number of times the slider needs to go to max, min to trigger settings mode
int ledSettingsCounterThreshold = 3000;     // Number of seconds to wait for the next max, min
int ledSettingsCounterLastTime = 0;   // Last time the slider was at max, min
bool ledSettingsLastIsMax = false;    // Last toggle of the slider
bool isSettingsMode = 0;              // Flag to check if we are in settings mode

int ledSettingsEdditingLastTime = 0;        // Last time the settings were edited
int ledSettingsEdditingThreshold = 5000;    // Number of milliseconds to wait before exiting settings mode
int ledSettingsEdditingLastValue[2];        // Last values of the sliders [intensity, color]
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
  isSettingsMode = true;

  ledSettingsEdditingLastTime = millis();           // Set the last time the settings were edited, to avoid auto-exiting right away

  while (isSettingsMode) {
    const int sliderIntexLight = NUM_SLIDERS - 1;
    
    // Check if the settings were edited
    updateSliderValues();
    bool editedSliiderLight = abs(analogSliderValues[sliderIntexLight] - ledSettingsEdditingLastValue[0]) > TOLERANCE;
    
    if (editedSliiderLight) {
      // Serial.println("Settings: Settings edited");
      ledSettingsEdditingLastValue[0] = analogSliderValues[sliderIntexLight];
      ledSettingsEdditingLastTime = millis();
      
    } else if (millis() - ledSettingsEdditingLastTime > ledSettingsEdditingThreshold) {
      // Serial.println("Settings: Auto-exiting settings mode");
      isSettingsMode = false;
      return;
    }

    // Set LED intensity
    ledIntensity = map(analogSliderValues[sliderIntexLight], 0, 1023, 0, 255);

    // Serial.println("Settings: LED intensity: " + String(ledIntensity));

    // Set LED color
    analogWrite(LEDR, ledIntensity);
    analogWrite(LEDG, ledIntensity);
    analogWrite(LEDB, ledIntensity);

    delay(10);
  }
}