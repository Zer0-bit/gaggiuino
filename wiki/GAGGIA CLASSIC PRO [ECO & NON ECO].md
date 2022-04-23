# 1.0 PROJECT REQUIREMENTS

## 1.1 Software requirements
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

## 1.2 Hardware Requirements
***The code has been designed to be plugable, meaning there is a minimal hardware configuration one can start with if certain features are not something of interest, it's all appropriately split under "BASE" or "EXTENDED" functionality (see bellow).***


### **Base Parts** 
* [Arduino Nano AT328](https://www.aliexpress.com/item/4000903444456.html?spm=a2g0s.8937460.0.0.30012e0enAx90o) - (Welded with Cable)
> * [Arduino Nano Expansion board](https://www.aliexpress.com/item/1005003677441375.html?spm=a2g0s.8937460.0.0.30012e0enAx90o)
* [2.4" Nextion LCD](https://bit.ly/3CAUzPj) - NX3224F024
* [MAX6675 thermocouple](https://bit.ly/3ejTUIj) - this is just for the board
* [C-M4 screw K-Type thermocouple sensor](https://bit.ly/3nP1WMm) - this is what will be used with the MAX6675 board and replacement of the one on the machine
* [40DA SSR Relay](https://bit.ly/33g1Pjr)

* [Thermo-resistant cables](https://bit.ly/3tjSQbI) (all of below):
> * AWG15, 1m, black AND red
> * AWG20, 1m, black AND red
> * AWG26, 5m, black, red, yellow, blue, green, purple AND white
* Spade connectors - [Male AND Female 6.3mm](https://bit.ly/2Sjrkhu)
* [Heat-shrink](https://bit.ly/2PQdnqt)

Check through the discord for any additional cable management parts you may like

Power Supply (**CHOOSE ONE** of the methods below) - could use an old mobile charger if it is above 1.5A
* [5v 2A power supply](https://www.aliexpress.com/item/33012749903.html?spm=a2g0o.9042311.0.0.e1ec4c4deG81E4) - **_Preferred method_**
* [5v AC Adaptor](https://bit.ly/3jWSrKa)
* If you get this [12v Power Supply](https://www.aliexpress.com/item/33012749903.html) you need the following: 
> * [12v to 5v stepdown USB output](https://www.aliexpress.com/item/32525175131.html) 

***

### **Extended Parts**
* [RobotDYN dimmer module - Dimmer 4A-400V](https://bit.ly/3xhTwQy)
* [Pressure sensor - 0-1.2Mpa](https://www.aliexpress.com/item/4000756631924.html)
> * Hose - [4mm x 9mm](https://www.aliexpress.com/item/1005003157483270.html?spm=a2g0o.9042311.0.0.e1ec4c4deG81E)
> * Sensor fitting options - [6mm x 1/4](https://www.aliexpress.com/item/33059380672.html)
> * T fitting - [5mm](https://a.aliexpress.com/_vcLoZM) - if hose with 4mm ID get 5mm brass T fitting
> * Clamps - [100pcs and plier](https://www.aliexpress.com/item/1005003341137707.html?spm=a2g0o.9042311.0.0.e1ec4c4deG81E)
> * PTFE aka Teflon aka plumbers tape for sealing fittings (cheaper to get from a local hardware store)
* [HX711 AND 500g Load Cells](https://www.aliexpress.com/item/32670225988.html?spm=a2g0o.9042311.0.0.e1ec4c4deG81E4) - _**TWO OF EACH**_

## 1.3 Tools
List of **suggested** tools you're going to need for the complete install.
* Heat gun
* Set of screwdrivers
* Multi-meter
* [Soldering iron](https://bit.ly/3tijlOI)
* [Wire strippers](https://www.aliexpress.com/item/1005003438703016.html?spm=a2g0o.productlist.0.0.b23d58d9dowJnZ&algo_pvid=bb9dd216-d8af-4a90-b854-e6f4ef3a7f14&algo_exp_id=bb9dd216-d8af-4a90-b854-e6f4ef3a7f14-7&pdp_ext_f=%7B%22sku_id%22%3A%2212000025793684332%22%7D&pdp_pi=-1%3B8.31%3B-1%3B-1%40salePrice%3BGBP%3Bsearch-mainSearch) (these come with additional features like wire cutting and crimping)
* Crimpers (if your stripper doesn't have crimping)
* Pliers (to help tighten the pressure sensor to the fitting)
* Black insulation tape
* Max 32GB microSD card
* Thermal paste of your choice 


## 1.4 3D Housing variants

All variants have been designed by the redditor [/u/LikeableBump1](https://www.reddit.com/r/gaggiaclassic/comments/phzyis/gaggia_with_arduino_pid_in_3d_printed_case/):
 * [Gaggiuino v1](https://www.thingiverse.com/thing:4949471)
 * [Gaggiuino v2](https://www.thingiverse.com/thing:5236286)

**Optional additional parts to use with the V1 housing type:**
* [DB15 D-SUB connectors, male and female](https://www.aliexpress.com/item/1005002650762015.html?spm=a2g0o.productlist.0.0.6f0a1b50CsSmjP&algo_pvid=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b&aem_p4p_detail=202109282059464734316742000220004860154&algo_exp_id=8cc51e0e-429a-4d9b-a99b-d6d059ebf29b-14&pdp_ext_f=%7B%22sku_id%22%3A%2212000021558768022%22%7D)
    >*These are used as a disconnectable pass-through to get the data and power cables through the back of the machine case. The 3d files include custom housings for these connectors. There will need to be 2 or 3 of the vertical "slats" cut out of the cooling vents on the machine case back to make these fit. Make sure you purchase both male and female DB15 connectors.*

*** 

# 2.0.0 TEST INSTALL
We are not installing inside the machine yet and we are doing minimal soldering at this point. We just want to test as much as we can to make sure we've not got any duds from a base functionality point of view.

We need to understand what goes where. The schematics in (**3.0.1 Schematics & Diagrams**) aren't really rocket science but for someone who's never disassembled or has no experience working with electrical circuits it might get confusing really fast. 

Note 1 - No permanent connections are needed during testing so no soldering needed for now.
Note 2 - The 5v/GND Arduino board pins will be shared between all the connected devices.

## 2.0.1 Arduino Config 
Place Arduino into the expansion board the correct way round. Power it through USB adapter for now during the testing phase.

Image of Arduino with pins soldered:

<img src="https://user-images.githubusercontent.com/53577819/154988207-3d54e924-3ab0-4e47-a4e9-188f7e839610.jpg" width="400" height="250">

Image of expansion with pins (solder from the bottom):

<img src="https://user-images.githubusercontent.com/53577819/154988218-26160147-e821-47e7-8152-d8ec0a8f0d43.png" width="400" height="250">

Make sure the terminals are correct way round and place the Arduino into the expansion board.

If you have to solder your pins then take the time now to get your soldering perfect. It'll save time in the long run. REFER TO APPENDIX *3.0.3 Recommendations* FOR SOLDERING GUIDES!

<img src="https://user-images.githubusercontent.com/53577819/154988330-430df58d-773c-4084-b7b3-921ca43909f2.jpg" width="400" height="250">

_**Attention! You are not soldering the Arduino to the expansion board itself. You are only soldering the pins to the Arduino and pins to the expansion board. The Arduino then just sits/connects into the expansion board pins.**_

## 2.0.2 MAX6675 Config
***
For the below section you are using **26AWG** cable to connect to the Arduino board.
 
**Refer to the appendix 3.2 for component wiring.**
***

You should have bought and received a MAX6675 board which comes with a test thermocouple. We're swapping this for the "C-M4 screw K-Type thermocouple sensor" as this is what will be replacing the original that's in the machine.
 
<img src="https://user-images.githubusercontent.com/53577819/154988392-619a5bc2-99cc-499d-8ad8-42792b094fd2.jpg" width="400" height="250">

<img src="https://user-images.githubusercontent.com/53577819/154988423-f741a369-ec60-4266-9544-0effedb57292.jpg" width="400" height="250">

## 2.0.3 Nextion Config
***
For the below section you are using **26AWG** cable to connect to the Arduino board. 
 
**Refer to the appendix 3.2 for component wiring.**
***

The correct screen size is **2.4**". If yours does not have markings for wiring then please see below image

<img src="https://user-images.githubusercontent.com/53577819/154988491-28c1e861-46bc-433b-a5f8-8e9eeb8b51ed.png" width="400" height="250">

<img src="https://user-images.githubusercontent.com/53577819/154988514-13a1d7af-f704-49b6-975b-aeee1318cf36.jpg" width="400" height="250">

## 2.0.4 Relay Config
***
For the below section you are using **26AWG** cable to connect to the Arduino board. 
 
**Refer to the appendix 3.2 for component wiring.**
***

This relay is what manages the temp by cutting the voltage when the thermocouple is at a set temp. 

<img src="https://user-images.githubusercontent.com/53577819/154988554-5be0bb0a-dcf2-4bf7-a70f-4c356d00eea6.jpg" width="600" height="450">

## 2.0.5 Software Install
### Arduino Software Upload
Plug the Arduino board into PC/Mac using the mini-USB cable that came with it and upload the code to the Arduino board.
Note: Uploading won't work with the LCD connected

1. Download/Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Download/Install [CH340 USB Driver](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
3. Download the latest version of Gaggiuino from the GitHub main repository.  Copy the unzipped files to an easily accessible location
4. Add Libraries:
* Click Sketch\Include Libraries\Manage Libraries
* Type in the Library name and install most recent version
* Install 2 Libraries:
  * Easy Nextion Library
  * MAX6675 by Ada-fruit
5. Add External Libraries:
* Download Library Zip Files:
  * PSM
**Files must be unzipped** to the Arduino Library folder (default path is located in your local documents folder.  You should see the added libraries from step 4 here as well.)
6. Open Gaggiuino.ino (located in Gaggiuino folder from step 3). If prompted to create a sketch folder, press OK

**AT THIS POINT AFTER INSTALLING LIBRARIES RESTART THE IDE SOFTWARE**

7. Unplug the LCD from our Arduino Board mock setup
8. Plug the Arduino in via USB cable (must be a data transfer cable, not just a power cable)
9. Set Board, Processor, Port:  Navigate to “Tools” in the top menu: 
* Board: “Arduino Nano”
* Processor: “ATmega328P”
* * Use the “Old Bootloader” version, but results may vary depending on your hardware.
* * When plugged in and the “port” section should be active and something should be selected.  If not manually select the port that contains the Arduino.  Mine was on “Com4”. Try unplugging then plugging in again if it doesn't work.

<img src="https://user-images.githubusercontent.com/53577819/154988600-334dd985-f9c1-4aed-a55c-ab5c03ef51ab.png" width="600" height="450">
 
10. Press Upload (***Make sure LCD is UNPLUGGED before uploading otherwise upload will fail***)
11. If upload is Successful, unplug Arduino Board from computer

### Flashing the Nextion LCD Software Upload
_Uploading the LCD ROM code_
Method 1 - Download and unzip the *.tft file on a FAT32(MS-DOS (FAT32)) formatted microSD card and upload on the LCD panel using the onboard card reader.

Method 2 - Open the .HMI file using Nextion Editor and using the File menu upload it on a microSD card

1. Plug LCD back into Arduino Board Setup
2. Locate “nextion-discovery-lcd.tft” or “nextion-basic-lcd.tft” (depends on what you purchased)
3. Copy the file to a FAT32 formatted MicroSD card
* If on Mac for some reason it adds a second file beginning with a "." use terminal to change directory to the microSD and use "rm ._nextion-*YOUR-VERSION*-lcd.tft" to remove that file.
4. Place MicroSD card inside the Nextion display
5. Power on the Arduino Board Setup and installation should occur automatically
6. Once “successed” reboot the Arduino

## 2.0.6 Test

You should now see the Gaggiuino project on the LCD. If your connections are all right (and well secured), you should see a temperature reading from the thermocouple (temp reading will contain the default offset of 7 which means the initial temp will be - room temp -7)

Try applying heat with your hand and you should see the temperature respond (if not, I recommend confirming all connections are correct and adequately tightened.  Mine did not respond at first and turned out one of the connections to the Arduino board was not tight enough)

If everything looks good, move on to Installing into the Gaggia Classic Pro

_**If all the above works as expected you're ready to install it inside the machine.**_

# 2.1.0 INSTALL
***

_**!! WARNING !!**_

_**Do not underestimate the danger of electricity or overestimate your ability to work around it.**_

_**Only start working on your machine while it's completely disconnected from the mains power socket, also by agreeing to follow the below guide I cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or goldfish and it will be entirely your fault!**_

***

Undo everything as we will now install into the machine. 

All components except the Arduino and LCD will be internal to the machine. Remember this when wiring the LCD. 
For each of the components we want to start guesstimating on cable length. You can do this by placing components where you want to place them. Close together components can share similar wiring i.e. 5v and GND.

Please use the tables in **3.0.2 Appendix - Component Wiring** for details on pin connections to the Arduino. 

## Base Functionality
### 2.1.1 Power Delivery
Take off the top cover of your machine by unscrewing the 2 top screws. Be sure to mark your top left power connector so you don't mix them up (even though it's not that hard to understand which one is which).

Prepare splitter cables with the below specs:
1. Black splitter, 15AWG, 10cm, female end, male end, male end
2. Red splitter, 15AWG, 10cm, female end, male end, male end

Example of a red and black splitter:

<img src="https://user-images.githubusercontent.com/53577819/154988645-5f1da175-7d09-437a-ac2c-f64e16f3a564.jpg" width="600" height="450">

The female end will go in Gaggia's front panel. One male splitter end will go into the connection that came out of the switch position the other male will be for some more wires further down.


<img src="https://user-images.githubusercontent.com/53577819/154988784-7ddbc106-4744-4d9f-8879-b9a107a9fe7c.jpg" width="600" height="450">


**Before trying to copy piggyback locations from below it’s recommended to check what the schematics say (3.0.1 Schematics & Diagrams).** 


By way of context, if looking inside the machine from the back, the left of the power switch is the brew switch. 
On the power switch - wire in the middle left as LIVE and middle right as GND 


<img src="https://user-images.githubusercontent.com/53577819/154979422-f78d41c4-87f4-4474-ab68-a3466d5e9e5e.jpeg" width="600" height="450">

The above translates to the following (**please be aware the image below has top connectors removed for clarity)**:

<img src="https://user-images.githubusercontent.com/53577819/154979529-eae513b2-00e3-40c9-a581-47be4da68edc.jpg" width="600" height="450">



If above standard schematics do not work it can be that your wiring is different. It is recommended to use a multimeter to test. 

<img src="https://user-images.githubusercontent.com/53577819/154982423-82b4bdb4-ca78-4077-90c0-77b349c8c3d2.JPG" width="600" height="450">



* Set it to 600V~ (AC) as shown above and test the connectors on the switches
* Just wiggle the connector on the switch out a touch to expose a tiny bit of metal where you put your lead on
* Use schematic to find a GND point
* Your live piggyback will show no voltage until the machine is switched on

As stated you want to find the connector that only shows voltage after flipping the machine machine on. 

If your LCD is on even though you've not flipped the machine switch on, this means you've piggybacked into the mains live which is constantly got voltage - this is not correct!



Please see some examples of known differences but really you should be testing with a multimeter.

**KNOWN - ECO - EU/UK - piggyback locations non-schematic:**

<img src="https://user-images.githubusercontent.com/53577819/154988901-b79270da-5bb3-4507-9ef2-57bf2e791d77.JPG" width="600" height="450">


Prepare 2 more cables with the below spec:
1. Black, 18/20AWG, about 15cm, female end, exposed end
2. Red, 18/20AWG, about 15cm, female end, exposed end

Connect the female end to the male end of the splitter cable from above step (match the colours)

If using the 5v PS - the 2 AC IN ports will be where the exposed end of your red or black piggyback cable go, it doesn't matter which way round (yes, you need to fiddle about getting wires in and solder correctly - recommended to trim, twist, tin and apply heat shrink).

<img src="https://user-images.githubusercontent.com/53577819/154989029-59ab20c6-fdda-4510-a9e0-50579b8e4a54.jpg" width="600" height="450">

Prepare more cables with the below spec:
1. Black, 26AWG, about 25cm, exposed ends
2. Red, 26AWG, about 25cm, exposed ends

In relation to the image above - the black will be connected to the GND and the red to the VCC the other ends are going to the Arduino. Please use the tables in **3.0.2 Appendix - Component Wiring** for details on wiring VCC and GND connections to the Arduino. 

_**Make sure this component is well insulated and enclosed. You do not want to touch it or let it make contact with anything whilst the machine is on! **_

**_If you have the ECO version (the machine turns off automatically after 20mins) bridge your brew switch with a 15AWG cable with two male ends, as per below:_**

<img src="https://user-images.githubusercontent.com/53577819/154989122-6237e1af-62d1-4289-901c-47c69ea3b1b9.jpg" width="600" height="350">

### 2.1.2 Thermocouple and MAX6675 Config 

Prepare the following cable to the below spec:

1. Black, 15AWG, 5cm, two male ends

[Detach the boiler](https://www.youtube.com/watch?v=0ipvBdWaVzQ) (only watch as far as the 5min mark) to gain enough access to remove the thermocouple and replace it with the m4 bolted thermocouple sensor. 

You’ll obviously need to remove the two connectors from the original thermostat first - which is located at the bottom of the boiler on the side closest to the power on switch.  

The small black cable you prepared with the male ends is used to bridge the two wires that you just disconnected from the brew thermostat. 


<img src="https://user-images.githubusercontent.com/53577819/154983715-46b1332f-f88a-4ef3-a625-b7f2788caf97.jpeg" width="600" height="450">


Now unscrew the original thermostat and replace with the m4 brass thermocouple. Be sure to apply some thermal paste (just a teeny tiny bit) on the _**threads only**_. It is important to barely tighten the m4 bolt at **barely** fingertip strength. 

<img src="https://user-images.githubusercontent.com/53577819/154989188-6bc38c2c-60df-47f6-98d9-b121f28438d6.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154989228-2f9a3669-551b-4f45-a82a-22b4fe41fa89.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154984016-714b6532-e31d-42d3-b2ee-cf064487db66.jpeg" width="600" height="450">


Now re-attach the boiler.
Check the appendix section **## 3.0.2 Component Wiring  2. MAX6675 WIRING**

_**Make sure this component is well insulated and enclosed. You do not want to touch it or let it make contact with anything whilst the machine is on!**_

### 2.1.3 Relay Config 

Prepare cables with below spec:

1. Red, 15AWG, 10cm, one male end, one exposed end
2. Red, 15AWG, 10cm, one male end, one exposed end

The 2 are used to connect the steam thermostat connections to port 1 and 2 of the SSR relay, so:

<img src="https://user-images.githubusercontent.com/53577819/154989308-14e9ea82-8a60-4f3d-bd9f-05dda6fa15c8.JPG" width="600" height="450">


Optional: might be a good idea to either tape up the exposed steam thermostat or remove it and tape up the exposed location.


<img src="https://user-images.githubusercontent.com/53577819/154989373-57093be0-ac48-4ac6-ad0b-d88adc2415d9.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154989357-ec8c4853-0bb9-4eec-8629-dfc76e72e725.jpg" width="600" height="450">

<img src="https://user-images.githubusercontent.com/53577819/154989336-76f91fb5-2fdd-4c53-b968-e860af3f8505.jpg" width="600" height="450">

Please use the tables in **3.0.2 Appendix - Component Wiring** for details on 3 & 4 connections to the Arduino. 

### 2.1.4 Steam Config

_**Very important to not turn on the machine until we check the steam switch wire positions**_

**NON-ECO VERSION**

Image of the steam switch schematic:

<img src="https://user-images.githubusercontent.com/53577819/154989457-f5ac859e-5aa5-4554-be46-fba6ad4fa1f0.png" width="600" height="450">

1. Move steam switch wire 4 to steam switch pole 1.
2. Unplug and secure steam switch wire 5.
3. Connect steam switch poles 4 and 5 to the Arduino nano as shown in **3.0.2 Component Wiring - 5. STEAM HANDLING WIRING**, using 26AWG wires.

**ECO VERSION**

Prepare a black splitter with below spec:

1. Black splitter, 15AWG, 5cm, two male ends and one female end


1. Disconnect the two top poles (1 & 4)
2. Use the splitter to bridge the connections you just removed and plug the female into pole 1
3. The connector with two white wires is not on the side of the orange wire (which is at the bottom) then make it so (to match schematics) - i.e move pole 5 connector into pole 2's location (should be two white wires going into the connector)
4. Leave the single white wire disconnected which was in pole 2's location
3. Connect steam switch poles 4 and 5 to the Arduino nano as shown in **3.0.2 Component Wiring - 5. STEAM HANDLING WIRING**, using 26AWG wires.

Image for reference below:

<img src="https://user-images.githubusercontent.com/53577819/155016949-a070e451-2bd8-45e8-9acb-669b7cc28bab.jpg" width="600" height="450">


### 2.1.5 Continuity Brew Detection

Prepare 2 cables with the below spec:
1. Green (not red or black), 26AWG, length from brew to Arduino, one exposed end, one female end 
2. Black, 26AWG, length from brew to Arduino, one exposed end, one female end

As shown below plug your cables in to the circled connections on the brew switch  

<img src="https://user-images.githubusercontent.com/53577819/154985120-fe503708-50c1-4376-9459-d72312fbbadc.jpeg" width="600" height="450">

Please use the tables in **3.0.2 Appendix - Component Wiring** for details on GND and OUT connections to the Arduino.

## Extended Functionality

### 2.1.6 RobotDYN Dimmer Config
Prepare 3 cables with the below spec:
1. Red, 18/20AWG, 15cm, one exposed end, one female end 
2. Red, 18/20AWG, 15cm, one exposed end, one male end 
3. Black splitter, 18/20AWG, 15cm, one exposed end, one male end, one female end

Please check whether your dimmer ports placement in the case it differs from the images before connecting the dimmer, it's very important to feed the IN and OUT wires correctly!

Example of dimmer ports:

<img src="https://user-images.githubusercontent.com/53577819/154989499-7e95ca82-f8eb-4f63-b57b-3ca1e7e14eeb.png" width="600" height="450">

Remove the connections detailed in the image below from the pump (mark the connector either L or R for where it was located either left or right in relation to the pump):-

<img src="https://user-images.githubusercontent.com/53577819/154989519-4e5ba059-22a4-4397-8dfb-5b7dff411bbf.jpg" width="600" height="450">

With the cables you prepared:
* Cable 1: From the dimmer positive line Out - single red wire with the female end going into one of (choose the left connection according to the image above) the pump connections itself. 

* Cable 2: From the dimmer positive line In - single red wire with the male end going into the connector that was removed from the pump above (in this case it would be the left connector).

* Cable 3: From the dimmer neutral line In - with the black splitter with the male into the connector taken out of the pump (right connector) and female into the (right) remaining connection on the pump itself.

_**Triple check your dimmer board on what is marked as IN and OUT. 100% need to make sure they are connected properly.**_

<img src="https://user-images.githubusercontent.com/53577819/155005284-b22a6652-87b0-42db-9d35-5d2bc8b9247e.jpg" width="600" height="450">


<img src="https://user-images.githubusercontent.com/53577819/155005572-1d7c2ef1-ed1e-4c6e-9004-2fc22c73d83b.jpg" width="600" height="450">



_**Again!!! Make sure this component is well insulated and enclosed. You do not want to touch it or let it make contact with anything whilst the machine is on!**_

Please use the tables in **3.0.2 Appendix - Component Wiring** for details on VCC, GND, Z-C and PSM connections to the Arduino. 

### 2.1.7 Pressure Transducer Config
Installing the pressure transducer. The pressure sensor will be tapping into the orange braided hose connecting the pump outlet and the boiler inlet. I would generally advise to take out the original hose and use the one ordered together with the pressure sensor, cut a similarly sized one out of the hose purchased and use the rest of the left length as additional transducer buffer.

It's advisable after making the connections and just before connecting the transducer itself turn on the machine and while cold engage the pump to fill the transducer hose with water as well, leaving a lot of air in the system might play funny with the readings (please be careful to do this outside the machine as water spill out...).

(INSERT IMAGES - of pressure transducer set up)

Please use the tables in **3.0.2 Appendix - Component Wiring** for details on RED, BLACK andYELLOW wire connections to the Arduino. 

### 2.1.8 Load Cells Config
_**TO DO**_

### 2.1.9 Finish
***
If you haven't already, you're ready to connect everything to the Arduino. Use 3.0.2 Appendix for **Component Wiring** tables. 

One piece of advice would be to solder all cables to their respective boards as during the machine operation there is quite a bit of vibration which can introduce noise/frequent. This can lead to unexplained behaviours.

Any components sitting next to each other can have their equivalent pins linked with a cable.

You will be expected to solder similar pins i.e. GND and 5v together in order to fit into and screw down on the expansion board terminals.
***

On first start up record and send to #first-start channel of discord. This helps with diagnosing issues.

Remember to change and save your correct region settings.

Configure you PP and PI settings and get prepped for a shot if all is well. Record and upload to #first-shot on the discord. 

All going well, feel like an absolute coffee titan each and every time you pull a shot.

# 3.0 Appendix
## 3.0.1 Schematics & Diagrams
### Schematics
[GAGGIA Classic Pro](https://github.com/Zer0-bit/gaggiuino/blob/release-0.2.2/schematics/gcp-schematics.png)

### Diagrams
[GAGGIA Classic Pro](https://github.com/Zer0-bit/gaggiuino/blob/release-0.2.2/schematics/GCP-CONNECTIONS-DIAGRAM.png)

## 3.0.2 Component Wiring 
You can find the defined pins at the top of the .ino file.

A suggestion on wiring; components that are inside same enclosure, the same connection can be linked i.e. solder a cable to the 5v pin then take the other end and solder to the 5V of the other component, from there take one cable to the Arduino. An alternative way is to  solder similar connections and apply heat-shrink before the exit to the machine then take one wire through the machine to the Arduino.

1. POWER DELIVERY RECOMMENDATION
Method 1 - If choosing to power the system using the AC adapter then the Arduino board and all the connected components will receive power by the means of the regulated 5v the AC adapter delivers through the USB port.

| PS | Arduino | 
| --- | --- |
| VCC OUT | 5v | 
| GND OUT | GND |

Method 2 - If powering using the [ 12v ] power supply module + [ 9v ] step-down convertor follow the bellow scheme:

| PS | Arduino | 
| --- | --- |
| 9v | VIN | 
| GND | GND |

All the other boards below will get their power from the Arduino 5v / GND pins and it's extremely important they are powered using those outputs.

2. MAX6675 WIRING

| MAX6675 | Arduino |
| ------- | ------- |
| VCC | 5v |
| GND | GND |
| SCK | D6 |
| SO  | D4 |
| CS  | D5 | 

3. RELAY WIRING 

| RELAY | ARDUINO |
| --- | --- |
| 4 | GND |
| 3 | D8 |

4. NEXTION WIRING

| NEXTION | ARDUINO |
| --- | --- |
|TX | RX |
|RX | TX |
|VCC | 5v |
|GND | GND |

5. STEAM HANDLING WIRING

4 & 5 are the switch points from 2.1.4 Steam Config

| GCP SWITCH | ARDUINO |
| --- | --- |
| 4 | D7 | 
| 5 | GND |

6. CONTINUITY WIRING

| BREW SWITCH | ARDUINO |
| --- | --- |
|TOP | GND |
|BOTTOM | A0 |

7. ROBOTDYN DIMMER WIRING

| DIMMER | ARDUINO |
| --- | --- |
| VCC | 5v |
| GND | GND |
| Z-C | D2 |
| PSM | D9 |

8. TRANSDUCER WIRING

| TRANSDUCER | ARDUINO |
| --- | --- |
| RED | 5v |
| BLACK | GND |
| YELLOW | A1 |

9. LOAD CELLS

TO DO

***

## 3.0.3 Recommendations
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