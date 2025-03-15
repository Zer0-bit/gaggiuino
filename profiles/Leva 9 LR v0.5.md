I wanted a lever profile, that could behave more like a real spring lever. 
The problem I had with existing lever profiles was that the declining pressure curve is time-dependent. 
So if, for example, one would grind too fine, the pressure would drop after a certain time, even if not much liquid got into the cup. 
In a spring lever, the pressure is not time-dependent, but dependent on the spring tension (and of course the resistance of the puck), meaning the pressure drops as the water inside the cylinder gets less and by that relaxing the spring.
So I tried emulating that with Gaggiuino by setting up multiple incremental pressure phases with “water pumped” as the stop condition, starting from 9 bar to 6 bar.
[Original creator post can be found here](https://discord.com/channels/890339612441063494/1326340673950973962/1326340673950973962)

*Preinfusion (Phases 1-3):*

For it to work properly, headspace and puck should be fully saturated.
I ensure this within my preinfusion phases, stopping either on 4 bar or 1 g in the cup.
I have it set to reduce flow as pressure rises in 3 phases.
You can change the preinfusion to fit your liking.

*Soak (Phase 4):*

The following soak is optional.
For now, it stops either after 5 secs, pressure dropping below 2 bar or 3 g in cup.

*Main Extraction (Phases 5 onwards):*

I played around with pressure and water pumped for each phase and the number of phases.
Right now I settled with this, but also 9-3, 8-5, etc. is possible.
For me, it reaches 6 bar after about 40 ml in the cup.
Basically the combination of pressure and pumped water of all phases defines the spring behavior and cylinder capacity of the simulated lever.

Additionally, I limited the flow in the main phases to 3 ml/s to prevent “gushers”.
The last phase continues until the desired weight is reached, so in this case it continues with 6 bar.
I only tried it with my 24.5 mm IMS competition basket and puck screen with 16 g of coffee.

*__This profile is extremely dependent on the calculated “pump flow”, so PZ calibration is important!__*

**Example shot graph:**
<img src="https://github.com/user-attachments/assets/4918e1db-3ff5-45fa-b61a-4ff94ec8cd1e" alt="example_graph" width="10%" height="10%">
