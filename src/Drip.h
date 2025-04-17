#include <arduino.h>
#include <FastLED.h>

#define ACCELERATION 5

class Drip
{
    fract8 Hue = 0;
    fract16 Location = 0;
    fract16 Velocity = 0;

    fract8 Friction = 128;
    fract16 TerminalVelocity = 800;

public:
    Drip()
    {
        Reset();
    }

    void Reset()
    {
        Location = 0;
        Hue = random8();
        Velocity = 0;
        TerminalVelocity += random8();

        Friction = random8();
        if (Friction > 250)
        {
            Friction = 250;
        }
    }

    void Update()
    {
        Hue++;

        fract16 accel = scale16by8(ACCELERATION, 255 - Friction);
        Velocity += max(1, (int)accel);
        if (Velocity > TerminalVelocity)
        {
            Velocity = TerminalVelocity;
        }

        fract16 lastLocation = Location;
        Location += Velocity;
        if (Location < lastLocation)
        {
            Reset();
        }
    }

    void Render(CPixelView<CRGB> leds)
    {
        leds[lerp16by16(0, leds.len - 1, Location)] += CHSV(Hue, 255, 100);
    }

};