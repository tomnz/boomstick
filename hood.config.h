#ifndef CONFIG
#define CONFIG

//////////////////////////
// LED HARDWARE SETTINGS

// LED strand data pin
#define LED_PIN     6

// Number of pixels in strand
#define N_PIXELS    27

// When enabled, uses N_PIXELS * 2 pixels on the data line, mirrored in the
// center
#define MIRROR_DUPE

// When enabled, reverses the ordering of the LED output
#define FLIP

// "End" of the string - one past the end so the peak dot can move off scale
#define TOP         (N_PIXELS + 2)

// When defined, uses an analog signal to vary the overall brightness
#define BRIGHTNESS_PIN 4

// Which effect to use on startup - if no button is present, then this
// is the only effect which is used
//   0 = Bars
//   1 = Pulse
#define INITIAL_EFFECT 0

// When defined, enables a button on a digital input to change effects
#define EFFECT_BUTTON_PIN 8

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

// Historic FFT frames to keep to determine high/low range
#define HISTORIC_FRAMES 40
#define NOISE_THRESHOLD 5
#define LEVEL_CUTOFF 30

/////////////////////////
// BAR EFFECT SETTINGS

// Color for the minimum intensity bar
#define MIN_COL     70
// Color for the maximum intensity bar
#define MAX_COL     255
// Derived color range between min/max
#define COL_RANGE   (MAX_COL - MIN_COL)
// Color difference between lower and upper parts of bar
#define COL_VAR     60

#define BAR_SCALE   1.4
#define MIN_BAR_SIZE 70

// Rate at which peak dot falls
#define PEAK_FALL_RATE 0.15

// Background color settings
#define BACKGROUND_COLOR 80
#define BACKGROUND_CUTOFF (N_PIXELS / 3)
#define BACKGROUND_MAX 0.2
#define BACKGROUND_INCREASE 0.001
#define BACKGROUND_DECREASE 0.01

#define HISTORIC_SCALE 1.2

///////////////////////////
// PULSE EFFECT SETTINGS

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

#endif
