# Boomstick

## Overview

Boomstick is an Arduino project designed for powering a string of Adafruit
NeoPixels (LEDs), based on peripheral input from a microphone. The intended use
for this software is to create wearable or carryable devices that react to
music.

## Hardware

Suggested hardware is one or two NeoPixel LED strings, paired with an
Adafruit Pro Trinket 5V 16MHz, and Electret Microphone with Amplifier breakout.

## Usage

* Copy config.h.template to config.h, and configure the settings you desire.
  Many aspects of the animation can be tweaked.
* Upload sketch to the Pro Trinket, and wire up power, mic + LEDs to the
  appropriate pins.
