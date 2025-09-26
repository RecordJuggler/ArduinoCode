**Arduino code for RecordJuggler**


**Serial commands:**

Serial communication is setup with a BaudRate of 115200.
At startup, the RecordJuggler will first perform a homing sequence to home the stepper towards the top and set all servo axes to safe positions.
After this, the Step Cycle will go to CaseStep will go to the step called 'JukePiCommand', after which it will be able to listen to new main commands. 
All commands sent through the serial communication are Case-INsensitive, meaning capital letters act as normal letters. e.g. 'DemoPickA', 'demopicka' and 'DeMoPiCkA' all give the same result.

From the step 'JukePiCommand', a few demo cycles can be called



**Current Demo-Cycles:**
- demoPickA
-   Performs a pick-sequence from the player in a way that the 'A' side of the LP is facing UP
- demoPickB
-   Performs a pick-sequence from the player in a way that the 'B' side of the LP is facing UP ('A' side is facing DOWN)
- demoPlaceA
-   Performs a place-sequence so that the 'A' side of the LP will be played by the record player
- demoPlaceB
-   Performs a place-sequence so that the 'B' side of the LP will be played by the record player
- demoPickRack
-   Performs a pick-sequence from the rack, after this command, you need to enter a value between 1-9 (including 1 and 9) to specify which rack position to pick from 
- demoPlaceRack
-   Performs a place-sequence into the rack, after this command, you need to enter a value between 1-9 (including 1 and 9) to specify which rack position to place into
-   

During prototyping, a few of these demos are still in testing and may require manual input to continue from one step to the next.
This can be done using the 'next' command. To automatically finish the sequence without having to send 'next' each time, the 'finish' command can be sent.
The demo sequence will then run automatically until the demo is finished and will end in the 'JukePiCommand' caseStep, awaiting a new (demo)command.



**Manual control of individual axes:**

To manually control each axis, some manual-control commands have been added.
These can be incredibly usefull to send the RecordJuggler to a certain position to store that position for an automatic sequence.
All Manual-control commands are identified by starting them with the letter 'x'.
The next character defines the axis that needs to be moved:
- S : Stepper
- R : Rotation
- T : Tilt
- C : Clamp
- U : ToneArmHeight
- I : ToneArmPosition

After these two letters, it is required to set a position you want the axis to go to.
This can be 1-270 for most servos, but 1-605 for the stepper.

A value of 0 is not possible, as the command is sent as a string and needs to be converted to an Int.
When the conversion is invalid, the toInt() function returns 0, which would then also result in the axis moving to position 0.
To prevent unwanted move to 0, this value is filtered out and regarded as an error in the ToInt() parsing.

A complete manual-control command would look like this:

xs250  : Moves the stepper to position 250

xr130  : Moves rotation to 130, this is the 'outwards' position 



**Other simple commands:**

33 : Starts the recordPlayer turning at 33RPM
45 : Starts the recordPlayer turning at 45RPM
stop : Stops any recordPlayer turning
togglepower : toggles the power of the recordPlayer, the player has a relay inside that can bypass the physical power button. This power is different from the RecordJuggler power.
up : Move the toneArm up
down: moves the toneArm down




**Case cycle**

As previously mentioned, the main cycle of the RecordJuggler is setup using case steps.
This means the cycle is structured using a switch-case structure.

current caseSteps include:
- startup,
- homing,
- jukePiCommand,

//Steps 1xx: Pick A-side sequence from player
- Step100_PickA_StepperPickBottomPos,
- Step110_PickA_TiltArmHorizontal,
- Step120_PickA_RotArmPickPos1,
- Step130_PickA_StepperPickIntermediatePos,
- Step135_PickA_RotArmPickPos2,
- Step140_PickA_StepperPickPos2,
- Step150_PickA_RotArmPickIn,
- Step160_PickA_CloseClamp,
- Step170_PickA_StepperPickTopPos,
- Step180_PickA_TiltPickSlightlyVertical,
- Step190_PickA_RotArmPickOutwards,
- Step195_PickA_TiltPickVertical,

//steps 2xx: Pick B-side sequence from player
- Step200_PickB_StepperPickTopPos,  //stepper to top to allow tilt horizontal
- Step210_PickB_TiltArmHorizontal,  //tilt arm horizontal for B-side
- Step220_PickB_RotArmPickOut,      //rotate arm in to 20 (maybe not necessary?)
- Step230_PickB_StepperBottomPos,
- Step240_PickB_RotArmPickPos1,            //rotate arm in to 35, slightly lift LP with inner arm
- Step250_PickB_StepperIntermediatePos,    //lift arm so outer arm is level with LP
- Step260_PickB_RotArmPickPos2,            //reach LP with outer arm
- Step270_PickB_StepperPickPos2,           //move stepper up but stay on center pin
- Step280_PickB_RotArmPickIn,              //Rotate arm to 50 to fully pick LP
- Step290_PickB_CloseClamp,                //does it even need a comment?..
- Step291_PickB_StepperPickTopPos,         //stepper up to 40 to allow tilt again
- Step292_PickB_TiltPickSlightlyVertical,  //tilt slightly to allow better rotation movement without potentially losing LP
- Step293_PickB_RotArmPickOutwards,        //Rotate arm out to 130
- Step295_PickB_TiltPickVertical,

//steps 3xx: Place A-side sequence on player
- Step300_PlaceA_stepperPlaceTopPos,     //stepper to height where we can place LP (120-ish should be fine?)
- Step310_PlaceA_TiltPlaceHorizontal,    //tilt horizontal A-side to place A side on top
- Step320_PlaceA_RotArmPlaceIn,          //rotate arm in to place position (should be the same for all picks and places)
- Step330_PlaceA_StepperPlaceCenterPin,  //stepper to height where LP is just within center pin
- Step340_PlaceA_RotArmShakeOnPin,       //Special move to shake rotation around 50-52-ish to account for inaccuracy and make sure LP is on Pin
- Step350_PlaceA_StepperPlaceBottomPos,  //move stepper down a bit (maybe not necessary?)
- Step360_PlaceA_ClampOpen,
- Step370_PlaceA_RotArmPlaceOutwards,  //move arm outwards to 130
- Step380_PlaceA_TiltPlaceVertical,    //tilt vertically again when free to do so
 
//steps 4xx: Place B-side sequence on player
- Step400_PlaceB_StepperPlaceTopPos,     //move stepper to 40 to allow tilt for B-side
- step410_PlaceB_TiltPlaceHorizontal,    //tilt horizontal for B-side (around 7deg)
- Step420_PlaceB_RotArmPlaceIn,          //rotate arm in to place position (should be the same for all picks and places)
- Step430_PlaceB_StepperPlaceBottomPos,  //stepper to height where LP is just within center pin
- Step440_PlaceB_RotArmShakeOnPin,       //Special move to shake rotation around 50-52-ish to account for inaccuracy and make sure LP is on Pin
- Step450_PlaceB_ClampOpen,
- Step460_PlaceB_RotArmPlaceRelease,   //rotate arm out to 20 to release LP onto pin
- Step470_PlaceB_StepperPlaceTopPos,   //move stepper up to allow tilt back
- Step480_PlaceB_RotArmPlaceOutwards,  //rotate out to 130
- Step490_PlaceB_TiltPlaceVertical,    //tilt vertically again when free to do so

//steps 5xx: Pick sequence from rack
- Step500_PickRack_StepperPickRackPos,     //stepper to position
- Step510_PickRack_TiltHorizontal,         //tilt vertcially A side (B side not possible on storage)
- step520_PickRack_RotateInPos1,           //rotate in, touch with inner side of arm
- Step530_PickRack_StepperPickPos2,        //stepper up 5mm, line up outer arm with lp
- Step540_PickRack_RotateIn,               //rotate in completely for storage
- Step550_PickRack_CloseClamp,             //Close clamp
- Step560_PickRack_StepperPickTop,         //stepper up 15/17mm (max 18mm with current spacing). to clear pin
- Step570_PickRack_RotateOut,              //rotate out to front
- Step580_PickRack_StepperToClearancePos,  //optionally, move stepper down before moving tilt vertical
- Step585_PickRack_TiltVertical,           //tilt vertical, ONLY POSSIBLE WHEN HEIGHT > 420!

//steps 6xx: Place sequence on rack
- Step600_PlaceRack_StepperPlaceTopPos,    //stepper to top place position
- Step601_PlaceRack_MoveStepperToSafePos,  //intermediate step to move down before doing tilt to prevent collision
- Step602_PlaceRack_TiltHorizontally,      //tilt horizontally at correct height
- Step603_PlaceRack_StepperPlaceTopPos,    //move back to up to correct height
- Step610_PlaceRack_TiltHorizontally,      //tilt horizontally A side (B side not possible for storage)
- Step620_PlaceRack_RotateIn,              //rotate in completely so hole of lp lines up with pin
- Step630_PlaceRack_StepperBottomPos,      //stepper down to place bottom pos
- Step635_PlaceRack_RotArmShakeOnPin,
- Step640_PlaceRack_OpenClamp,             //release LP from clamp
- Step650_PlaceRack_RotateOut,             //rotate out to front
- Step660_PlaceRack_StepperToClearancePos, //optionally, move stepper down before moving tilt vertical
- Step665_PlaceRack_TiltVertically,        //tilt vertical, ONLY POSSIBLE WHEN HEIGHT > 420!
 
stopCommand,  //from either JukePi or player end sensor
startPlaying,
stopPlaying


As can be seen, the main cycle steps are divided into value ranges of 100. 
Steps 1xx are reserved for steps that perform a pick-cycle from the recordPlayer from the 'A'-side
Steps 2xx are reserved for steps that perform a pick-cycle from the recordPlayer from the 'B'-side
Steps 3xx are reserved for steps that perform a place-cycle from the recordPlayer on the 'A'-side
Steps 4xx are reserved for steps that perform a place-cycle from the recordPlayer on the 'B'-side
Steps 5xx are reserved for steps that perform a pick-cycle from the storage rack
Steps 6xx are reserved for steps that perform a place-cycle into the storage rack

Main steps within a cycle jump with a value of 10, while smaller intermediate steps can change by just 1.



as of writing this, no official commands between the JukePi and Arduino has been setup, so no official commands have been created for that part.
Right now all these commands are meant for human control using a pc/laptop directly connected to the Arduino.









