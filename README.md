Robotic Firmware
================
Named due to its purpose as a the robotic firmware for my final year project this currently drives 2WD vehicles.

Uses an Arduino Shield, a Particle Shield Shield and a Particle Photon.

The Dagu 2WD robotic kit has been been used as a chassis, along with a 6AA battery holder to get the 8-9V that the 
two 4.5V motors require.

One 6dB 2.4GHz omni directional antenna has been attached to improve WiFi connection, using the u.FL connector on the Photon.

Current Inventory
=================

Currently the following items are 'in stock' (can be used):

* 1 x Dagu 2WD chassis

* 1 x 6 battery pack (AA)

* 32 x AA batteries

* 3 x Arduino Motor Shields

* 2 x Photon Shield Shields

* 3 x Photons

* 1 x SR 04 USonic Sensor

* 1 x MPU 6050 accelerometer

* 30 x male to male jumper cables

* 2 x breadboards

* 1 x SMA Antenna (2.4GHz)

* 1 x u.FL to SMA cable

Building and Flashing
=====================

The firmware can be built using the Makefiles supplied, this requires a Linux distribution and the setup, this requires the *'build-essentials'* package, and if running on a 64-bit machine then *'lib32ncurses5'* and *'lib32z1'* may be required. GCC must also be installed as this is the compiler used throughout the firmware project. The command *'make program'* will build the software as required and load it onto a Particle Device in DFU-MODE connected by USB to the machine. These steps are discussed within this [guide](https://github.com/spark/firmware/blob/develop/docs/gettingstarted.md "Guide").