# Distributed Vision Visualization

## Overview
This is the ECE 4180 Final Project submitted by Joe Spall and Sarkis Ter Martirosyan. The goal for this project is to read multiple [April Tags](https://april.eecs.umich.edu/software/apriltag) using a [StereoPi](https://stereopi.com/) vision module to identify the location of those April Tags relative to the cameras. That estimation is transmitted wirelessly over TCP to a server hosted by a [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/?resellerType=home) that then displays the location of the April Tags on a HUB75 64x64 RGB Matrix. 

## Parts List
### Server/RGB-Matrix Module

| Part |
| ---- |
| [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/?resellerType=home) | 
| [Pi Cobbler+](https://www.adafruit.com/product/914) | 
| [Female-to-Male Jumper Wires](https://www.amazon.com/GenBasic-Solderless-Dupont-Compatible-Breadboard-Prototyping/dp/B01L5UKAPI/ref=sr_1_4?dchild=1&keywords=male+to+female+jumper+wires&qid=1605817419&sr=8-4) |
| [RGB Matrix Bonnet for Raspberry Pi](https://www.adafruit.com/product/3211) |
| [64x64 RGB Neopixel Display](https://www.adafruit.com/product/3649) |

### Client/Camera Module


| Part |
| ---- |
| TODO |

## Setup and Installation

### Server/RGB-Matrix Module

#### Wiring
The wiring on the client side is relatively straight forward. The Pi Zero W must be inserted into the Pi Cobbler+ so that the SD Card on the Raspberry Pi is aligned with the words ``Cobbler+'' on the Pi Cobbler PCB. From there, the Pi Cobbler+ must be hooked up to the RGB Matrix Bonnet so that the pins on the Raspberry Pi line up with the pins on the Bonnet as shown [here](https://learn.adafruit.com/assets/51032). 


### Client/Camera Module
TODO lmao
