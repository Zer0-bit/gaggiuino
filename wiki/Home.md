**_Total estimated install time will be based on understanding and experience._**

**_Documentation contributors:_**
* [mr.toor](https://www.youtube.com/watch?v=dQw4w9WgXcQ)
* [Samir Kouider](https://github.com/samirkouider)
# Features / Completeness 
**BASE:**
- [x] Integrated Housings - **credits** [![I'm on Reddit](https://img.shields.io/reddit/user-karma/combined/LikeableBump1?style=social)](https://www.reddit.com/user/LikeableBump1 "I'm on Reddit")
- [x] Customised UI/UX - **credits** [![I'm on Reddit](https://img.shields.io/reddit/user-karma/combined/different-wishbone81?style=social)](https://www.reddit.com/user/different-wishbone81 "I'm on Reddit")
- [x] Brew/Steam temp control
- [x] Shot timer
- [x] Graphing
- [x] Realtime values update/save

**EXTENDED:**
- [x] Pre-infusion
- [x] Pressure profiling
- [x] Manual pressure control
- [x] Descale program
- [ ] Integrated scales _>> WIP ~90%_
- [ ] Saving/Loading profiles
- [ ] Web interface
- [ ] OTA updates
- [ ] Advanced Flow control

# Mod in operation:
[![Video](https://img.youtube.com/vi/hpxB1Q6AFkY/maxresdefault.jpg)](https://www.youtube.com/watch?v=hpxB1Q6AFkY)

***USAGE:***

 * **BOILER**      - sets the desired temperature at the boiler level
 * **OFFSET**      - sets the offset value used to calculate the real water temperature
 * **HPWR**        - sets the relay start pulse length
 * **M.C.DIV**     - sets the main cycle divider(aka non brew heating beahaviour), used in conjunction with HPWR
 * **B.C.DIV**     - sets the brew cycle divider
 * **Brew(Auto)**  - all pressure settings are following the bellow:
   * PREINFUSION - enables pre-infusion
     * Time       - sets the length of the PI phase
     * Bar        - sets the max reachable pressure for the PI phase
     * Soak       - sets the length of the soaking(blooming) phase
   * P-PROFILING - enables AUTO pressure profiling mode
     * Start      - sets the desired starting point  of the PP phase, can be High->Low or Low->High.
     * Finish     - sets the desired finish point of the PP phase, same as above can be from High->Low or Low->High.
     * Hold       - sets the length of the PP hold period, if it's desired to maintain the "Start" pressure for a period of time before the pressure drop/raise is applied this is where it's done.
     * Length     - sets the length(aka speed) of the PP drop/raise behaviour, so one can change the pressure slow or fast if desired.
 * **Brew(Manual)** - allows for manual pressure control at brew time.
 * **DESCALE**     - enables the descaling program, at this point there's only one default behaviour:

         flush - 10s x5 at 2bar
         flush - 20s x5 at 1 bar
         idle  - 5min at 0 bar

# **Deprecations:**
* Pressure control:
> Pump actuation was switched form PWM to PSM and as a result, if PWM is preferred then **release-0.2.2** is the one to stay on. Static pressure control is only possible by using PWM as a result if there are no pressure sensing devices installed it's advised to stay on PWM. 

* **ACS712**
> As of **release-0.2.3** the hall effect sensor was deprecated due to it's unstable work and configuration complexity, it is highly recommended to update to the optocoupler board that took it's place as not doing that means being stuck on **realeas-0.2.2** feature set.

# TIMELINE
## Initial
> * Purchase the parts listed, from Ali and expect a wait of 21 days. Any parts purchased anywhere else are done so at your own risk (they've not been tested).
> * Connect test components described in the doc to Arduino - using the expansion board twist the ends of cables and connect to the screw terminals. At this point using DuPont connections is fine but please note later we will solder to the boards or pins.
> * Flash Arduino and LCD with code.
> * Plug in and test - check for a temp reading (it will contain the default offset of 7 degrees which means the initial temp will be room temp -7).

## Base
> * Plan out where the components will sit inside the machine to determine cable length
> * Create piggyback cables. Determine what switch points to piggyback from.
> * Wire in power delivery method - isolate the board using an enclosure or tape it up after wiring.
> * If you have the eco timer, disable it.
> * Swap out thermocouple - ease out the boiler (don't fully remove it) in order to gain more access.
> * Install the max temp board - isolate the board using an enclosure or tape it up after wiring.
> * Place and wire relay - attach the brew thermostat wires to the SSR relay and sit/attach the back plate or relay on the body of the machine, add some thermal paste
> * Re-Wire the steam switch for steam handling - you need to swap the brew thermostat wires (above step) for the steam thermostat wires and bridge the brew thermostat wires together then take some wires from the steam switch to the Arduino.
> * Wire brew switch for continuity 
> * Test.

## Extended
> * Install dimmer - isolate the board using an enclosure or tape it up after wiring.
> * Install the pressure sensor.
> * Install the load cells.

## Finish
> * Make sure all connections are proper i.e, no metal is exposed and well isolated, all soldering is perfect and wrapped in heat-shrink.
> * Flash the Arduino and LCD with the latest version from GitHub (there could have been changes since).
> * Record your first start. Post this to #first-start on discord.
> * Find out your regional settings and set them in the settings of the Arduino.
> * Check all other settings save correctly.
> * Record your first shot. Post this to #first-shot on discord.