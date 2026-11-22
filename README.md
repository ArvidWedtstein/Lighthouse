<a name="readme-top"></a>

<div align="center">

[![Contributors][contributors-shield]][contributors-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]

</div>
<div align="center">

[![Created][repocreated-shield]][repo-url]

</div>

<br />
<div align="center">
  <a href="https://github.com/ArvidWedtstein/Lighthouse">
    <img width="140" height="256" alt="Lighthouse" src="https://github.com/user-attachments/assets/1b690146-a83d-4ef8-9bc1-7e6615903704" />
  </a>

  <h3 align="center">Lighthouse</h3>

  <p align="center">
    Source code for Lighthouse in my garden, powered by an Ardunio Uno
    <br />
    <a href="https://github.com/ArvidWedtstein/Lighthouse"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/ArvidWedtstein/Lighthouse/issues">Report Bug 🪲</a>
    ·
    <a href="https://github.com/ArvidWedtstein/Lighthouse/issues">Request Feature</a>
  </p>
</div>

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
      </ul>
    </li>
    <li>
      <a href="#usage">Usage</a>
      <ul>
        <li><a href="#cables">Cables</a></li>
        <li><a href="#lighthouse-settings">Lighthouse Settings</a></li>
        <li><a href="#lighthouse-status">Lighthouse Status</a></li>
      </ul>
    </li>
    <li><a href="#contributing">Contributing</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->

## About The Project

Garden Lighthouse.
About 2 meter tall and 1 meter wide.
Features a rotating light on top and some interior lights which turn on at dusk and off after a set time.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

Lighthouse is built with the following innovative technologies:

- Wood
- PLA Plastic
- Arduino Uno
- Car Battery for power
- Geared DC Motor
- DRV8871 Motor driver
- DS3231 RTC for time control

<!-- GETTING STARTED -->

## Getting Started

To get a local copy up and running follow these simple steps.

### Prerequisites

To run this project you'll need to Ardunio IDE or VSCode with Platformio installed. You can download the Ardunio IDE from [here](https://www.arduino.cc/en/software) and Platformio from [here](https://platformio.org/install/ide?install=vscode).

Instructions TBA...

### Deploying / Uploading to lighthouse

Using Platformio, build project first and then upload to the Ardunio Uno. Make sure to select the correct board and port in Platformio.

You might need to install the CH341 driver for your computer to recognize the board. You can download the driver from [here](http://www.wch.cn/download/CH341SER_EXE.html).

If you have a cheap chinese clone of the Ardunio Uno, you will have to press reset button on the board then platformio is on "uploading" step.

<!-- USAGE EXAMPLES -->

## Usage

The Lighthousse itselves selves as distraction for bypassing ships.

### Testing

Testing is done manually.

### Cables

#### Light sensor

- Yellow -> 5V
- Brown -> GND
- Yellow -> Out (A0)

#### Cheap 128x64 oled Display

- Gray -> V1 (GND)
- Purple -> V2 (5V)
- Brown -> SDA (A4)
- White -> SCL (A5)

#### Mode Button

- Green -> D2
- Green -> GND

#### 10k Potentiometer

- Unknown -> Middle pin (A1)
- Unknown -> Left pin (5V)
- Unknown -> Right pin (GND)

#### DS3231 RTC

- Orange -> VCC (5V)
- Black -> GND
- Brown -> SDA (A4)
- White -> SCL (A5)

#### Relay

- Brown -> GND
- Orange -> VCC (5V)
- Green -> IN (D6)

#### DRV8871 Motor driver

- Unknown -> GND (GND)
- Unknown -> VM (12V)
- Unknown -> IN1 (D9)
- Unknown -> IN2 (GND)

### Lighthouse Settings

The lighthouse features a 4 digit 7-segment display and a single button for user interaction. The display shows the current duration setting in minutes, which can be adjusted using a potentiometer. The button is used to enter settings mode and to confirm changes.

- 1. Btn press: Enter settings mode earlies start time edit. (Lighthouse cannot turn on before this time)
- 2. Btn press: screen swaps to edit duration time
- 3. Btn press: Screen swaps to show off time.

The lighthouse operates based on which of these happens first.

### Lighthouse Status

The built in Arduino UNO has a onboard status light.
This light will light up depending on several states.
If several faults are active, only the highest-priority one is shown (2, then 3, then 4).
Unexpected restarts show up as the 5-flash pattern, so a random reset will be visible if you're watching.

| LED Pattern                   | Description                                                                                                     |
| ----------------------------- | --------------------------------------------------------------------------------------------------------------- |
| 5 rapid flashes, then a pause | The board just started or restarted                                                                             |
| 1 very short blink every ~8s  | Everything is fine (heartbeat)                                                                                  |
| 2 blinks every cycle          | Clock problem: the RTC isn't responding, or it lost power and its time was reset                                |
| 3 blinks every cycle          | Sensor problem: it never saw light for the whole 12-hour cooldown, so it may be stuck, covered, or disconnected |
| 4 blinks every cycle          | The OLED isn't responding on the I2C bus                                                                        |

<!-- CONTRIBUTING -->

## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this project better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! <3

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->

[repo-url]: https://github.com/ArvidWedtstein/Lighthouse
[contributors-shield]: https://img.shields.io/github/contributors/ArvidWedtstein/Lighthouse.svg?style=for-the-badge
[contributors-url]: https://github.com/ArvidWedtstein/Lighthouse/graphs/contributors
[stars-shield]: https://img.shields.io/github/stars/ArvidWedtstein/Lighthouse.svg?style=for-the-badge
[stars-url]: https://github.com/ArvidWedtstein/Lighthouse/stargazers
[repocreated-shield]: https://img.shields.io/github/created-at/ArvidWedtstein/Lighthouse?label=Created
[issues-shield]: https://img.shields.io/github/issues/ArvidWedtstein/Lighthouse.svg?style=for-the-badge
[issues-url]: https://github.com/ArvidWedtstein/Lighthouse/issues
