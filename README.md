<div align="center">

[![Gaggiuino](/images/GAGGIUINO_LOGO_transp.png)](https://github.com/Zer0-bit/gaggiuino)
  
[![Compile Sketch](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml)
[![Join Discord](https://img.shields.io/discord/890339612441063494)](https://discord.gg/qh6Pm3b6HX "Join Discord Help Chat")
[![I'm on Reddit](https://img.shields.io/reddit/user-karma/combined/VSparxx?style=social)](https://www.reddit.com/user/VSparxx "I'm on Reddit")
[![License](https://img.shields.io/badge/License-GNU-blue)](/LICENSE "Go to license section")
</div>

## Intro
**Gaggiuino started as an idea to improve an already capable coffee machine while keeping the machine appearance and button functionality as close as possible to the original. An important part is that no internal cables/connectors were modified; all the connections were made by creating splitters using the purchased spade connectors.**
***
#### SUCCESSFUL INSTALLS:

   MODEL              |    YEAR   |  VOLTAGE   |PHOTOS
   -------------------|-----------|------------|--------
   Gaggia Classic | 2012 | 240v     | https://bit.ly/2Xv1fiv
   Gaggia Classic Pro | 2021 | 115V | https://imgur.com/gallery/SF3zP0b
   Gaggia CLassic Pro | 2020 | 240v | https://bit.ly/3jQESw3
   
***
#### Features / Completeness 

- [x] [Housing](https://www.thingiverse.com/thing:4949471) - **credits** [![I'm on Reddit](https://img.shields.io/reddit/user-karma/combined/LikeableBump1?style=social)](https://www.reddit.com/user/LikeableBump1 "I'm on Reddit")
- [x] UI/UX - **credits** [![I'm on Reddit](https://img.shields.io/reddit/user-karma/combined/different-wishbone81?style=social)](https://www.reddit.com/user/different-wishbone81 "I'm on Reddit")
- [x] Temp control
- [x] Realtime temp values update
- [x] Saving set values
- [x] Descale program
- [x] Steaming alert
- [x] Brew start sense
- [x] Pre-infusion
- [x] Auto pressure profiling
- [x] Manual pressure control
- [x] Auto shot timer
- [x] Regional power options
- [ ] Tray integrated load cells
- [ ] Realtime pressure feedback
- [ ] Steam control
- [ ] Water flow feedback
- [ ] Saving/Loading profiles
- [ ] Graphing shot stats

>Notes:
>* Pressure:
>     * It is expected your OPV has been tuned to 9bar by default.
>     * Initial pressure tuning has been performed according to a portafilter pressure gauge. While this is better than nothing and gives nice results, it's not real PP per se as the pressure at the puck can't be directly controlled. I have started looking for a suitable (food safe) pressure transducer which will allow for a feedback loop and real time dimmer control for a real PP experience.
>* Regional power:
>     * Defaults to 230V/50Hz and should be changed to the regional values for the correct work of the ACS712 sensor.

***
#### Planned future additions:
* Tray integrated load cells
   * Used for all things weight related
* Pressure transducer
   * Used for a feedback loop to make real-time pressure adjustments based on puck resistance
* Flow sensor
   * Used to monitor the water flow for true flow profiling
* Steam control
* STM32F411CE(blackpill) support
* ESP32 support:
   * Web interface
   * Graph outputs
   * OTA firmware updates
   * TBC

**[<< Ideas are welcome in the Issues tab >>](https://github.com/Zer0-bit/gaggiuino/issues)**
***
### Mod in operation:
**Brew mode:**
<div align="center">
 
https://user-images.githubusercontent.com/42692077/139340155-a3739359-33e1-423f-9ecf-10836f9446de.mp4
</div>

**Steaming mode:**
<div align="center">
  
https://user-images.githubusercontent.com/42692077/139340240-0602f792-8052-4ba4-96f8-e54d31add210.mp4
</div>


**Flush:**
<div align="center">

https://user-images.githubusercontent.com/42692077/139340295-698b1392-94cd-418b-8a63-222a4390aed5.mp4
</div>

***USAGE:***

 * BOILER      - sets the desired temperature at the boiler level
 * OFFSET      - sets the offset value used to calculate the real water temperature
 * HPWR        - sets the relay start pulse length
 * M.C.DIV     - sets the main cycle divider(aka non brew heating beahaviour), used in conjunction with HPWR
 * B.C.DIV     - sets the brew cycle divider
 * PREINFUSION - sets the preinfusion length and pressure
 * P-PROFILING - enables either AUTO or MAN pressure profiling mode

       AUTO     - sets the profiling trend incremental or decremental (can be combined with preinfusion), some examples bellow:
       MAN      - allows for manual pressure control at brew time
 * DESCALE     - enables the descaling program, at this point there's only one default behaviour:

         flush - 10s x5 at 2bar
         flush - 20s x5 at 1 bar
         idle  - 5min at 0 bar

***Pre-infusion[PI] and pressure profile[PP] usage examples:***

No| BREW MODE   | DESCRIPTION
---|--------------|--------------
1.| [PP] 9bar >> 6bar | the brew starts at 9 bars and stays at 9bar for 8 seconds then starts slowly dropping towards 6 bars,all this aims the official 30 seconds shot times, if the shot goes above 30s seconds the pressure will just remain at 6 bar untill the end of the shot.
2.| [PP] 6bar >> 9bar | same as above but viceversa.
3.| [PP] 6bar >> 6bar | the pressure stays locked at 6 bar.
4.| [PI] 8sec >> 2bar [PP] 9bar >> 5bar | preinfusion for 8 seconds at 2 bar >> pause 3 seconds (aka soaking) >> behaviour from point 1.



## **PROJECT REQUIREMENTS**

### **Software requirements:**
1. [Arduino IDE](https://www.arduino.cc/en/software)
    >*Needed to upload the code ".ino" to the arduino ROM*


   **Libraries to [add](https://www.arduino.cc/en/Guide/Libraries?setlang=en):**
   
         Library manager:
          - Easy Nextion Library
          - MAX6675 by Adafruit

         External libraries:
          - ACS712 > https://github.com/rkoptev/ACS712-arduino
          - RBDDimmer > https://github.com/RobotDynOfficial/RBDDimmer


2. [Nextion Editor](https://nextion.tech/nextion-editor/)
    >*Only necessary if planning on editing the ".HMI" file to ammend the LCD functionality*
3. [CH340 USB Driver](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
    >*USB driver so your system recognizes the Arduino clone board, let's say i have found this the hard way as apparetly the majority of cloned arduinos use a cheaper USB controller comparing to "genuino"*

### **Hardware requirements:**
###### The code has been designed to be plugable, meaning there is a minimal hardware configuration one can start with if certain features are not something of interest, next to the hardware component there will be anotated whether its a [BASE] or [EXT] functionality component type

* [Arduino Nano AT328](https://bit.ly/3eXSfXZ)  **[BASE]**
  * [Arduino Nano expansion board](https://www.aliexpress.com/item/32831772475.html?spm=a2g0o.store_pc_allProduct.8148356.21.7ed173b9bTMew3)
* [2.4" Nextion LCD](https://bit.ly/3CAUzPj)  **[BASE]**
* [MAX6675 thermocouple](https://bit.ly/3ejTUIj)  **[BASE]**
* [C-M4 screw K-Type thermocouple sensor](https://bit.ly/3nP1WMm)  **[BASE]**
* [40DA SSR Relay](https://bit.ly/33g1Pjr)  **[BASE]**
* [RobotDYN dimmer module - Dimmer 4A-400V ](https://bit.ly/3xhTwQy) **[EXT]**
* [Hall Current Sensor Module ACS712 - 20A](https://bit.ly/38MCiRv) **[EXT]**
* [Thermo-resistant cables AWG 15 and 18 ( 1m black/red ) and AWG 30 ( 5m black/red/yellow )](https://bit.ly/3tjSQbI) **[BASE]**
* [Spade connectors M/F 6.3mm](https://bit.ly/2Sjrkhu)
* Power Supply (multiple options provided choose what fits your needs): **[BASE]**
  * [5v AC Adaptor](https://bit.ly/3jWSrKa) 
  * [12v Power Supply](https://www.aliexpress.com/item/33012749903.html) should work only together with any of the bellow 2 options:
    * [12v to 5v stepdown USB output](https://www.aliexpress.com/item/32525175131.html)
    * [12v to 9v stepdown](https://www.aliexpress.com/item/1005001534605959.html)
 >*I used an old mobile charger I had lying around which conveniently had a USB port for plugging the cable that came with the  board, exactly like in the link above.
 >*It's recommended though to power the arduino board using  12V 1000mA power supply and a stepdown convertor of your choice from the provided list.*

### Housing:
 * [Gaggia Classic Arduino & Touchscreen Housing](https://www.thingiverse.com/thing:4949471)
     >Designed by the redditor [/u/LikeableBump1](https://www.reddit.com/r/gaggiaclassic/comments/phzyis/gaggia_with_arduino_pid_in_3d_printed_case/)

**Additional parts required to fully assemble housing:**
* [12v power supply](https://www.aliexpress.com/item/33012749903.html?spm=a2g0s.9042311.0.0.370a4c4dSNhQoq)
    >*This replaced the mobile charger that is recommended above. I still used a mobile charger with the casing removed to make it fit, but these should simplify things, and are cheap. Make sure you purchase the 12v 1000mA version.*
* [DB15 D-SUB connectors, male and female](https://www.aliexpress.com/item/1005002650762015.html?spm=a2g0o.productlist.0.0.6f0a1b50CsSmjP&algo_pvid=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b&aem_p4p_detail=202109282059464734316742000220004860154&algo_exp_id=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b-14&pdp_ext_f=%7B%22sku_id%22%3A%2212000021558768022%22%7D)
    >*These are used as a disconnectable pass-through to get the data and power cables through the back of the machine case. The 3d files include custom housings for these connectors. There will need to be 2 or 3 of the vertical "slats" cut out of the cooling vents on the machine case back to make these fit. Make sure you purchase both male and female DB15 connectors.*

**Additional notes for 3d printed housing:**
   >*If you use the standard .tft file attached above, the UI will be upside down, due to the way the screen has to be installed in the 3d printed housing. There is an additional .tft file for those using this housing, or you can open the .hmi file in the Nextion Editor yourself and flip it.*

**Optional:**
 * [Heat shrink](https://bit.ly/2PQdnqt)
 * [Soldering iron](https://bit.ly/3tijlOI) 
    >I bought it for my soldering needs, seems to do a great job. It's important to not leave it ON while not in use as the heating element will get damaged.

### Schematics :
* [GAGGIA Classic](schematics/gc-schematics.png)
* [GAGGIA Classic Pro](schematics/gcp-schematics.png)

***!! WARNING !!***
>*First and foremost please do not underestimate the danger of electricity or overestimate your ability to work around it. Only start working on your machine while it's  completely disconnected from the mains power socket, also by agreeing to follow the below guide I cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or gold fish and it will be entirely your fault!*

## Assembling
First we need to understand what goes where. The schematics aren't really rocket science but for someone who's never disasembled or has no experience working with electrical circuits it might get confusing really fast so I will try to describe it as simple as possible yet limited by my vocabulary.
First let's check that the setup works as expected while outside the machine so you don't have it all installed and realise just afterwards it's not reading any temperature because of a faulty component or the relay doesn't switch between the ON/OFF modes.

>**Note 1 - no permanent connections are needed during testing so no soldering needed for now.**
>
>**Note 2 - the 5v/GND Arduino board pins will be shared between all the connected devices.**

>***!! IMPORTANT !!***
>
>**All the boards should be powered by the arduino 5v/GND pins, taking power from other sources like an AC adapter will lead to the system working incorrectly.**
>
>**The AC adapter should be used only to provide power to the Arduino board itself by connecting to the VIN/GND ports.**

**BASE FUNCTIONALITY**
1. The first step will be connecting the MAX6675 module to the arduino board using the pins defined in the code. You can find them defined at the top of the .ino file.

    MAX6675  |  Arduino
    ---------|-----------
     VCC     |   5v
     GND     |   GND
     SCK     |   D6
     SO      |   D4
     CS      |   D5

2. Connect the relay. For now only connect the circuit controlling ports to check whether the relay LED indicates the power states.

    Relay   |  Arduino
    --------|-----------
      4     |   GND
      3     |   D8
      
   **Relay ports [1] and [2] are the high voltage circuit breaker**

4. Plug the arduino board in using the mini USB cable that came with it and upload the code to the arduino board. 
    >*Note: uploading won't work with the LCD connected*
5. Nextion LCD wiring

    Nextion  |  Arduino
    ---------|-----------
      TX     |   RX
      RX     |   TX
      VCC    |   5v
      GND    |   GND
      
6. Uploading the LCD ROM code

    **Method 1** 

        Just copy the *.tft file on a FAT32 formatted microSD card and upload onthe LCD panel using the onboard card reader

    **Method2** 

        Open the .HMI file using Nextion Editor and using the File menu upload it on a microSD card
     >*Note: card needs to be FAT32 formatted*

7. After the upload is finished get the card out and power cycle the LCD.
8. You should see temp readings on your screen if everything went according to plan.
    >*Don't forget to test the thermocouple/relay combo operation, apply some heat to the thermocouple end and see whether the relay led operates in HIGH/LOW modes*

**At this point if all the above works as expected you're ready to install it all inside the machine. For this we'll need to prepare some splitters that we'll use to connect to the Gaggia internals without introducing any permanent modifications so in the event of a desire to revert to stock it's a few disconnects away!**

**POWER DELIVERY RECOMMENDATION**

*Method 1:*
>*If choosing to power the system using the AC adapter then the arduino board and all the connected componnents will receive power by the means of the regulated 5v the AC adapter delivers through the USB port.*

*Method 2:*
>*If powering using the 12v power supply module follow the bellow scheme:*

   PS  |  Arduino
  -----|-----------
  12v  |   VIN
  GND  |   GND
 
 All the other boards will get their power from the arduino 5v / GND pins and it's extrmely important they are powered using those outputs.



**EXTENDED FUNCTIONALITY**

1. Adding the ACS712 hall current sensor is quite straight forward

    ACS712  |  Arduino
   ---------|-----------
      VCC   |   5v
      GND   |   GND
      OUT   |   A0

   **The high voltage circuit control ports will act as a passthrough for the front panel brew button positive wire**
      
2. Adding the dimmer

    Dimmer   |  Arduino
    --------|-----------
      VCC   |   5v
      GND   |   GND
      Z-C   |   D2
      PSM   |   D9
      
   **Dimmer high voltage circuit control ports will act as a passthrough for the pump positive circuit wire**


### Now I won't be explaining every single detail, as always with such projects common sense should be applied at all times, it's expected people doing such sort of modifications will have some basic understanding. ###

>*AGAIN!!! Triple check your machine is disconnected from any power sources, even better just pull the power cable out of it!*

#### BASE FUNCTIONALITY

1. Take off the top cover by unscrewing the 2 top screws. You should be able to see something similar to the below image minus the SSR relay:
<img src="https://db3pap006files.storage.live.com/y4m4pob4r1pDtjBPqIyA-dqHOH_eZDJaf6W2dYdHlIh8G8OWusXig9WUKOA-iBCk2QRN-lL3ajrWDDUBASx_frpWqz_2z1dxeAnksAKKysKqL-eXE9PVRYeA2SdmS_DSkAA3TJ5ZVe3ybpkLYV0-PDKLjEhxNZluA_UX8ektw8kGW4PXKQeQU-UUJtjuaDSYKsG?width=3496&height=4656&cropmode=none" width="769" height="1024" />

2. Prepare 2 splitters like in the below image using the AWG15 cable, be sure one splitter to be black(negative) and one red(positive)
<img src="https://db3pap006files.storage.live.com/y4mGdTrz4hXNuu3rvDk5qro2WGn5xqy8ZVGwhJSXFSDqmJErI8dYufS1H-l_PnIIa0HffKXPuPkvbRjNHt_2OogxaW8UohuFKatz3BfjjK8NEGmynX2unmeZ6opV3_gd-u0f3cCAlgh9nF5spGDt12McFxpxzsatrSK2YuRgrFTfFnxMvMmiXss0XSLrZGx5xIa?width=769&height=1024&cropmode=none" width="769" height="1024" />

3. Be sure to mark your top left power connector so you don't mix them up ( even though it's not that hard to understand which one is which)
<img src="https://db3pap006files.storage.live.com/y4mM3HVfjbCIdcPtx16eN0pEgGC4Z0ih04agqfBwI-NLtpUWvmlBFI23fT2LPVagGHXYsgRiIVq8oSjJckuZscCcdTKqq7GNxCK5GxRqsp2pCZinopGHCdqJtnZqMCFBlSq-yOT-vNtsATxE734AN05DO47i1as2NSW4E-87r78gV7kpP_sy9tHiBqey26s9xP0?width=769&height=1024&cropmode=none" width="769" height="1024" />

4. Disconnect all 3 of them as you'll use the midlle and bottom ones for power sharing.
<img src="https://db3pap006files.storage.live.com/y4m9fZvq6wrObA7QFSkvQpfFyOinQbfs-u_E8DLZqp784cPLcUIlZKoZeqbLWn3JPAElseiwa0G1KuN-yXJfOx_4N-3k-IU0YprmK-YHBYEG-33CaUXTEWVPNVwjy6y7kMyaDvbEzH445Su6hSKNqZqAPXmwJQlIH8lAQmXaTduAk3WIsUrZSw2J0lPhRRVEqzX?width=769&height=1024&cropmode=none" width="769" height="1024" />

5. The hardest part will be now in my opinion as you'll have to unscrew the bottom boiler stock thermostat and screw back in the new thermocoule. Be sure to apply some thermal paste on the thermocouple threads. (Just a teeny tiny bit.)
<img src="https://db3pap006files.storage.live.com/y4m9lHvLCobQ3DlmTfqYPJvV1M-z2T0_GMxgeE21-hqP3nfu5UwB1PTyPF6rIT5A6ebeyv9TXwEfCbB1SBwkyVy0r8AIbrLGogaWcebzjSpaskThwbn-S45n5E-IAkeKPJgYAjLMPYeQPjALL--D1CC60ZFaOGsaHtAOSNvcb1Y4X2IEWV7n4bUP4v40sKawusJ?width=496&height=660&cropmode=none" width="768" height="1024" />

6. Prepare 2 cables you'll use to connect the cables disconnected from the thermostat to the port 1 and 2 of the SSR relay. Use the red cable for that. They shouldn't be too long, about 10cm will suffice, one end should be crimpled with a male spade connector and the loose end screwed to the relay and attach the relay to the machine case itself, I have no clue what screw size I used as i have just matched one by trial and error lol, but be sure to apply some thermal paste to the SSR backplate that will make contact with the metal case of the machine.
<img src="https://db3pap006files.storage.live.com/y4m3ZmhHEobr2VPen1wScEO8AMLiaOVfWjsnW8Aw4645PaRXerrLpV8iZmZjzU88KDPmKEfPXbUBqYhn6ejVOfXIB7hgucmljyOLL54tqWKUahFoqnYEf2rBh0TRXjFEtzAvptdT3W8nSTzc96sfvD8hcK-pQEeEaYaLaq4gpNaKyXdWQtETpPtrzpMd1mLzuyb?width=2584&height=2140&cropmode=none" width="769" height="769" /> 

So you end up having them connected like this:
<img src="https://db3pap006files.storage.live.com/y4m5HfjPvkFSOtSm8gkTJJ4saqEw8dcB1no2eoaHcDZjYD9OV95tBnrG8tqtRuOcor7aFZrIRw7167k4QGMUneOPATitBFctkgdklxWOohMyBir3zLdm9fkAnTQW8TquTZouRh9rzKSC0t5bkerK1g8AzlFYTfuZoDPQ3juFiOJ19JKiL6VpODn40Z1q8JwVpGy?width=2625&height=2831&cropmode=none" width="769" height="769" />

7. Prepare 2 more 10cm cables( also colour coded accordingly) which should be crimpled with M/F spade connectors at each end, the female end will go in Gaggia's front panel and the male end will go to the female end of the previously prepared (2) splitters, as in the below image:
<img src="https://db3pap006files.storage.live.com/y4m2LoENSwBicOmRtvdMKM9srnf4tfNECBkaNXWkkGxxEAMbc_BuQhQnYgVH7h0FZ52NIDiIudlx2NhDkm1747Y9wT60F7uoHMJu-lx_MF-mPXBbzOyeZNVuEdrISfsi56v7VYKNZIgby3qUD922gMJCI177q0IttLhWXn_VM9OamG0FvyKQ3T26uqOye6H50eV?width=769&height=1024&cropmode=none" width="769" height="1024" />
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />

8. Connect the front panel cables to any of the free male ends of the splitters as well, black one for the negative cable and red for the positive one (on my machine the positive is having 2 cables crimped together).

9. To power the Arduino system I have used an old 5v mobile charger which I'm sure all of us have laying around. Just solder 2 cables to the 2 ends of the charger and for the other ends use 2 F spade connectors, after which plug them to the remaining 2 splitter (2) ends.
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />

10. Now you're ready to connect everything to the arduino like you did it when testing everything, one piece of advice would be to solder all the Arduino connected cables as during the machine operation there is quite a bit of vibration and that can introduce all sorts of noise/frequent disconnects to certain pins which will lead to unexplained behaviours.


#### EXTENDED FUNCTIONALITY

1. While installing the ACS712 Hall current sensor please note in the photo below the way the sensor faces the camera and how the cable passthrough is done with the cable originally connected to the middle slot front panel brew button (1). It's a short cable connecting the brew button with the steam button, we leave it connected to the steam button but the end which was connected to the middle slot of the brew button connects now to the top port of the ACS712 board (1) and then the exit (2) port of the ACS712 board  feeds a cable to the original front panel brew button position.

>**It is important to connect the cables properly as the sensor has a polarity and when incorrectly connected outputs a negative value!
>Another VERY important aspect is to have the board connected just as shown in the picture (location and position) as well as only the necessary cables connected to the sensor board itself, no additional wires(high or low voltage) should be positioned anywhere around the hall effect sensor as this will most likely create unexpected reads that will confuse the program. A easy way to understand whether your program has false readings is if the shot timer keeps starting randomly when the machine is in idle state.**

<img src="https://db3pap006files.storage.live.com/y4mDAGJwdsJP2Vv-Z4FCeTheZSWYlCj09fbWdURcj2t_qeDla_UaWY3qT23MZKsIiQtbH-d7TOS4nRDypYBrDUeFm7eEsL2LidREYBNIylJlCSIntsNItisdsSFSfqbPvGYsZDJ-dJ7_uKYD1gjYabCTC99pB-EVuW7BvWW2v7ut7aK4NLarH9iTjjNRDBpAc88?width=768&height=1024&cropmode=none" width="768" height="1024" />

2. Installing the RobotDYN dimmer module.
<img src="https://db3pap006files.storage.live.com/y4mC6f3vykKE40Z3FnrvrwriXgztoGaPAV_PoeIreyugKLhfK58o9KyQo1mANet1k-s0EZ2Shp6a1P6dEv13bQBIGwZKyxHMuIoPMJTO4GjAzESpHOXRNeBMtV5rZ8qGRWoPKVo13ahvh0V9hAYXGdPs5Zp7KpXnCSwjA2xiSqODEEy0y9d5zVJRxJAL1ovjVgB?width=1920&height=1080&cropmode=none" width="810" height="520" />

>Once installed tests can be done to verify the dimmer output is the expected one, this will require having either a [portafilter gauge](https://www.amazon.co.uk/Portafilter-Pressure-Espresso-Machines-ESPRESS/dp/B00ONTGKNA) or an [internally mounted gauge](https://www.ebay.com/itm/401980168959?hash=item5d97e29aff:g:fugAAOSwzkRd4~Jl).
