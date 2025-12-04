//Servo EEPROM move functions
//byte rotInterimPos = 0;
bool RotInPos = false;

//function to safely move Rotation servo in set stepsize (speed)
bool MoveRotationServo(byte pos, byte stepSize, bool limitArmYpos) {
  //rotInterimPos = RotStoredPos;  //init interimPos
  RotInPos = false;

  if (!ServoPositionKnown) {
    RotStoredPos = EEPROM.read(RotationEEPROMAddress);
  }

  if (RotStoredPos <= pos - stepSize) {  //if stepsize still fits
    //move forward
    RotStoredPos += stepSize;
    //for (byte interimPos = RotStoredPos; interimPos < pos; interimPos += stepSize) {
    Rotation.write(RotStoredPos);
    //RotStoredPos = interimPos;
    delay(10);
    //}

  } else if (RotStoredPos >= pos + stepSize) {  //curent position > GoToPosition
    //move backward
    RotStoredPos -= stepSize;
    //for (byte interimPos = RotStoredPos; interimPos > pos; interimPos -= stepSize) {
    Rotation.write(RotStoredPos);
    //RotStoredPos = rotInterimPos;
    delay(10);

  } else if ((RotStoredPos > pos - stepSize && RotStoredPos < pos + stepSize) || RotStoredPos == pos) {  //is in closest position it can reach with set stepSize
    //write pos last time in case stepSize does not end at pos exactly
    Rotation.write(pos);
    RotInPos = true;
  }

  if (RotInPos) {
    RotStoredPos = pos;
    //write position back to EEPROM after move
    EEPROM.update(RotationEEPROMAddress, RotStoredPos);
  }

  if (limitArmYpos) {
    //if (calculateArmYpos(RotStoredPos, TiltStoredPos) < -100) {
      //tilt to prevent more offset
      //Serial.println(TiltAngle(RotStoredPos, -100));
      MoveTiltServo(TiltAngle(RotStoredPos, -100), 1);
    //}
  }



  //Serial.println(TiltAngle(RotStoredPos, -15));

  return RotInPos;
}



bool TiltInPos = false;
//function to safely move Tilt servo in set stepsize (speed)
bool MoveTiltServo(byte pos, byte stepSize) {
  TiltInPos = false;

  if (!ServoPositionKnown) {
    TiltStoredPos = EEPROM.read(TiltEEPROMAddress);
  }

  if (TiltStoredPos <= pos - stepSize) {  //if step still fits
    //move forward
    TiltStoredPos += stepSize;
    //for (byte interimPos = TiltStoredPos; interimPos < pos; interimPos += stepSize) {
    Tilt.write(TiltStoredPos);
    //TiltStoredPos = interimPos;
    delay(10);
    //}

  } else if (TiltStoredPos >= pos + stepSize) {
    //move backward
    TiltStoredPos -= stepSize;
    //for (byte interimPos = TiltStoredPos; interimPos > pos; interimPos -= stepSize) {
    Tilt.write(TiltStoredPos);
    //TiltStoredPos = interimPos;
    delay(10);
    //}
  } else if ((TiltStoredPos > pos - stepSize && TiltStoredPos < pos + stepSize) || TiltStoredPos == pos) {  //is in closest position it can reach with set stepSize
    //write pos last time in case stepSize does not end at pos exactly
    Tilt.write(pos);
    TiltInPos = true;
  }

  if (TiltInPos) {
    TiltStoredPos = pos;
    //write position back to EEPROM after move
    EEPROM.update(RotationEEPROMAddress, RotStoredPos);
  }

  //Serial.print("Position stored: ");
  //Serial.println(TiltStoredPos);
  calculateArmYpos(RotStoredPos, TiltStoredPos);

  return TiltInPos;
}


bool ClampInPos = false;
//function to safely move Clamp servo in set stepsize (speed)
bool MoveClampServo(byte pos, byte stepSize) {
  ClampInPos = false;

  if (!ServoPositionKnown) {
    ClampStoredPos = EEPROM.read(ClampEEPROMAddress);
  }


  if (ClampStoredPos <= pos - stepSize) {
    //move forward
    ClampStoredPos += stepSize;
    Clamp.write(ClampStoredPos);
    delay(50);

  } else if (ClampStoredPos >= pos + stepSize) {
    //move backward
    ClampStoredPos -= stepSize;
    Tilt.write(ClampStoredPos);
    delay(50);
  } else if ((ClampStoredPos > pos - stepSize && ClampStoredPos < pos + stepSize) || ClampStoredPos == pos) {
    //last step
    Clamp.write(pos);
    ClampInPos = true;
    delay(50);
  }

  if (ClampInPos) {
    ClampStoredPos = pos;
    //write position back to EEPROM after move
    EEPROM.update(ClampEEPROMAddress, ClampStoredPos);
  }

  //Serial.print("Position stored: ");
  //Serial.println(ClampStoredPos);
  return ClampInPos;
}


bool ArmHeightInPos = false;
//function to safely move Arm Height servo in set stepsize (speed)
bool MoveArmHeightServo(byte pos, byte stepSize) {
  ArmHeightInPos = false;

  if (!ServoPositionKnown) {
    ArmHeightStoredPos = EEPROM.read(ArmHeightEEPROMAddress);
  }


  if (ArmHeightStoredPos <= pos - stepSize) {
    //move forward
    ArmHeightStoredPos += stepSize;
    toneArmHeight.write(ArmHeightStoredPos);
    delay(50);

  } else if (ArmHeightStoredPos >= pos + stepSize) {
    //move backward
    ArmHeightStoredPos -= stepSize;
    toneArmHeight.write(ArmHeightStoredPos);
    delay(50);
  } else if ((ArmHeightStoredPos > pos - stepSize && ArmHeightStoredPos < pos + stepSize) || ArmHeightStoredPos == pos) {
    //last step
    toneArmHeight.write(pos);
    ArmHeightInPos = true;
    delay(50);
  }

  if (ArmHeightInPos) {
    ArmHeightStoredPos = pos;
    //write position back to EEPROM after move
    EEPROM.update(ArmHeightEEPROMAddress, ArmHeightStoredPos);
  }
  //Serial.print("Position stored: ");
  //Serial.println(ArmHeightStoredPos);
  return ArmHeightInPos;
}


bool ArmPosInPos = false;
//function to safely move Arm Height servo in set stepsize (speed)
bool MoveArmPosServo(int pos, byte stepSize) {
  ArmPosInPos = false;

  if (!ServoPositionKnown) {
    ArmPosStoredPos = EEPROM.read(ArmPosEEPROMAddress);
  }

  if (ArmPosStoredPos <= pos - stepSize) {
    //move forward
    ArmPosStoredPos += stepSize;
    toneArmPos.writeMicroseconds(ArmPosStoredPos);
    delay(5);

  } else if (ArmPosStoredPos >= pos + stepSize) {
    //move backward
    ArmPosStoredPos -= stepSize;
    toneArmPos.writeMicroseconds(ArmPosStoredPos);
    delay(5);

  } else if ((ArmPosStoredPos > pos - stepSize && ArmPosStoredPos < pos + stepSize) || ArmPosStoredPos == pos) {
    //write pos last time in case stepSize does not end at pos exactly
    toneArmPos.write(pos);
    ArmPosInPos = true;
  }


  if (ArmPosInPos) {
    ArmPosStoredPos = pos;
    //write position back to EEPROM after move
    EEPROM.put(ArmPosEEPROMAddress, ArmPosStoredPos);
  }

  return ArmPosInPos;
}


void ShakeRotation(int pos, int amount) {
  for (int i = 0; i < amount; i++) {
    while (!MoveRotationServo(pos - 5, 2, false)) {}
    while (MoveRotationServo(pos + 5, 2, false)) {}
  }
}


float calculateArmYpos(byte Rot, byte Tilt) {
  //constants:
  byte L = 240;  //length of arm from rotation point to center of plate
  byte r = 165;  //radius of arm in mm


  //angles
  float Phi = Rot - 26;

  float scaledTilt = (180.0 / (134.0 - 4.0)) * (float(Tilt) - 4.0);
  float Theta = scaledTilt - 90;
  //Serial.println(Theta);

  //y-offset of furthest armpoint from middlepoint, backwards
  float l = float(r) * sin((PI / 180.0) * Phi) * sin((PI / 180.0) * Theta);
  //Serial.println(l);

  //y component of middle of arm compared to fixed alu profile
  float Ny = float(L) * cos((PI / 180.0) * Phi);
  //Serial.println(Ny);

  //total offset of furthest point of arm
  float yPos = Ny - l;
  Serial.println(yPos);
  return yPos;
}


/*
reverse tilt calculation:
l / (float(r) * sin((PI / 180.0) * double(Phi))) = sin((PI / 180.0) * double(Theta));
Theta = asin( l / (float(r) * sin((PI / 180.0) * double(Phi)) )

l = Ny - ypos, yPos is given
Ny = float(L) * cos((PI / 180.0) * double(Phi));

*/
byte TiltAngle(byte RotPos, float MaxYPos) {
  //constants
  byte L = 240;  //length of arm from rotation point to center of plate
  byte r = 165;  //radius of arm in mm

  float Phi = RotPos - 26;
  float Ny = float(L) * cos((PI / 180.0) * double(Phi));
  float l = Ny - MaxYPos;

  if (l > 0 && l < r) {
    //tilt is only needed when l > 0, as otherwise tilt would move the arm towards to player (inwards)
    //tilt is only possible when l < r, as otherwise the distance needed to get to yPos is greater than what tilt can provide
    float Theta = asin(l / (float(r) * sin((PI / 180.0) * double(Phi))));
    float ThetaDeg = (180 / PI) * Theta;
    float scaledTilt = ThetaDeg + 90;
    float tilt = ((134.0 - 4.0) / 180) * scaledTilt + 4;
    /*
    Serial.print("Theta: ");
    Serial.println(ThetaDeg);

    Serial.print("Tilt: ");
    Serial.println(tilt);

    Serial.print("yPos: ");
    calculateArmYpos(RotPos, tilt);
*/
    return byte(tilt);  //floor van float, gaat goed doordat naar beneden afronden ervoor zorgt dat we altijd binnen yMax vallen
  }
  return 0;
}
