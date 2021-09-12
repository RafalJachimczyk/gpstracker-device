# GPS tracker based on SIM800C GSM module, L80R GPS and MMA8452QR1 Accelerometer

## TL;DR;

This is the main repository containing documentation and code for the GPS tracker device I have built during the 2019-2020. I am not a professional electronic designer, however I was studing electronics during my college years. Professionally I am a Software Engineer with experience in cloud computing, XP practicess and modern programming languages (Node.js and Go). To lern more about me check out [my CV](https://rafaljachimczyk.github.io/markdown-cv/).

This is NOT a production ready product. There are no guarantees nor support is to be expected from myself when you try and build this yourself. I am happy to answer questions though. 

This repo contains only the C++ code for the device itself. Check the other repositories for [iOS app]() and the [Google Cloud functions]() required to run the full project.

## Project goals and intentions

Myself and my partner are a proud "parents" of a lovely dog called Russel. Russel loves to chase various animals that are present in the forests nearby our house. One day on a walk, to our rising panic levels, Russel dissappeared from our sight chasing a deer. We have discussed purchasing a GPS collar for him, but that would be too easy. So I set of on a never ending project to build a GPS tracker, small enough to fit on his collar. 

After a solid year of engineering this is what it looked like : 

[Image of the GPS tracker]

## GPS tracker requirements

- [ ] Small enough to fit on a dog collar
- [ ] Mobile app to see the device on the map
- [ ] Cloud based backend infrastructure
- [ ] Enough battery life to run for 2 days
- [ ] Send additional telemetry for debugging (battery charge, gsm signal strength)
- [ ] Two update modes
    - Continouos (update as fast as possible, when device is moving and app was used within last 5 minutes)
    - Every minute (standard when moving)
- [ ] Sleep mode when the device is not moving

## High level design

![High Level diagram explaining the solution](docs/images/high-level.drawio.png)

The design is not overly complex. The GPS tracker sends the data using Google's [Protocol Buffers](https://developers.google.com/protocol-buffers) over HTTPS. I chose this format as I believed it would make the payloads smaller (binary format) and be lighter on the battery life. The PB endoded data is sent directly to [GCP function](https://cloud.google.com/functions) (equivalent of AWS Lambdas) written in Go, which in turn stores the data in a database. 

I could have gone for gRPC instead of HTTPS but that added extra complexity. I aimed for simplicity for the prototype. 

On the mobile side (iOS app), the code queries another GCP function (also go) using HTTPs, which queries the database. 

## Schematics

![Main board schematics](docs/images/Main.png)

![Accelerometer schematics](docs/images/Accelerometer.png)

![Test Pins](docs/images/Test-Pins.png)

I will discuss each part of the schematics to explain design decisions I have made, and the constraints I was working with. 

## Power Supply

The GPS tracker is powered by a standard LiPo battery which operates between 3.2v to 4.2v. Any lower and the battery will be irreversably damaged, any higher - the battery might burst into flames. 

The charging circuitry is built in into the device itself. For this I have chosen the trusted TP4056 iC, allowing for up to 1A charge current. Additionally we have battery protection IC - the FS312F.

I also wanted to be able to tell the current usage. For this I have used the ds2782e+ fuel gauge iC. 

The device operates on the 3.3V logic levels.
I have used the TPS77801D low drop out voltage regulator (it somes with a handy status output on pin 8, to indicate low input voltage - we can use this to indicate low battery level and swithch off parts of system via MCU). The LDO delivers the power to all but one part of the system - the GSM module is powered directly from the battery, as it has massive peak current usage when communicating with GSM networks. Also the Sim800c operates exactly in the voltage range of the LiPo battery.


### MCU

As eplained at the very beginning of this document, I am not a professional electronics engineer so you will likely find huge design flaws in here :-) I have chosen Atmega 644P-20AU because it contains 2 hardware serial ports. Another reason for this was that the only previous experience I had was with Arduinio and ESP8266/32 boards. I know there are ARM mcus that would probably be much better for this job. 

This was the biggest constraint initially for me. The GPS and GSM modules use serial interface to communicate with the MCU. I wanted to be able to see Serial Monitor output as well, to debug the device - that would be a third serial port! 

The design decision I made was to use the two hardware ports of Atmega644 for GPS and Serial. The GSM would have to do with SoftwareSerial.


# Software

[] say something about lessons learned, Interrupts, RTOS