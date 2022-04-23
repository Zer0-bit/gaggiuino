
## **REQUIREMENTS**

### **BOM:**
***The code has been designed to be plugable, meaning there is a minimal hardware configuration one can start with if certain features are not something of interest, it's all appropriately split under "BASE" or "EXTENDED" functionality (see bellow).***

#### **BASE FUNCTIONALITY** 
***
* [Arduino Nano AT328](https://bit.ly/3eXSfXZ)
  * [Arduino Nano expansion board](https://www.aliexpress.com/item/32831772475.html?spm=a2g0o.store_pc_allProduct.8148356.21.7ed173b9bTMew3)
* [2.4" Nextion LCD](https://bit.ly/3CAUzPj)
* [MAX6675 thermocouple](https://bit.ly/3ejTUIj) 
* [C-M4 screw K-Type thermocouple sensor](https://bit.ly/3nP1WMm)
* [40DA SSR Relay](https://bit.ly/33g1Pjr)
* [Thermo-resistant cables AWG15 and AWG20 ( 1m black/red ) and AWG26 ( 5m black/red/yellow/blue )](https://bit.ly/3tjSQbI)
* [Spade connectors M/F 6.3mm](https://bit.ly/2Sjrkhu)
* Power Supply (multiple options provided choose what fits your needs):
  * [5v AC Adaptor](https://bit.ly/3jWSrKa) 
  * [12v Power Supply](https://www.aliexpress.com/item/33012749903.html) and [12v to 5v stepdown](https://www.aliexpress.com/item/1005001821783419.html) (these two should only be used together)
 >*It's recommended to power the arduino board using  12V 1000mA power supply and the stepdown convertor.*

#### **EXTENDED FUNCTIONALITY**
***
* [RobotDYN dimmer module - Dimmer 4A-400V ](https://bit.ly/3xhTwQy)
* [1-Bit AC 220V Optocoupler](https://www.aliexpress.com/item/1005003228104606.html)
* [Pressure sensor - 0-1.2Mpa](https://www.aliexpress.com/item/4000756631924.html)
  * [Fitting - 6-02/PCF ](https://www.aliexpress.com/item/4001338642124.html)
  * [Fitting - 6mm/PE](https://www.aliexpress.com/item/4001338085412.html)
  * [Hose - 6x4 - 1 meter](https://www.aliexpress.com/item/4000383354010.html)
 
### **Software requirements:**
1. [Arduino IDE](https://www.arduino.cc/en/software)
    >*Needed to upload the code ".ino" to the arduino ROM*


   **Libraries to [add](https://www.arduino.cc/en/Guide/Libraries?setlang=en):**
   
         Library manager:
          - Easy Nextion Library
          - MAX6675 by Adafruit
          - TimerInterrupt_Generic


         External libraries:
          - PSM > https://github.com/banoz/PSM.Library
          - HX711 > https://github.com/banoz/HX711


2. [Nextion Editor](https://nextion.tech/nextion-editor/)
    >*Only necessary if planning on editing the ".HMI" file to ammend the LCD functionality*
3. [CH340 USB Driver](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
    >*USB driver so your system recognizes the Arduino clone board, let's say i have found this the hard way as apparetly the majority of cloned arduinos use a cheaper USB controller comparing to "genuino"*

### Housing variants:
 * [Gaggiuino v1](https://www.thingiverse.com/thing:4949471)
 * [Gaggiuino v2](https://www.thingiverse.com/thing:5236286)

**Optional additional parts to use with the V1 housing type:**
* [DB15 D-SUB connectors, male and female](https://www.aliexpress.com/item/1005002650762015.html?spm=a2g0o.productlist.0.0.6f0a1b50CsSmjP&algo_pvid=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b&aem_p4p_detail=202109282059464734316742000220004860154&algo_exp_id=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b-14&pdp_ext_f=%7B%22sku_id%22%3A%2212000021558768022%22%7D)
    >*These are used as a disconnectable pass-through to get the data and power cables through the back of the machine case. The 3d files include custom housings for these connectors. There will need to be 2 or 3 of the vertical "slats" cut out of the cooling vents on the machine case back to make these fit. Make sure you purchase both male and female DB15 connectors.*


### Schematics & diagrams:
**Schematics:**
* [GAGGIA Classic](https://user-images.githubusercontent.com/42692077/161397293-82df427a-2ac2-4226-bdc6-fa831a962265.png)

**Diagrams:**
* [GAGGIA Classic](https://user-images.githubusercontent.com/42692077/160548957-88c93198-6d81-4081-8db6-552b6f6c5281.png)

***
***!! WARNING !!***
>*First and foremost please do not underestimate the danger of electricity or overestimate your ability to work around it. Only start working on your machine while it's  completely disconnected from the mains power socket, also by agreeing to follow the below guide I cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or gold fish and it will be entirely your fault!*

## Assembling
First we need to understand what goes where. The schematics aren't really rocket science but for someone who's never disassembled or has no experience working with electrical circuits it might get confusing really fast so I will try to describe it as simple as possible yet limited by my vocabulary.
First let's check that the setup works as expected while outside the machine so you don't have it all installed and realise just afterwards it's not reading any temperature because of a faulty component or the relay doesn't switch between the ON/OFF modes.

>**Note 1 - no permanent connections are needed during testing so no soldering needed for now.**
>
>**Note 2 - the 5v/GND Arduino board pins will be shared between all the connected devices.**

#### **ARDUINO CONNECTIONS**
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
    >
    >*Due to the way the Arduino design works as well as the nature of how the on-board ADC functions in order to avoid a loop in regards to the **BREW MODE** it's required to connect the pins A0 -> GND, this is only needed if the ACS712 sensor board is not connected to the Arduino.*

**At this point if all the above works as expected you're ready to install it all inside the machine. For this we'll need to prepare some splitters that we'll use to connect to the Gaggia internals without introducing any permanent modifications so in the event of a desire to revert to stock it's a few disconnects away!**

**POWER DELIVERY RECOMMENDATION**

*Method 1:*
>*If choosing to power the system using the AC adapter then the arduino board and all the connected componnents will receive power by the means of the regulated 5v the AC adapter delivers through the USB port.*

*Method 2:*
>*If powering using the [ 12v ] power supply module + [ 5v ] stepdown convertor follow the bellow scheme:*

   PS  |  Arduino
  -----|-----------
   5v  |   5v
  GND  |   GND

*** 
**STEAM DETECTION**

  ***Gaggia Classic:***
  
  1. Disconnect all the steam switch high voltage wires
  2. Bridge the wires connected to the steam switch poles 1 and 2.
  3. Bridge the steam thermostat wires.
  4. Connect steam switch poles 3 and 4 to the arduino nano like in the table bellow using some AWG26 wires.
  5. Make sure you secure all the left disconnected wires so they don't make any accidental contact.

![GC - steam handling](https://user-images.githubusercontent.com/42692077/147904917-feeb3230-3ea0-460c-82de-4ad16034a48e.png)

GC SWITCH| Arduino
---------|-----------
   3     |   D7
   4     |   GND

***
**BREW DETECTION** 

  _Will look slightly different for people using a **GC** vs those with a **GCP**_

  **GAGGIA CLASSIC:**

   OPTOCOUPLER|  Arduino
   ---------|-----------
   VCC   |   5v
   GND   |   GND
   OUT   |   A0
   
  _The high voltage circuit control ports will splice into existing brew switch wires._
  
***
**DIMMER**

  Dimmer  |  Arduino
  --------|-----------
  VCC   |   5v
  GND   |   GND
  Z-C   |   D2
  PSM   |   D9
  
_Dimmer high voltage circuit control ports will act as a passthrough for the pump LIVE and NEUTRAL wires_
   
***
**PRESSURE HANDLING**

  Transducer|  Arduino
  ----------|-----------
   RED      |   5v
   BLACK    |   GND
   YELLOW   |   A1


***
**ATTENTION !!!**

**As always with such projects common sense should be applied at all times, it's expected people doing such sort of modifications will have some basic understanding.**

>_AGAIN!!! Triple check your machine is disconnected from any power sources, even better just pull the power cable out of it if you haven't done so yet!_

#### BASE FUNCTIONALITY
***
1. Take off the top cover by unscrewing the 2 top screws. You should be able to see something similar to the below image minus the SSR relay:
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m4pob4r1pDtjBPqIyA-dqHOH_eZDJaf6W2dYdHlIh8G8OWusXig9WUKOA-iBCk2QRN-lL3ajrWDDUBASx_frpWqz_2z1dxeAnksAKKysKqL-eXE9PVRYeA2SdmS_DSkAA3TJ5ZVe3ybpkLYV0-PDKLjEhxNZluA_UX8ektw8kGW4PXKQeQU-UUJtjuaDSYKsG?width=3496&height=4656&cropmode=none" width="769" height="1024" />
</div>

2. Prepare 2 splitters like in the below image using the AWG15 cable, be sure one splitter to be black(negative) and one red(positive)
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4mGdTrz4hXNuu3rvDk5qro2WGn5xqy8ZVGwhJSXFSDqmJErI8dYufS1H-l_PnIIa0HffKXPuPkvbRjNHt_2OogxaW8UohuFKatz3BfjjK8NEGmynX2unmeZ6opV3_gd-u0f3cCAlgh9nF5spGDt12McFxpxzsatrSK2YuRgrFTfFnxMvMmiXss0XSLrZGx5xIa?width=769&height=1024&cropmode=none" width="769" height="1024" />
</div>

3. Be sure to mark your top left power connector so you don't mix them up ( even though it's not that hard to understand which one is which)
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4mM3HVfjbCIdcPtx16eN0pEgGC4Z0ih04agqfBwI-NLtpUWvmlBFI23fT2LPVagGHXYsgRiIVq8oSjJckuZscCcdTKqq7GNxCK5GxRqsp2pCZinopGHCdqJtnZqMCFBlSq-yOT-vNtsATxE734AN05DO47i1as2NSW4E-87r78gV7kpP_sy9tHiBqey26s9xP0?width=769&height=1024&cropmode=none" width="769" height="1024" />
</div>

4. Disconnect all 3 of them as you'll use the midlle and bottom ones for power sharing.
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m9fZvq6wrObA7QFSkvQpfFyOinQbfs-u_E8DLZqp784cPLcUIlZKoZeqbLWn3JPAElseiwa0G1KuN-yXJfOx_4N-3k-IU0YprmK-YHBYEG-33CaUXTEWVPNVwjy6y7kMyaDvbEzH445Su6hSKNqZqAPXmwJQlIH8lAQmXaTduAk3WIsUrZSw2J0lPhRRVEqzX?width=769&height=1024&cropmode=none" width="769" height="1024" />
</div>

5. The hardest part will be now in my opinion as you'll have to unscrew the bottom boiler stock thermostat and screw back in the new thermocoule. Be sure to apply some thermal paste on the thermocouple threads. (Just a teeny tiny bit.)
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m9lHvLCobQ3DlmTfqYPJvV1M-z2T0_GMxgeE21-hqP3nfu5UwB1PTyPF6rIT5A6ebeyv9TXwEfCbB1SBwkyVy0r8AIbrLGogaWcebzjSpaskThwbn-S45n5E-IAkeKPJgYAjLMPYeQPjALL--D1CC60ZFaOGsaHtAOSNvcb1Y4X2IEWV7n4bUP4v40sKawusJ?width=496&height=660&cropmode=none" width="768" height="1024" />
</div>

6. Prepare 2 cables you'll use to connect the cables disconnected from the thermostat to the port 1 and 2 of the SSR relay. Use the red cable for that. They shouldn't be too long, about 10cm will suffice, one end should be crimpled with a male spade connector and the loose end screwed to the relay and attach the relay to the machine case itself, I have no clue what screw size I used as i have just matched one by trial and error lol, but be sure to apply some thermal paste to the SSR backplate that will make contact with the metal case of the machine.
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m3ZmhHEobr2VPen1wScEO8AMLiaOVfWjsnW8Aw4645PaRXerrLpV8iZmZjzU88KDPmKEfPXbUBqYhn6ejVOfXIB7hgucmljyOLL54tqWKUahFoqnYEf2rBh0TRXjFEtzAvptdT3W8nSTzc96sfvD8hcK-pQEeEaYaLaq4gpNaKyXdWQtETpPtrzpMd1mLzuyb?width=2584&height=2140&cropmode=none" width="769" height="769" /> 
</div>

So you end up having them connected like this:
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m5HfjPvkFSOtSm8gkTJJ4saqEw8dcB1no2eoaHcDZjYD9OV95tBnrG8tqtRuOcor7aFZrIRw7167k4QGMUneOPATitBFctkgdklxWOohMyBir3zLdm9fkAnTQW8TquTZouRh9rzKSC0t5bkerK1g8AzlFYTfuZoDPQ3juFiOJ19JKiL6VpODn40Z1q8JwVpGy?width=2625&height=2831&cropmode=none" width="769" height="769" />
</div>

7. Prepare 2 more 10cm cables( also colour coded accordingly) which should be crimpled with M/F spade connectors at each end, the female end will go in Gaggia's front panel and the male end will go to the female end of the previously prepared (2) splitters, as in the below image:
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4m2LoENSwBicOmRtvdMKM9srnf4tfNECBkaNXWkkGxxEAMbc_BuQhQnYgVH7h0FZ52NIDiIudlx2NhDkm1747Y9wT60F7uoHMJu-lx_MF-mPXBbzOyeZNVuEdrISfsi56v7VYKNZIgby3qUD922gMJCI177q0IttLhWXn_VM9OamG0FvyKQ3T26uqOye6H50eV?width=769&height=1024&cropmode=none" width="769" height="1024" />
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />
</div>

8. Connect the front panel cables to any of the free male ends of the splitters as well, black one for the negative cable and red for the positive one (on my machine the positive is having 2 cables crimped together).

9. To power the Arduino system I have used an old 5v mobile charger which I'm sure all of us have laying around. Just solder 2 cables to the 2 ends of the charger and for the other ends use 2 F spade connectors, after which plug them to the remaining 2 splitter (2) ends.
<div align="center">
<img src="https://db3pap006files.storage.live.com/y4myMKUSADo1HIGEXQ42p9tP1UKzL2aUqI6gCv3st6cBqR921Y-xWkhHB9QYaUlubJC-wCs5swyMaXX-p9LJu0qDgOgMKwkMyW-KUdUkkQWZ7-VdJNZiWv2duaBEcFtGo34uX1_-mqF66PpgseniGFKGhJmO-o5n8Pb2TP2it0vyQBcLAgX00jzVl-H6L5NeVzE?width=769&height=1024&cropmode=none" width="769" height="1024" />
</div>

10. Now you're ready to connect everything to the arduino like you did it when testing everything, one piece of advice would be to solder all the Arduino connected cables as during the machine operation there is quite a bit of vibration and that can introduce all sorts of noise/frequent disconnects to certain pins which will lead to unexplained behaviours.


#### EXTENDED FUNCTIONALITY
***
1. **BREW DETECTION** 

**Gaggia Classic(pre 2018) ONLY**

![GC - brew handling](https://user-images.githubusercontent.com/42692077/154805193-76068521-3ad4-4020-b2ee-8dab9394d4fe.png)

  * Prepare two **"Y"** splitters **BLACK** and **RED** with the ends crimped as follows: 
     * **RED SPLITTER:** 1 x **MALE**, 1 x **FEMALE**, 1 x **BARE**.
     * **BLACK SPLITTER:** 2 x **FEMALE**, 1 x **BARE**.
  * Wire connected to **STEAM** button pole **4** connects to BREW button pole **5**, original disconnected **BREW** pole **5** wire should be left disconnected but properly secured.
  * Connect the RED **"Y"** splitter to **BREW** switch pole **6** and plug the original connected wire to the **MALE** spade end, the **BARE** splitter end going to the optocoupler **L** terminal. 
  * Tap into the **PSU** splitter used for the **N** connection using the **BLACK** splitter and connect the bare end to the optocoupler **N**

2. Installing the RobotDYN dimmer module.

![dimmer_guide](https://user-images.githubusercontent.com/42692077/162638303-dd1333b0-212f-41b5-be64-de8543dc1153.png)
  
![dimmer-install-info](https://user-images.githubusercontent.com/42692077/147135452-fb3931c5-ac48-4a61-91ff-1b4275aaccb2.png)
</div>

>**The image above is provided as a reference to understand how the connection through the dimmer is made, please check whether your dimmer high voltage ports placement differs from the above image before connecting the dimmer, it's very important to feed the IN wires properly.**

3. Installing the pressure transducer.
  The pressure sensor will be tapping into the hose connecting the **pump outlet** and the **boiler inlet**, the way people connect it is up to individuals.

***It's advisable after making the connections and just before connecting the transducer itself turn on the machine and while cold engage the pump to fill the transducer hose with water as well, leaving a lot of air in the system might play funny with the readings.***
  
  ![New Project (11) (1)](https://user-images.githubusercontent.com/42692077/146647799-f4887edb-95ec-4a33-8561-4e4afda6256e.png)
  
  ![New Project (12)](https://user-images.githubusercontent.com/42692077/146647809-970c0ccd-47c6-430c-9c71-9b651bab4bf4.png)

## Recommendations
### Soldering 
* Take your time soldering perfectly
* Use the "Western Union" splice to link wires (GND and 5v)
* Tin your wires to make them easier to solder.

<img src="https://user-images.githubusercontent.com/53577819/154989813-02169bfd-99c6-4695-a255-67f93cf57097.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154989830-7402a644-e4f8-489b-82db-735552c757df.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154989847-822f76da-f33a-455e-95d1-ff173c4f1b8e.jpg" width="600" height="450">

### Resources
* GitHub: https://github.com/Zer0-bit/gaggiuino
* Discord: https://discord.com/channels/890339612441063494/890339612441063497
* Soldering 1: https://youtu.be/Fb7ONG08BUk
* Soldering 2: https://youtu.be/Fp37DPZVdRI
* Crimping : https://youtu.be/nvPESov0HbY
* Nextion : https://nextion.tech/faq-items/using-nextion-microsd/
