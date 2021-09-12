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

