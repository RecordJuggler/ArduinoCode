//toneArmPos; //150: full left (home), ~107: startpos plate, ~0: endpos plate
//toneArmHeight; //0: down, 60: up

/*
 * Servo S-curve:
 * Sigmoid function
 * S(x)=A/(1+e^(-Bx+C))
 * A = bereik curve, 0->A
 * B = slope
 * C = shift links/rechts (positief naar rechts)
 * 
 * 
 */


//Clamp Min Max:
//in: 40, clamp closed
//Out: 140, clamp open


enum toneHeight {
  DOWN = 0,
  UP = 40
} toneArmHeightEnum;

enum tonePos {
  BASE = 170,
  START = 105,
  END = 0
} toneArmPosEnum;


enum RotationPos {
  //determine correct angles for Rotation
  RotationIn = 50,     //rotation where center of LP is on center of stack/player
  RotationOutFront = 130,  //rotation far enough out to freely tilt or move stepper on A side/front
  RotationOutB = 20,   //rotation far enough out to freely tilt or move stepper on B side
  RotationPos1A = 65,  //rotation where inside is just around LP A side
  RotationPos2A = 58,  //rotation where tip is just around LP A side
  RotationPos1B = 35,  //rotation where inside is just around LP B side
  RotationPos2B = 40   //rotation where tip is just around LP B side
} RotationArmPosEnum;


enum ClampPos {
  ClampClose = 40,
  ClampOpen = 140
} ClampPosEnum;
/*tilt:
5: horizontal, opening outwards
135: horizontal, opening inwards (towards record player), 130 puts LP slightly higher
70: vertical, opening up
75: probably save to move stepper up/down
*/
enum TiltPos {
  TiltHorizontalA = 133,
  TiltHorizontalB = 4,
  TiltVertical = 70,
  TiltSlightlyVerticalA = 125,
  TiltSlightlyVerticalB = 10
} TiltArmPosEnum;


int LPPositions[] = {
  120,  //[0] player top pos, free to rotate and tilt slightly
  400,  //[1] storage pos 1
  440,  //[2] storage pos 2
  480,  //[3] storage pos 3
  520,  //[4] storage pos 4
  560,  //[5] storage pos 5
  600,  //[6] storage pos 6
  640,  //[7] storage pos 7
  200,  //[8] storage pos 8 (potential)
  200,  //[9] storage pos 9 (potential)
  175,  //[10] player bottom pos
  165,  //[11] player lift pos1
  160,  //[12] player lift pos 2
  40,   //[13] top pos, free to tilt above player
};


/*
pick sequence positions:
A side:
xs (stepper) ~170, height to start reach in to pick from player
xt tilt 135
xr rotate in to ~65
xs stepper up to 165 to lift LP slightly on arm base side
xr rotate in to 60
xs stepper up to 158
xr rotate in to 50
xc clamp 40, dicht
xs move stepper up (to 120 should be fine)
xt tilt naar 125 om lp better vast te houden
xr rotate 130
xt tilt 70

B side:
xt tilt 01 (0)
xr rotate 20 (max till 13 until you hit the side against the alu profile)
xs (stepper) ~172, height to start reach in to pick from player
xr rotate 35, slightly lift lp with inner arm
xs stepper 168, lift lp and set outer arm on level of LP
xr rotate 42, reach LP with outer arm
xs stepper 155, lift LP a little again (but stay on center pin)
xr rotate ~50, grab LP with outer arm
xs stepper 40
xc clamp close (40)
xt tilt 10, hold better for rotation movement
xr rotate 130 (out)
xt tilt 70 vertical


place sequence positions:
A side:
xt Tilt 135 horizontal (A-side)
xr rotate 50
xs stepper 160, slightly on pin
xr rotate 52, back to 50 quickly to shake LP onto pin (correct for inaccuracy of servo)
xc clamp 140 open
xr rotate 130
xt tilt 70
Stepper free to move


B side
xs stepper 40 up to allow tilt to move above player
xt tilt 07 tilt horizontal (B-side)
xr rotate 50
xs stepper 160
xr rotate 52, back to 50 quickly to shake LP onto pin (correct for inaccuracy of servo)
xc clamp 140 open
xr rotate 20, release LP onto pin
xs stepper 40
xr rotate 130
xt tilt 70
Stepper free to move


*/

//stepper max pos before bottoming out
int maxPos = 650;