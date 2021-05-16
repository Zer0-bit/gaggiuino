### WIP ####

### Better documenting coming soon, just delayed a bit by some job/life things i need to sort out now ###
# gaggiuino
Gaggiuino started as an idea to improve an already cappable coffee machine while keeping the machine looks and buttons functionality as close as possible to the original, an important part is that no internal cables/connectors were modified, all the connections were made by creating splitters using the purchased spade connectors.

First and foremost please do not underestimate the danger of electricity or overestimate your ability to work around it, only start working on your machine while it's  completely disconnected from the mains power socket, also by agreeing to follow the bellow guide i cannot be deemed responsible for any of the damage you induce to your house appliances, yourself, your cat, friend or gold fish and it will be entirely your fault! 

I will assume you have  the necessary components and installed the related software, for the sake of not gettign stuck at some point just double check again the list bellow.

<< PROJECT REQUIREMENTS >>

Software needed:
1. Arduino IDE
2. Nextion Editor

Hardware requirements:
1. Arduino Nano - https://bit.ly/3eXSfXZ
   Be sure to buy the AT328 version, also if you're buying the mini usb version buy the cable from them, you'll save yourself a lot of problems.
2. Nextion touch screen (i used a 2.4 inch one) - https://bit.ly/33dO8RV
3. MAX7765 thermocouple - https://bit.ly/3ejTUIj
4. C-M4 screw K-Type thermocouple sensor - https://bit.ly/3nP1WMm
5. 40DA SSR Relay - https://bit.ly/33g1Pjr
6. Thermo-resistant cables AWG 15 ( 1m black/red ) and AWG 30 ( 1m black/red/yellow ) - https://bit.ly/3tjSQbI
7. Spade connectors M/F 6.3mm - https://bit.ly/2Sjrkhu

Optional:
- Heat shrink - https://bit.ly/2PQdnqt
- U-shaped Parallel Terminal - https://bit.ly/3h8phaF  - These are in case you're coosing not to solder
- Soldering iron - https://bit.ly/3tijlOI  - the one i bought for my soldering needs,  seems to do a great job
- Nextion LCD case - https://bit.ly/3b2nxf7
- Magnet sheet - https://bit.ly/3xKTMJn

![GaggiaPID1](https://github.com/Zer0-bit/gaggiuino/blob/main/images/PXL_20210426_180310406.jpg)

First you need to understand what goes where, the schematics isn't really rocket science but for someone who's never disasembled or has no experience working with electrical circuits it might get very confusing real fast so i will try to describe it as simple as possible, sometimes adding some photos as well.
First you need to check your setup works as expected outside the machine so you don't ahve it all installed and realise just afterwards it's not reading any temperature because of a faulty thermocouple or any other element, or your relay doesn't go between the ON/OFF modes.


1. The first step will be connecting the max6675 module to the arduino board using the pins defined in the code, you can find them defined at the top of the .ino file ( pins 4,5,6 )
2. Connect the relay
4. Plug the arduino board using the mini usb cable that came with it and upload the code to the arduino board. ( Note uploading won't work with the LCD connected )
5. Connect the nextion LCD ( Nextion TX goes to Arduino RX, and RX to TX )
6. Upload the HMI file to the screen ROM ( using a microSD card )
7. Disocnnect/reconnect te mini usb cable
8. You should see temp readings on your screen if everything went according to the plan.
