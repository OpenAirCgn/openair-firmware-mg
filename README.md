[![Build Status](https://travis-ci.com/OpenAirCgn/openair-firmware-mg.svg?branch=master)](https://travis-ci.com/OpenAirCgn/openair-firmware-mg)
# OpenAir Firmware

## Overview

This project contains the firmware to operate the ESP32 based OpenAir board and some peripherals.

## Building

This firmware requires Mongoose-OS tools. The Mongoose tools require docker to be installed.

Installation instructions are available [ here ]( https://mongoose-os.com/docs/mongoose-os/quickstart/setup.md#1-download-and-install-mos-tool )

After installing the `mos` tools, it should just be a matter of calling:

````
mos build --platform esp32 --local --no-libs-update
````

to build, followed by:

````
mos flash
````

with a connected ESP32 device.
