# Boomstick

## Overview

Boomstick is an Arduino project designed for powering a string of Adafruit
NeoPixels (LEDs), based on peripheral input from a microphone. The intended use
for this software is to create wearable or carryable devices that react to
music.

## Features

* FFT detects bass frequencies in order to react to music beats.
* Reasonably sophisticated transformation algorithms attempt to adapt to
  changes in volume over time, in order to function the same regardless of
  how loud your environment is.
* Multiple effects! Use a button wired to a digital input in order to change
  effects on the fly.
* Software-level brightness control using an analog input. Connect up a 10K
  potentiometer, and change brightness while in operation.

## Hardware

Suggested hardware is one or two NeoPixel LED strings, paired with an
Adafruit Pro Trinket 5V 16MHz, and Electret Microphone with Amplifier breakout.
It's recommended NOT to use a mic with automatic gain control.

## Usage

* Copy config.h.template to config.h, and configure the settings you desire.
  Many aspects of the animation can be tweaked.
* Upload sketch to the Pro Trinket, and wire up power, mic and LEDs to the
  appropriate pins. Optionally hook up a potentiometer and button for extra
  controls.
