#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <math.h>

unsigned long lastTiltChangeTime = 0;
bool lastFlatState = false;

const unsigned long debounceDelay = 1500; // milliseconds to confirm a state change
const int averageSamples = 10;

float avgPitch = 0.0;
float avgRoll = 0.0;

float stablePitch = 0.0; // global or static

// === Create sensor object ===
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// === Calibration offsets (adjust for your sensor mount) ===
const float offsetX = -1.3;
const float offsetY = +0.27;
const float offsetZ = +0.86;

// === Flatness threshold in degrees ===
const float maxPitch = 2.5; // degrees
const float maxRoll = 2.5;  // degrees

void initAccelerometer()
{
    if (!accel.begin())
    {
        Serial.println("Failed to initialize ADXL345!");
        while (1)
            ;
    }

    Serial.println("ADXL345 Initialized.");
    delay(150);
}

// === Calculate pitch (rotation front-back) ===
float getPitch(float y, float x, float z)
{
    return atan2(y, sqrt(x * x + z * z)) * (180.0 / PI);
}

// === Calculate roll (rotation left-right) ===
float getRoll(float x, float y, float z)
{
    return atan2(x, sqrt(y * y + z * z)) * (180.0 / PI);
}

// === Check if the car is level ===
bool isCarFlat(float pitch, float roll)
{
    return abs(pitch) < maxPitch && abs(roll) < maxRoll;
}

bool carLeveled()
{
    float sumPitch = 0.0;
    float sumRoll = 0.0;

    for (int i = 0; i < averageSamples; i++)
    {
        sensors_event_t event;
        accel.getEvent(&event);
        float x = (event.acceleration.x + offsetX) * -1; // keep your flipped axis
        float y = event.acceleration.y + offsetY;
        float z = event.acceleration.z + offsetZ;
        // Serial.printf("X: %0.2f  - Y: %0.2f - Z: %0.2f\n",x,y,z);
        sumPitch += getPitch(y, x, z);
        sumRoll += getRoll(x, y, z);

        delay(10); // short delay between samples
    }

    avgPitch = sumPitch / averageSamples;
    avgRoll = sumRoll / averageSamples;

    if (abs(avgPitch) < 1.5)
        avgPitch = 0;
    if (abs(avgRoll) < 1.5)
        avgRoll = 0;

    
    // Serial.print("Pitch: ");
    // Serial.print((avgPitch >= 0 ? "+" : ""));
    // Serial.print(avgPitch, 2);
    // Serial.print("°\t");

    // Serial.print("Roll: ");
    // Serial.print(avgRoll, 2);
    // Serial.print("°\t");

    bool currentFlat = isCarFlat(avgPitch, avgRoll);

    // Debounce logic
    if (currentFlat != lastFlatState)
    {
        if (millis() - lastTiltChangeTime > debounceDelay)
        {
            lastFlatState = currentFlat;
            lastTiltChangeTime = millis();
        }
        else
        {
            // Keep previous state if within debounce window
            currentFlat = lastFlatState;
        }
    }
    else
    {
        lastTiltChangeTime = millis(); // reset timer
    }

    if (currentFlat)
    {
        // Serial.println("--> FLAT");
        return true;
    }
    else
    {
        // Serial.print("--> TILTED: ");
        // if (avgPitch >= maxPitch)
        //     Serial.print("UPHILL ");
        // if (avgPitch <= -maxPitch)
        //     Serial.print("DOWNHILL ");
        // if (avgRoll >= maxRoll)
        //     Serial.print("TILT RIGHT");
        // if (avgRoll <= -maxRoll)
        //     Serial.print("TILT LEFT");
        // Serial.println();
        return false;
    }
}

float carAngle()
{
    sensors_event_t event;
    accel.getEvent(&event);

    float x = (event.acceleration.x + offsetX) * -1; // flipped axis
    float y = event.acceleration.y + offsetY;
    float z = event.acceleration.z + offsetZ;

    float pitch = getPitch(y, x, z);

    // --- Improved hysteresis logic ---
    static float stablePitch = 0.0;
    const float enterThreshold = 2.0; // start counting slope above this
    const float exitThreshold  = 1.0; // consider flat below this

    // If we're beyond enterThreshold → update stable pitch normally
    if (abs(pitch) > enterThreshold)
    {
        stablePitch = pitch;
    }
    // If within deadband → hold last stable value (don’t force to 0 immediately)
    else if (abs(pitch) < exitThreshold)
    {
        stablePitch = 0.0;
    }
    // Otherwise keep previous stablePitch (no change between thresholds)

    return stablePitch;
}
