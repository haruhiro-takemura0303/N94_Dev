# NXP Application Code Hub
[<img src="https://mcuxpresso.nxp.com/static/icon/nxp-logo-color.svg" width="100"/>](https://www.nxp.com)

## Demo of mikroe oledBclick in FRDM with CMSIS driver and GPIO adapter

Demo is an example for Mikroe OLED B click module in FRDM boards usign CMSIS driver for I2C comunication, and GPIO component.

Mikroe OLED B click demo is in [This repository](https://github.com/nxp-appcodehub/dm-mikroe-oledBclick-frdm)
and is supported for many boards.
### Specific boards
- FRDM MCXC041
- FRDM MCXC242
- FRDM MCXC444
- FRDM MCXA153
- FRDM MCXA156
- FRDM MCXN236
- FRDM MCXN947
- FRDM MCXW71
- FRDM RW612
#### Boards: FRDM-MCXN947, FRDM-RW612, FRDM-MCXW71, FRDM-MCXA153, FRDM-MCXA156, FRDM-MCXC041, FRDM-MCXC242, FRDM-MCXC444, FRDM-MCXN236
#### Categories: Graphics, HMI, User Interface
#### Peripherals: GPIO, I2C, UART
#### Toolchains: MCUXpresso IDE, VS Code

## Table of Contents
1. [Software](#step1)
2. [Hardware](#step2)
3. [Setup](#step3)
4. [Results](#step4)
5. [FAQs](#step5) 
6. [Support](#step6)
7. [Release Notes](#step7)

## 1. Software<a name="step1"></a>
- [MCUXpresso 11.10.0 or newer.](https://nxp.com/mcuxpresso)
- [MCUXpresso for VScode 24.8.9 or newer](https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/general-purpose-mcus/lpc800-arm-cortex-m0-plus-/mcuxpresso-for-visual-studio-code:MCUXPRESSO-VSC?cid=wechat_iot_303216)
- [SDK for FRDM board](https://mcuxpresso.nxp.com/en/select)

## 2. Hardware<a name="step2"></a>
- [FRDM-MCXN947](https://www.nxp.com/design/design-center/development-boards-and-designs/general-purpose-mcus/frdm-development-board-for-mcx-n94-n54-mcus:FRDM-MCXN947)   
[<p align="center"><img src="../Common/Images/MCXN947.png" width="400"/></p>](../Common/Images/MCXN947.png)
- [MIKROE OLED-B-CLICK](https://www.mikroe.com/oled-b-click)   
[<p align="center"><img src="../Common/Images/OledBClick.png" width="100"/></p>](../Common/Images/OledBClick.png)


## 3. Setup<a name="step3"></a>

### 3.1 Plug Mikroe Joystick-2-click in FRDM-MCXN947
[<p align="center"><img src="../Common/Images/plug_MCXN947.png" width="400"/></p>](../Common/Images/plug_MCXN947.png)<br>
### 3.2 Upload code in FRDM-MCXN947

## 4. Results<a name="step4"></a>
    - Open serial monitor with next configuration
        - Baudrate:  115200
        - Parity:    None
        - StopBits:  1
    - Reset FRDM-MCXN947 with SW1
    - Serial monitor should shows next
[<p align="center"><img src="../Common/Images/Terminal.PNG" width="500"/></p>](../Common/Images/Terminal.PNG)<br>
OLED screen should show next
[<p align="center"><img src="../Common/Images/oledBclick.gif" width="200"/></p>](../Common/Images/oledBclick.gif)

### Convert Images
Demo includes python script to resize and convert ever image to binary for the screen.<br>
- Run script on CMD and type the root and name of image to convert.<br>
[<p align="center"><img src="../Common/Images/script_results.PNG" width="500"/></p>](../Common/Images/script_results.PNG)
- Script generates .c file with the image array.<br>
[<p align="center"><img src="../Common/Images/file_generated.PNG" width="500"/></p>](../Common/Images/file_generated.PNG)
#### Project Metadata

<!----- Boards ----->
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXN947-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;RW612-blue)](https://www.nxp.com/pip/RW612)
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXW71-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXA153-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXA156-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXC041-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXC242-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXC444-blue)]()
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXN236-blue)]()

<!----- Categories ----->
[![Category badge](https://img.shields.io/badge/Category-GRAPHICS-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+graphics+in%3Areadme&type=Repositories)
[![Category badge](https://img.shields.io/badge/Category-HMI-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+hmi+in%3Areadme&type=Repositories)
[![Category badge](https://img.shields.io/badge/Category-USER%20INTERFACE-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+ui+in%3Areadme&type=Repositories)

<!----- Peripherals ----->
[![Peripheral badge](https://img.shields.io/badge/Peripheral-GPIO-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+gpio+in%3Areadme&type=Repositories)
[![Peripheral badge](https://img.shields.io/badge/Peripheral-I2C-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+i2c+in%3Areadme&type=Repositories)
[![Peripheral badge](https://img.shields.io/badge/Peripheral-UART-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+uart+in%3Areadme&type=Repositories)

<!----- Toolchains ----->
[![Toolchain badge](https://img.shields.io/badge/Toolchain-MCUXPRESSO%20IDE-orange)](https://github.com/search?q=org%3Anxp-appcodehub+mcux+in%3Areadme&type=Repositories)
[![Toolchain badge](https://img.shields.io/badge/Toolchain-VS%20CODE-orange)](https://github.com/search?q=org%3Anxp-appcodehub+vscode+in%3Areadme&type=Repositories)

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Warning**: For more general technical questions regarding NXP Microcontrollers and the difference in expected functionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/X-Follow%20us%20on%20X-black.svg)](https://x.com/NXP)

## 7. Release Notes<a name="step7"></a>
| Version | Description / Update                           | Date                        |
|:-------:|------------------------------------------------|----------------------------:|
| 1.0     | Initial release on Application Code Hub        | September 23<sup>rd</sup> 2024 |

<small>
<b>Trademarks and Service Marks</b>: There are a number of proprietary logos, service marks, trademarks, slogans and product designations ("Marks") found on this Site. By making the Marks available on this Site, NXP is not granting you a license to use them in any fashion. Access to this Site does not confer upon you any license to the Marks under any of NXP or any third party's intellectual property rights. While NXP encourages others to link to our URL, no NXP trademark or service mark may be used as a hyperlink without NXP’s prior written permission. The following Marks are the property of NXP. This list is not comprehensive; the absence of a Mark from the list does not constitute a waiver of intellectual property rights established by NXP in a Mark.
</small>
<br>
<small>
NXP, the NXP logo, NXP SECURE CONNECTIONS FOR A SMARTER WORLD, Airfast, Altivec, ByLink, CodeWarrior, ColdFire, ColdFire+, CoolFlux, CoolFlux DSP, DESFire, EdgeLock, EdgeScale, EdgeVerse, elQ, Embrace, Freescale, GreenChip, HITAG, ICODE and I-CODE, Immersiv3D, I2C-bus logo , JCOP, Kinetis, Layerscape, MagniV, Mantis, MCCI, MIFARE, MIFARE Classic, MIFARE FleX, MIFARE4Mobile, MIFARE Plus, MIFARE Ultralight, MiGLO, MOBILEGT, NTAG, PEG, Plus X, POR, PowerQUICC, Processor Expert, QorIQ, QorIQ Qonverge, RoadLink wordmark and logo, SafeAssure, SafeAssure logo , SmartLX, SmartMX, StarCore, Symphony, Tower, TriMedia, Trimension, UCODE, VortiQa, Vybrid are trademarks of NXP B.V. All other product or service names are the property of their respective owners. © 2021 NXP B.V.
</small>