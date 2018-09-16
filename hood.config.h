#ifndef CONFIG
#define CONFIG

#define ENABLE_SERIAL

//////////////////////////
// LED HARDWARE SETTINGS

// LED strand data pin
#define LED_PIN     5
#define LED_COLOR_ORDER GRB

// Number of pixels in strand
#define N_PIXELS    52

// When defined, uses an analog signal to vary the overall brightness
#define BRIGHTNESS_PIN 0

// Enable/disable effects
// Note that depending on the available RAM etc, you might not be able to enable ALL
// effects. If you see the lights stall or restart, try disabling one or more effects.
#define EFFECT_BARS
// #define EFFECT_PULSE
#define EFFECT_NOISE
#define EFFECT_SINELON
#define EFFECT_FIRE

// Must be updated to reflect the number of enabled effects!
#define N_EFFECTS 4

// Which effect to use on initial startup - if no button is present, then this is
// the only effect which is used. For subsequent startups, EEPROM is used to save
// the previously selected effect.
#define INITIAL_EFFECT 0

// When defined, enables a button on a digital input to change effects
#define EFFECT_BUTTON_PIN 7


////////////////////
// AUDIO SETTINGS

// Which FFT index (0 - 7) to pull data from
#define FFT_SLOT    0

// Optional second slot to pull data from
#define FFT_SLOT2   1

// Amount of smoothing for historic volume value
#define HISTORIC_SMOOTH_FACTOR_UP 500.0
// Faster historic volume drop, so we stay closer to the low end of current
// general noise
#define HISTORIC_SMOOTH_FACTOR_DOWN 150.0

// Amount of smoothing for current volume value
#define SMOOTH_FACTOR 3.0

// Boost all effects
#define BASE_LEVEL_SCALE   1.6

// Historic FFT frames to keep to determine high/low range
#define HISTORIC_FRAMES 40
#define NOISE_THRESHOLD 5
#define LEVEL_CUTOFF 30

// Sanitized level is designed to be a less "jumpy" level source. It will always be
// in the range [0.0, 1.0]. It operates in a peak/decay fashion - if the current level
// is higher, it will jump to that level, but then decays at a set rate if the level
// drops again.
#define SANITIZED_LEVEL_SCALE 1.4
// The incoming level signal must be greater than this minimum to have any effect
#define SANITIZED_LEVEL_MIN 0.4
// Instead of directly using the level, treat it like a peak, decaying it at a fixed rate
#define SANITIZED_LEVEL_DECAY 0.05


/////////////////////////
// BAR EFFECT SETTINGS

#define BAR_MIRROR true
// When defined, reverses the direction of the bar
#define BAR_FLIP

// Boost bar effect
#define BAR_LEVEL_SCALE 1.0

// Color for the low intensity bar
#define BAR_COL_LOW     0
// Color for the high intensity bar - must be greater than min
#define BAR_COL_HIGH     160
// When defined, inverts high/low bar color spectrum
#define BAR_COL_INVERT
// Derived color range between min/max
#define BAR_COL_RANGE   (BAR_COL_HIGH - BAR_COL_LOW)
// Color difference between lower and upper parts of bar
#define BAR_COL_VAR     60

#define MIN_BAR_SIZE 70

// Rate at which peak dot falls
#define PEAK_FALL_RATE 0.2

#define PEAK_RADIUS 1

// Background color settings
#define BACKGROUND_COLOR 160
#define BACKGROUND_CUTOFF (N_PIXELS / 3)
#define BACKGROUND_MAX 0.18
#define BACKGROUND_INCREASE 0.0005
#define BACKGROUND_DECREASE 0.01

#define HISTORIC_SCALE 1.2


///////////////////////////
// PULSE EFFECT SETTINGS

#define PULSE_MIRROR false

// High/low values for transformed level that determine when a beat
// stops and starts
#define BEAT_LOW 0.5
#define BEAT_HIGH 0.7

// Number of frames the level must be below BEAT_LOW to consider a beat
// finished
#define BEAT_LOW_FRAMES 20

// Minimum brightness at lowest volume
#define PULSE_MIN_BRIGHTNESS 0.1

// Boost amount of brightness - helps prevent low volumes being clamped by
// the minimum brightness
#define PULSE_BRIGHTNESS_BOOST 0.0

// If uncommented, pulse will pick a random color each time instead of
// advancing the color wheel
// #define PULSE_RANDOM

// Advances the color wheel by this amount on each beat
#define PULSE_COLOR_INCREMENT 55

#define PULSE_FADE_FACTOR 20.0
#define PULSE_BRIGHTNESS_BEAT 2.5


///////////////////////////
// NOISE EFFECT SETTINGS

#define NOISE_MIRROR true

#define NOISE_SCALE_BASE 70
#define NOISE_SCALE_DELTA 100

// Minimum rate at which to shift the noise pattern
#define NOISE_SHIFT_MIN 150
// Audio level multiplier to shift the noise pattern at
#define NOISE_SHIFT_FACTOR 1500

// The brightness and saturation are shifted for the current level, between [MIN, 255]
#define NOISE_BRIGHTNESS_MIN 200
#define NOISE_SATURATION_MIN 150

// After applying the brightness shift for the current level, contrast is applied.
// If the brightness is above the midpoint, it's boosted, otherwise it's reduced.
#define NOISE_CONTRAST_MIDPOINT 50
#define NOISE_CONTRAST_AMOUNT 0.8

// If defined, makes the noise align on each end
//#define NOISE_CIRCULAR


/////////////////////////////
// SINELON EFFECT SETTINGS

#define SINELON_MIRROR false

// The brightness and saturation are shifted for the current level, between [MIN, 255]
#define SINELON_BRIGHTNESS_MIN 120
#define SINELON_SATURATION_MIN 180

// Number of dots in the effect
#define SINELON_DOTS 3

// Position advances by a maximum of this value, and a minimum of the MIN_LEVEL multiplied by it.
#define SINELON_POS_RATE 150.0
#define SINELON_POS_MIN_LEVEL 0.1

// Ditto for hue - worth noting that small values will round down to 0, and the saturation won't
// advance at all.
#define SINELON_HUE_RATE 8.0
#define SINELON_HUE_MIN_LEVEL 0.1

// Number of oscillators per dot - increasing this adds randomness, but increases the chance of
// the dot being in the middle most of the time
#define SINELON_NUM_OSCILLATORS 2

// Min/max period defines how quickly each of the oscillators for a dot will move
#define SINELON_PERIOD_MIN 15
#define SINELON_PERIOD_MAX 40


//////////////////////////
// FIRE EFFECT SETTINGS

#define FIRE_MIRROR true

// For these parameters, the min is used when the level is zero, and max when the level is at maximum.

// How much does the air cool as it rises?
// Less cooling = taller flames. More cooling = shorter flames. Tweak for the number of LEDs.
#define FIRE_COOLING_MIN  45
#define FIRE_COOLING_MAX  45

// What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire. Lower chance = more flickery fire.
#define FIRE_SPARKING_MIN 30
#define FIRE_SPARKING_MAX 160

// Heat for each new spark. Also used to maintain a continuous "flame" at the bottom
#define FIRE_SPARK_HEAT_MIN 100
#define FIRE_SPARK_HEAT_MAX 200

// If the animation is too quick, we can artificially slow it with some delay
#define FIRE_FRAME_DELAY_MS 4

#endif
