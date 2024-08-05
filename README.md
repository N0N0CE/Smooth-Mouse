![alt text](https://github.com/N0N0CE/Smooth-Mouse/blob/master/Smooth%20Mouse/Smooth%20Mouse.ico)

This Windows program aims to give a good smoothing to the mouse input. To do so it syncs the mouse's refresh rate to the monitor's one, 
then it applies a simple decay function to the mouse's positions.

## State of the project:

This program is in a very crude state, it has not GUI and require editing a config file to change it's parameters.

## Requirements:

- A mouse !
- A Windows system (10 is working at least)
- The Interception driver (https://github.com/oblitum/Interception)
- Maybe some other unknown requirements ?

## Installation:
You need to install the Interception driver for this program to work, see https://github.com/oblitum/Interception

Unzip the release binaries into your folder of choice.

## Usage:
Edit the config.cfg values to your liking.

Start the executable and have a smooth day !

### Here are the config parameters explained:

start_on (1 or 0) Define if the smoothing starts right away or not.

keep_pressed (1 or 0) Define if the button you set to activate the smoothing is to be kept pressed or if it behaves as a toggle.

activate_with_mouse (1 or 0) Define if the mouse's buttons can be used to activate the smoothing.

activate_mouse_code (some code number*) Define the mouse button that will act as the smooth button.

activate_with_keyboard (1 or 0) Define if the keyboard's key can be used to activate the smoothing.

activate_key_code (some code number*) Define the keyboard key that will act as the smooth button.

decay_factor (between 0 and <1) Define the decay strengh. 0 would be no smoothing, near 1 would be a very strong one. Pratical values are around 0.9

(*) To find the key or button code, press it when the program is running, it should be ouputed to the console window.

## Notes:

The Interception driver is a low level driver, please evaluate the security vulnerabilities it could open for your use case.

The mouse's syncing works by tracking a dummy directX window. (That was ChatGPT's sugestion) 
You must not close that window or minimize it if you want the smoothing to work.
Full screen softwares shouldn't prevent this from working though.

Please keep your parameters's value range correct, the program do not checks them.

Currently the smoothing will not be consistent if the refresh rate of the monitor is variable.

The Nice And Clean Code committee rated the source code as hazardous materials, but it's there.
