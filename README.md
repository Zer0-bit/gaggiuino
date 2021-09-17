# GAGGIUINO #
[![Compile Sketch](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml)

<!-- <img src = "/images/PXL_20210525_144343463.jpg" width = 500 height =700> -->
Gaggiuino started as an idea to improve an already cappable coffee machine while keeping the machine looks and buttons functionality as close as possible to the original, an important part is that no internal cables/connectors were modified, all the connections were made by creating splitters using the purchased spade connectors.

#### SUCCESSFUL INSTALLS:

   MODEL              |    YEAR   |  VOLTAGE   |PHOTOS
   -------------------|-----------|------------|--------
   Gaggia Classic | 2012 | 240v     | https://bit.ly/2Xv1fiv
   Gaggia Classic Pro | 2019 | 115V | https://imgur.com/gallery/SF3zP0b
   

#### Features / Completeness 

- [x] Housing - **credits @LikeableBump**
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

>Note on pressure:
>1. It is expected your OPV has been tuned to 9bar by default.
>2. Pressure isn't tuned yet due to the absence of a gauge, it's been ordered and we're at the delivery services' mercy, for the time being pressure was calculated according to water flow.

##### [<< Ideas are welcome in the Issues tab >>](https://github.com/Zer0-bit/gaggiuino/issues)

#### As part of considerably upgrading the project i've taken it upon me to bring a new UI as well, hopefully nicer than the old one:
<img src="https://db3pap006files.storage.live.com/y4mVSHlwib9lZho2UFEGaXuhEy_-udrWos8DhK9nqyGQbwSvrOUYjv0LZC3yKIGTdN1iR0A8a5hdYz6-Xq12ut8ZEBgyX5ga4AsGR9KCVaGB8NN4OetwCOO4OxejVb2VB2eXID9eFXlfLk30IR4-x88ahRgCHiXFpZhCvF7N6RTa_V8Uh0mz0ByTsM76aumAk7s?width=401&height=302&cropmode=none" width="401" height="302" />  <img src="https://db3pap006files.storage.live.com/y4mY1e7N9LI7HrylwhK6UReqIxJXkKPND5DaiWfiknTKQuUzYqXZDKLgqplNqilHISaFH-QAAGJbktzicEAb4Hzlu03lZQJTZv_Z849SpBdbVdS6dDaSIiNDkVCVGQ6KQ1-_iFIB0gye7JCwzCQ6J2RUkAPzPMgIuxQLiYKbJMxbBW1a3ZPr5P7OkaIkm1XBiuU?width=402&height=302&cropmode=none" width="402" height="302" />  <img src="https://db3pap006files.storage.live.com/y4mWHmyHeLZ_L2xH19oGMsCGnuIv1RTazI_KQWbprL7pSqFU8Hkb0r0GE6DU_kXJyRB9CkVyu0Uv8ZRK7mrzjWs1xqoLwWV2aUYg_Dt-iz-a5Osed4kXEoBT9FxsEVseMKzKibCoxANqcue7C7dT6lJzMe5TsiJ7M_WIWUiXtkoA_OKYHJZGg_0x2S8tA08sNni?width=401&height=305&cropmode=none" width="401" height="305" />  <img src="https://db3pap006files.storage.live.com/y4mL4BllGyh_Zqmajbi48A0v5Zwq5kEkGuf3bEk9v4wNJTjE865qde3oRMdm6bkKzcLFfRc5jlNGNKY0gOIFiQOqJxk-EmjlBqpJKqsN8JbwDtH3rEQJd85wrrb9nsEbG3JkxYRRcEPvgpkHg8EKZcLH7vfzxPzhIL5Q1BG8GtKr6TiMD9PMcyh8-j7xkyNOJ7Z?width=403&height=302&cropmode=none" width="403" height="302" />  <img src="https://db3pap006files.storage.live.com/y4mcMSHHBr1BWmGxc_6W-2UpFuMFAnS2qKIDRM_qVqnGfqbO6dEQzzVYEK9mweS209LblIZYIfRJQ53zNb35vcmkHp6ptnzkVEc_Q07CKfqluRRFNFSwJgjOB1ahEyNbMdulsc7pyxBUmZeEzQrrF2DjHRMNWyvMqGkX-CcffIWk_qDQlQJGxxtjhn1L1a-2uF-?width=402&height=302&cropmode=none" width="402" height="302" />  <img src="https://db3pap006files.storage.live.com/y4mWqoKtmta3flPxER2MCJEdT1bMzWQ9oUce4_vr1rBbFThnBdSGI5qrsblWxVnfNoIkkb-FRfdP0ZxzX_WwS5rd7RuUD2sOhz3Wnj7U1y7QVEIzlj5T3C_khz-vyR9iCoruiC4CDr6AEYJL5FKZUT-F2_LQz4uWZJXvR4Ze8ylmlNx5NXFXP4RPUmFgVKbe1lb?width=402&height=305&cropmode=none" width="402" height="305" />

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



# **PROJECT REQUIREMENTS**

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
3. [CH340 USB Driver](https://electropeak.com/learn/how-to-install-ch340-driver/)
    >*USB driver so your system recognizes the Arduino clone board, let's say i have found this the hard way as apparetly the majority of cloned arduinos use a cheaper USB controller comparing to "genuino"*

### **Hardware requirements:**
###### The code has been designed to be plugable, meaning there is a minimal hardware configuration one can start with if certain features are not something of interest, next to the hardware component there will be anotated whether its a [BASE] or [EXT] functionality component type

1. [Arduino Nano](https://bit.ly/3eXSfXZ)  **[BASE]**
    >*Be sure to buy the AT328 version, also if you're buying the mini usb version buy the cable from them, you'll save yourself a lot of problems.
2. [Nextion touch screen](https://bit.ly/33dO8RV)  **[BASE]**
3. [MAX7765 thermocouple](https://bit.ly/3ejTUIj)  **[BASE]**
4. [C-M4 screw K-Type thermocouple sensor](https://bit.ly/3nP1WMm)  **[BASE]**
5. [40DA SSR Relay](https://bit.ly/33g1Pjr)  **[BASE]**
6. [RobotDYN dimmer module - Dimmer 4A-400V ](https://bit.ly/3xhTwQy) **[EXT]**
7. [Hall Current Sensor Module ACS712 - 20A](https://bit.ly/38MCiRv) **[EXT]**
8. [Thermo-resistant cables AWG 15 ( 1m black/red ) and AWG 30 ( 1m black/red/yellow )](https://bit.ly/3tjSQbI) **[BASE]**
9. [Spade connectors M/F 6.3mm](https://bit.ly/2Sjrkhu)
10. [5v Power Adaptor](https://bit.ly/2WPHGkg) **[BASE]**
    >*I used an old mobile charger i had lying around which conveniently had a USB port for plugging the cable that came with the  board, exactly like in the link above.*

**Optional:**
 * [Heat shrink](https://bit.ly/2PQdnqt)
 * [U-shaped Parallel Terminal](https://bit.ly/3h8phaF)  - These are in case you're choosing not to solder
 * [Soldering iron](https://bit.ly/3tijlOI) - the one i bought for my soldering needs,  seems to do a great job
 * [Nextion LCD case](https://bit.ly/3b2nxf7)
 * [Magnet sheet](https://bit.ly/3xKTMJn)


 ## The housing for all of the above hardware has been designed and provided kindly by @LikeableBump and can be found on thingiverse by following the link below:
 
   [Gaggia Classic Arduino & Touchscreen Housing](https://www.thingiverse.com/thing:4949471)

***!! WARNING !!***
>*First and foremost please do not underestimate the danger of electricity or overestimate your ability to work around it, only start working on your machine while it's  completely disconnected from the mains power socket, also by agreeing to follow the below guide i cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or gold fish and it will be entirely your fault!*

# Assembling #
First we need to understand what goes where, the schematics aren't really rocket science but for someone who's never disasembled or has no experience working with electrical circuits it might get confusing real fast so i will try to describe it as simple as possible yet limited by my vocabulary.
First let's check the setup works as expected while outside the machine so you don't have it all installed and realise just afterwards it's not reading any temperature because of a faulty component or the relay doesn't switch between the ON/OFF modes.

>**Note 1 - no perament connections are needed during testing so no soldering needed for now.**
>
>**Note 2 - the 5v/GND Arduino board pins will be shared between all the connected devices.**

**BASE FUNCTIONALITY**
1. The first step will be connecting the MAX6675 module to the arduino board using the pins defined in the code, you can find them defined at the top of the .ino file.

    MAX6675  |  Arduino
    ---------|-----------
     VCC     |   5v
     GND     |   GND
     SCK     |   CLK
     SO      |   DO
     CS      |   CS

2. Connect the relay, for now only connect the circuit controlling ports to check whether the relay LED indicates the power states.

    Relay   |  Arduino
    --------|-----------
      3     |   GND
      4     |   D8
      
   **Relay ports [1] and [2] are the high voltage circuit breaker**

4. Plug the arduino board using the mini usb cable that came with it and upload the code to the arduino board. 
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

7. After upload finished get the card out and power cycle the LCD.
8. You should see temp readings on your screen if everything went according to the plan.
    >*Don't forget to test the thermocouple/relay combo operation, apply some heat to the thermocouple end and see whether the relay led operates in HIGH/LOW modes*

**At this point if all the above works as expected you're ready to install it all inside the machine, for this we'll need to prepare some splitters that we'll use to connect to the Gaggia internals without introducing any permanent modifications so in the event of a desire to revert to stock it's a few disconnects away!**


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


### Now i won't be explaining every single detail, as always with such projects common sense should be applied at all times, it's expected people doing such sort of modifications will have some basic understanding. ###

>*AGAIN!!! Tripple check your machine is disconnected from any power sources, even better just pull the power cable out of it !*

#### BASE FUNCTIONALITY

1. Take off the top cover by unscrewing the 2 top screws, you should be able to see smth similar to the below image minus the SSR relay:
<img src="https://db3pap006files.storage.live.com/y4m4pob4r1pDtjBPqIyA-dqHOH_eZDJaf6W2dYdHlIh8G8OWusXig9WUKOA-iBCk2QRN-lL3ajrWDDUBASx_frpWqz_2z1dxeAnksAKKysKqL-eXE9PVRYeA2SdmS_DSkAA3TJ5ZVe3ybpkLYV0-PDKLjEhxNZluA_UX8ektw8kGW4PXKQeQU-UUJtjuaDSYKsG?width=3496&height=4656&cropmode=none" width="769" height="1024" />

2. Prepare 2 splitters like in the below image using the AWG15 cable, be sure one splitter to be black(negative) and one red(positive)
<img src="https://db3pap006files.storage.live.com/y4mGdTrz4hXNuu3rvDk5qro2WGn5xqy8ZVGwhJSXFSDqmJErI8dYufS1H-l_PnIIa0HffKXPuPkvbRjNHt_2OogxaW8UohuFKatz3BfjjK8NEGmynX2unmeZ6opV3_gd-u0f3cCAlgh9nF5spGDt12McFxpxzsatrSK2YuRgrFTfFnxMvMmiXss0XSLrZGx5xIa?width=769&height=1024&cropmode=none" width="769" height="1024" />

3. Be sure to mark your top left power connector so you don't mix them up ( even though it's not that hard to understand which one is which)
<img src="https://db3pap006files.storage.live.com/y4mM3HVfjbCIdcPtx16eN0pEgGC4Z0ih04agqfBwI-NLtpUWvmlBFI23fT2LPVagGHXYsgRiIVq8oSjJckuZscCcdTKqq7GNxCK5GxRqsp2pCZinopGHCdqJtnZqMCFBlSq-yOT-vNtsATxE734AN05DO47i1as2NSW4E-87r78gV7kpP_sy9tHiBqey26s9xP0?width=769&height=1024&cropmode=none" width="769" height="1024" />

4. Disconnect all 3 of them as you'll use the midlle and bottom ones for power sharing.
<img src="https://db3pap006files.storage.live.com/y4m9fZvq6wrObA7QFSkvQpfFyOinQbfs-u_E8DLZqp784cPLcUIlZKoZeqbLWn3JPAElseiwa0G1KuN-yXJfOx_4N-3k-IU0YprmK-YHBYEG-33CaUXTEWVPNVwjy6y7kMyaDvbEzH445Su6hSKNqZqAPXmwJQlIH8lAQmXaTduAk3WIsUrZSw2J0lPhRRVEqzX?width=769&height=1024&cropmode=none" width="769" height="1024" />

5. The hardest part will be now in my opinnion as you'll have to unscrew the bottom boiler stock thermostat and screw back in the new thermocoule, be sure to apply some thermal paste on the thermocouple threads ( just a teeny tiny bit ).
<img src="https://db3pap006files.storage.live.com/y4m9lHvLCobQ3DlmTfqYPJvV1M-z2T0_GMxgeE21-hqP3nfu5UwB1PTyPF6rIT5A6ebeyv9TXwEfCbB1SBwkyVy0r8AIbrLGogaWcebzjSpaskThwbn-S45n5E-IAkeKPJgYAjLMPYeQPjALL--D1CC60ZFaOGsaHtAOSNvcb1Y4X2IEWV7n4bUP4v40sKawusJ?width=496&height=660&cropmode=none" width="768" height="1024" />

6. Prepare 2 cables you'll use to connect the cables disconnected form the thermostat to the port 1 adn 2 of the SSR relay, use the red cable for that they shouldn't be too long, about 10cm will suffice, one end should be crimpled wiht a male spade connector and the loose end screwed to the relay and attach the relay to the machine case itself, ihave no clue what screw size i used as i have just matched one by trial and error lol, but be sure to apply some thermal paste to the SSR backplate that will make contact with the metal case of the machine.
<img src="https://db3pap006files.storage.live.com/y4m3ZmhHEobr2VPen1wScEO8AMLiaOVfWjsnW8Aw4645PaRXerrLpV8iZmZjzU88KDPmKEfPXbUBqYhn6ejVOfXIB7hgucmljyOLL54tqWKUahFoqnYEf2rBh0TRXjFEtzAvptdT3W8nSTzc96sfvD8hcK-pQEeEaYaLaq4gpNaKyXdWQtETpPtrzpMd1mLzuyb?width=2584&height=2140&cropmode=none" width="769" height="769" /> 

So you end up having them connected like this:
<img src="https://db3pap006files.storage.live.com/y4m5HfjPvkFSOtSm8gkTJJ4saqEw8dcB1no2eoaHcDZjYD9OV95tBnrG8tqtRuOcor7aFZrIRw7167k4QGMUneOPATitBFctkgdklxWOohMyBir3zLdm9fkAnTQW8TquTZouRh9rzKSC0t5bkerK1g8AzlFYTfuZoDPQ3juFiOJ19JKiL6VpODn40Z1q8JwVpGy?width=2625&height=2831&cropmode=none" width="769" height="769" />

7. Prepare 2 more 10cm cables( also colour coded accordingly) which should be crimpled with M/F spade connectors at each end, the female end will go in Gaggia's front panel and the male end will go to the female end of the previously prepared(2) splitters, as in the below:
<img src="https://db3pap006files.storage.live.com/y4m2LoENSwBicOmRtvdMKM9srnf4tfNECBkaNXWkkGxxEAMbc_BuQhQnYgVH7h0FZ52NIDiIudlx2NhDkm1747Y9wT60F7uoHMJu-lx_MF-mPXBbzOyeZNVuEdrISfsi56v7VYKNZIgby3qUD922gMJCI177q0IttLhWXn_VM9OamG0FvyKQ3T26uqOye6H50eV?width=769&height=1024&cropmode=none" width="769" height="1024" />
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />

8. Connect the front panel cables to any of the free male ends of the splitters as well, black one for the negative cable and red for the positive one( on my machine the positive is having 2 cables crimpled together).

9. To power the arduino system i have used an old 5v mobile charger which i'm sure all of us have laying around, just solder 2 cables to the 2 ends of the charger and for the other ends use 2 F spade connectors, after which plug them to the remaining 2 splitter(2) ends.
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />

10. Now you're ready to connect everything to the arduino like you did it when testing everything, one advice would be to solder all the arduino connected cables a during the machine operation there is quite a bit of vibration and that can introduce all sorts of noise/frequent disconnects to certain pins which will lead to unexplained behaviours.


#### EXTENDED FUNCTIONALITY

1. While installing the ACS712 Hall current sensor please note in the photo below the way the sensor faces the camera and how the cable passthrough is done with the cable originall connected to the middle slot front panel brew button (1), it's a short cable connecting the brew button with the steam button, we leave it connected to the steam button but the end which was connected to the middle slot of the brew button connects now to the top port of the ACS712 board (1) and then the exit(2) port of the ACS712 board  feeds a cable to the original front panel brew button position.

>**!It is important to conect the cables properly as the sensor has a polarity and when incorrectly connected outputs a negative value**

<img src="https://db3pap006files.storage.live.com/y4mDAGJwdsJP2Vv-Z4FCeTheZSWYlCj09fbWdURcj2t_qeDla_UaWY3qT23MZKsIiQtbH-d7TOS4nRDypYBrDUeFm7eEsL2LidREYBNIylJlCSIntsNItisdsSFSfqbPvGYsZDJ-dJ7_uKYD1gjYabCTC99pB-EVuW7BvWW2v7ut7aK4NLarH9iTjjNRDBpAc88?width=768&height=1024&cropmode=none" width="768" height="1024" />

2. Installing the RobotDYN dimmer module.
<img src="https://db3pap006files.storage.live.com/y4mC6f3vykKE40Z3FnrvrwriXgztoGaPAV_PoeIreyugKLhfK58o9KyQo1mANet1k-s0EZ2Shp6a1P6dEv13bQBIGwZKyxHMuIoPMJTO4GjAzESpHOXRNeBMtV5rZ8qGRWoPKVo13ahvh0V9hAYXGdPs5Zp7KpXnCSwjA2xiSqODEEy0y9d5zVJRxJAL1ovjVgB?width=1920&height=1080&cropmode=none" width="810" height="520" />
