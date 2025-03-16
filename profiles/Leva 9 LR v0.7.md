*A profile that emulates a spring lever*

The pressure decline of a real spring lever machine is not dependent on elapsed time, but on the volume of water displaced inside the cylinder.
This profile tries to emulate this behaviour with Gaggiuino by setting up multiple incremental pressure phases with "water pumped" as stop conditions, declining from 9 to 6 bar.

[Original creator post can be found here](https://discord.com/channels/890339612441063494/1326340673950973962/1326340673950973962).

*Preinfusion (Phases 1-3):*

For the profile to work properly, the headspace and puck must be fully saturated at the end of preinfusion.
Preinfusion starts with fast flow and slows down as the pressure rises.
The last phase is a 3 bar hold, stopping either after 15 seconds or 3 grams in the cup.

*Main Extraction (Phases 4 onwards):*

The combination of "pressure" and "pumped water" of all phases defines the "spring behavior" and "cylinder capacity" of the simulated lever.

Extraction starts with a rise to 9 bar and continues to decline to 6 bar.
The output weight after reaching 6 bar should be about 40 grams.
The flow in the main phases is limited to 3 ml/s to prevent "gushers".
The last phase continues with 6 bar if the desired weight is greater than 40 grams.

Different preinfusions, pressure declines, etc. are possible.
Feel free to change the profile to your liking! 

*__This profile is extremely dependent on the calculated "pump flow", so PZ calibration is important!__*
