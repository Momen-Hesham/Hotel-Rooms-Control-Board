# Hotel-rooms-control-board

## Table of Contents

* [Description](#Description)
* [Project In Details](#Project-In-Details)
* [Software and Hardware Used](#software-and-Hardware-Used)
* [Schematic](#Schematic)
* [Youtube Video](#Video) 

## Description
This project aims to design a control boardusing Tiva C to lock and unlock a solenoid lock. it gets its input from the hotel receptionist's PC through UART or from the Guest using Keypad.
The project's goal is to apply many concepts like GPIO, UART, KeyPad, SysTick Timer ... etc. 
Code was not Professional enough as it was my first project in Embedded Systems. 

## Project-In-Details
there are 4 Modes of Operations:
  1-Setup Mode.
  here The Board Store number of rooms via UART connection with PC.
  2-Statuc of The Room.
  Then receptionist enter the status of the choosen room from 3 states:
    -Free (presented by entering number 0).
    -Occupied (presented by entering number 2).
    -Room Cleaning (presented by entering number 1).
 3-Set Password by UART connection with PC (in case choosing Occupied status).
  4-Open the Door by Guest by entering the password using KeyPad.
 
 Here to test and simulate the project i choose the room number 5 to be controlled by KeyPad but you can change this number if you want.
## Software-and-Hardware-Used
This project is completly wrriten in C and run and tested at Keil.
To run and test this project you only must have Tiva c, Keypad,1 Solenoid"or 2 Leds to test ", and a Pc to act like the receptionist using UART terminal PORT.

## Schematic
![Project Design](/Design.png)

## Video

[![](http://img.youtube.com/vi/Iv2p9JPx5JY/0.jpg)](http://www.youtube.com/watch?v=Iv2p9JPx5JY "Hotel Room's Control Board")
