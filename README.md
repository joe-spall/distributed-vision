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
The wiring on the client side is relatively straight forward. The Pi Zero W must be inserted into the Pi Cobbler+ so that the SD Card on the Raspberry Pi is aligned with the words ``Cobbler+'' on the Pi Cobbler PCB. From there, the Pi Cobbler+ must be hooked up to the RGB Matrix Bonnet so that the pins on the Raspberry Pi line up with the pins on the Bonnet as shown [here](https://learn.adafruit.com/assets/51032). Connect the 64x64 RGB Display to the Bonnet as shown in the tutorial for whatever RGB display you buy. The final step is to solder pins 4 and 18 on the RGB Matrix Bonnet.  

#### Code
The first step in setting up the code is to follow the installation instructions for the Bonnet library shown [here](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi?view=all#driving-matrices). When it comes to select between quality and convenience, please select the quality option. From there clone in the server branch of this repository. You can get this branch onto your Raspberry Pi Zero W server by running these commands: 

```bash
git clone https://github.com/joe-spall/distributed-vision.git
git branch 
```


### Client/Camera Module
TODO lmao
