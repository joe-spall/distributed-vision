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
git checkout server 
```

From there you can start the server running by executing the file [final_server.py](https://github.com/joe-spall/distributed-vision/blob/server/rpi-rgb-led-matrix/bindings/python/samples/final_server.py) located in the directory `distributed-vision/rpi-rgb-led-matrix/bindings/python/samples/final_server.py`. Change the variable `my_ip` in `final_server.py` to reflect the actual IP of the Raspberry Pi Zero W on your local network. To run the server properly for a 64x64 RGB Matrix, you need to use this bash command:

```bash
sudo ./final_server.py -r 64 --led-cols 64
```

After this, the server should be up and waiting for a connection from the client. 

#### Photos
These are some sample photos of our setup. 

| Example Photos |
| -------------- |
| [Power Supply Brick](https://photos.google.com/share/AF1QipNsIqyeEHIrXT9VP8A1IKr34ptzQHY_JPHF02mI6lNcXrIQbD_YwltlO2YAApMyZw/photo/AF1QipOaRkycxnqihjz6Owt76IpKrcNicT8PogM8LFIy?key=M2FtRUh1cS1XQUVLdS1ZVDlhTDZ0bGZsQkJxM1FB) |
| [Raspberry Pi Zero W](https://photos.app.goo.gl/KHfsGGmuaw87fhBf7) |
| [Raspberry Pi connected to RGB Matrix Bonnet](https://photos.app.goo.gl/cqnCgESpTngZwZeS9) |
| [RGB Matrix Bonnet](https://photos.google.com/share/AF1QipMsmtNqdN-5M4sXHGo1j6Kf1vuD9z6663KYCGvuELj4ML6NbWCxB8GrgW7piCADYg/photo/AF1QipNQNMJdxwCsRYsph6B9GGWgMzhMmoAychx4W5IL?key=YVVzVjRDWVJlb01Eay1IWDVFVE8zVno3c2xxSTFn) |

### Client/Camera Module
TODO lmao

## Demonstration

For the demonstration, here are several videos that highlight the system tracking three April Tags in real-time. 

| Views | 
| ----- |
| [Dot Matrix Cam](https://youtu.be/hsFkJoQ_Yo8) |
| [Client Screen Capture Cam](https://youtu.be/FzzDqCPymg8) |
| [Physical April Cam Movement Cam](https://youtu.be/cZ3JV5UcapI) |
