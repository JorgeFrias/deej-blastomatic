#define LEDR 8
#define LEDG 9
#define LEDB 7

const int NUM_SLIDERS = 5;
const int analogInputs[NUM_SLIDERS] = {0, 1, 2, 3, 4};

int analogSliderValues[NUM_SLIDERS];

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
