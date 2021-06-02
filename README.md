# gaggiuino

## WIP ##
 >UPDATE: I have taken photos and started editing them preparing the whole step by step guide, soon ( i'm thinking next week ) it should be all ready and published here.
 
### Better documenting coming soon, just delayed a bit by some job/life things i need to sort out now ###

Gaggiuino started as an idea to improve an already cappable coffee machine while keeping the machine looks and buttons functionality as close as possible to the original, an important part is that no internal cables/connectors were modified, all the connections were made by creating splitters using the purchased spade connectors.

>*First and foremost please do not underestimate the danger of electricity or overestimate your ability to work around it, only start working on your machine while it's  completely disconnected from the mains power socket, also by agreeing to follow the bellow guide i cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or gold fish and it will be entirely your fault!*

I will assume you have  the necessary components and installed the related software, for the sake of not gettign stuck at some point just double check again the list bellow.

<< PROJECT REQUIREMENTS >>

Software needed:
1. [Arduino IDE](https://www.arduino.cc/en/software)
    >*Needed to upload the code ".ino" to the arduino ROM*
2. [Nextion Editor](https://nextion.tech/nextion-editor/)
    >*Only necessary if planning on editing the ".HMI" file to ammend the LCD functionality*
3. [CH340 USB Driver](https://electropeak.com/learn/how-to-install-ch340-driver/)
    >*USB driver so your system recognizes the Arduino clone board, let's say i have found this the hard way as apparetly the majority of cloned arduinos use a cheaper USB controller comparing to "genuino"*

Hardware requirements:
1. [Arduino Nano](https://bit.ly/3eXSfXZ) - Be sure to buy the AT328 version, also if you're buying the mini usb version buy the cable from them, you'll save yourself a lot of problems.
2. [Nextion touch screen (i used a 2.4 inch one)](https://bit.ly/33dO8RV)
3. [MAX7765 thermocouple](https://bit.ly/3ejTUIj)
4. [C-M4 screw K-Type thermocouple sensor](https://bit.ly/3nP1WMm)
5. [40DA SSR Relay](https://bit.ly/33g1Pjr)
6. [Thermo-resistant cables AWG 15 ( 1m black/red ) and AWG 30 ( 1m black/red/yellow )](https://bit.ly/3tjSQbI)
7. [Spade connectors M/F 6.3mm](https://bit.ly/2Sjrkhu)
8. 5v power supply - for powering the arduino board once it's inside the machine.
>*I used an old mobile charger i had lying around which conveniently had a USB port for plugging the cable that came with the  board.*

Optional:
- [Heat shrink](https://bit.ly/2PQdnqt)
- [U-shaped Parallel Terminal](https://bit.ly/3h8phaF)  - These are in case you're coosing not to solder
- [Soldering iron](https://bit.ly/3tijlOI) - the one i bought for my soldering needs,  seems to do a great job
- [Nextion LCD case](https://bit.ly/3b2nxf7)
- [Magnet sheet](https://bit.ly/3xKTMJn)

![GaggiaPID1](https://github.com/Zer0-bit/gaggiuino/blob/main/images/PXL_20210526_071342331.jpg)

First you need to understand what goes where, the schematics isn't really rocket science but for someone who's never disasembled or has no experience working with electrical circuits it might get very confusing real fast so i will try to describe it as simple as possible, sometimes adding some photos as well.
First you need to check your setup works as expected outside the machine so you don't ahve it all installed and realise just afterwards it's not reading any temperature because of a faulty thermocouple or any other element, or your relay doesn't go between the ON/OFF modes.


1. The first step will be connecting the max6675 module to the arduino board using the pins defined in the code, you can find them defined at the top of the .ino file ( pins 4,5,6 )
2. Connect the relay, for now only connect the circuit controlling ports to check whether the relay LED indicates the power states.
>*Relay 3 to Arduino GND, Relay 4 to Arduino D8, the ports 1 and 2 on the relay are the high voltage circuit breaker*
4. Plug the arduino board using the mini usb cable that came with it and upload the code to the arduino board. 
>*Note: uploading won't work with the LCD connected*
5. Upload the .HMI file on a microSD card
>*Note: card needs to be FAT32 formatted*
6. Insert the card in the LCD card reader slot and connect the LCD to the arduino board.
>*Nextion TX goes to Arduino RX, and RX to TX, VCC goes to Arduino 5V and GND to GND*
7. After upload finished get the card out and power cycle the LCD.
8. You should see temp readings on your screen if everything went according to the plan.
>*Don't forget to test the thermocouple/relay combo operation, apply some heat to the thermocouple end and see whether the relay led operates in HIGH/LOW modes*

>Note: don't do any perament connections while testing, if you can i recommend buying a small breadboard which will allow to share ertain ports between the relay, thermocouple and the lcd display, like for ex the 5v and and the GND pins will be shared between all these devices.


### At this point if all the above works as expected you're ready to install it all inside the machine, for this we'll need to prepare some splitters that we'll use to connect to the Gaggia internals without introducing any permanent modifications so in the event of a desire to revert to stock it's a few disconnects away! ###

>*Okay so first things first, tripple check your machine is disconnected from any power sources, even better just pull the power cable out of it !*

###Now i won't be explaining every single detail as to how to disasemble the machine or how to use a pair of pliers, i'm expecting the people doing such sort of modifications will have some basic knowledge.###

##So here goes:##
1. Take off the top cover by unscrewing the 2 top screws, you should be able to see smth similar to the bellow image minus the SSR relay:
<img src="https://db3pap006files.storage.live.com/y4m4pob4r1pDtjBPqIyA-dqHOH_eZDJaf6W2dYdHlIh8G8OWusXig9WUKOA-iBCk2QRN-lL3ajrWDDUBASx_frpWqz_2z1dxeAnksAKKysKqL-eXE9PVRYeA2SdmS_DSkAA3TJ5ZVe3ybpkLYV0-PDKLjEhxNZluA_UX8ektw8kGW4PXKQeQU-UUJtjuaDSYKsG?width=3496&height=4656&cropmode=none" width="769" height="1024" />

2. Prepare 2 splitters like in the bellow image using the AWG15 cable, be sure one splitter to be black(negative) and one red(positive)
<img src="https://db3pap006files.storage.live.com/y4mGdTrz4hXNuu3rvDk5qro2WGn5xqy8ZVGwhJSXFSDqmJErI8dYufS1H-l_PnIIa0HffKXPuPkvbRjNHt_2OogxaW8UohuFKatz3BfjjK8NEGmynX2unmeZ6opV3_gd-u0f3cCAlgh9nF5spGDt12McFxpxzsatrSK2YuRgrFTfFnxMvMmiXss0XSLrZGx5xIa?width=769&height=1024&cropmode=none" width="769" height="1024" />

3. Be sure to mark your top left power connector so you don't mix them up ( even though it's not that hard to understand which one is which)
<img src="https://db3pap006files.storage.live.com/y4mM3HVfjbCIdcPtx16eN0pEgGC4Z0ih04agqfBwI-NLtpUWvmlBFI23fT2LPVagGHXYsgRiIVq8oSjJckuZscCcdTKqq7GNxCK5GxRqsp2pCZinopGHCdqJtnZqMCFBlSq-yOT-vNtsATxE734AN05DO47i1as2NSW4E-87r78gV7kpP_sy9tHiBqey26s9xP0?width=769&height=1024&cropmode=none" width="769" height="1024" />

4. Disconnect all 3 of them as you'll use the midlle and bottom ones for power sharing.
<img src="https://db3pap006files.storage.live.com/y4m9fZvq6wrObA7QFSkvQpfFyOinQbfs-u_E8DLZqp784cPLcUIlZKoZeqbLWn3JPAElseiwa0G1KuN-yXJfOx_4N-3k-IU0YprmK-YHBYEG-33CaUXTEWVPNVwjy6y7kMyaDvbEzH445Su6hSKNqZqAPXmwJQlIH8lAQmXaTduAk3WIsUrZSw2J0lPhRRVEqzX?width=769&height=1024&cropmode=none" width="769" height="1024" />

5. The hardest part will be now in my opinnion as you'll have to unscrew the bottom boiler stock thermostat and screw back in the new thermocoule, be sure to apply some thermal paste on the thermocouple threads ( just a teeny tiny bit ):

<img src="https://db3pap006files.storage.live.com/y4mTxf55lGmZZXwW9c5ZtPSKiib-e1-GUz55rqAWBa6jWDovmKHDfve5ldN1OU4__vWXlqpDieD3WAFSyndaGA0KV76zeo49UjVOuWKTUTmeVBseXmgPze6pTjTBHTCQmzJ3gAs_Wxl8DmdQkU5rSZFLCZkuGiaQc4hZjnOBMoyoEATlMpWKGmCeiF98_3udqTh?width=3496&height=4656&cropmode=none" width="769" height="1024" />

6. Prepare 2 cables you'll use to connect the cables disconnected form the thermostat to the port 1 adn 2 of the SSR relay, use the red cable for that they shouldn't be too long, about 10cm will suffice, one end should be crimpled wiht a male spade connector and the loose end screwed to the relay.
<img src="https://db3pap006files.storage.live.com/y4m3ZmhHEobr2VPen1wScEO8AMLiaOVfWjsnW8Aw4645PaRXerrLpV8iZmZjzU88KDPmKEfPXbUBqYhn6ejVOfXIB7hgucmljyOLL54tqWKUahFoqnYEf2rBh0TRXjFEtzAvptdT3W8nSTzc96sfvD8hcK-pQEeEaYaLaq4gpNaKyXdWQtETpPtrzpMd1mLzuyb?width=2584&height=2140&cropmode=none" width="769" height="1024" /> 

So you end up having the m connected like this:
<img src="https://db3pap006files.storage.live.com/y4m5HfjPvkFSOtSm8gkTJJ4saqEw8dcB1no2eoaHcDZjYD9OV95tBnrG8tqtRuOcor7aFZrIRw7167k4QGMUneOPATitBFctkgdklxWOohMyBir3zLdm9fkAnTQW8TquTZouRh9rzKSC0t5bkerK1g8AzlFYTfuZoDPQ3juFiOJ19JKiL6VpODn40Z1q8JwVpGy?width=2625&height=2831&cropmode=none" width="769" height="1024" />
