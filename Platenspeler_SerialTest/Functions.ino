//Servo EEPROM move functions
//byte rotInterimPos = 0;
bool RotInPos = false;

//function to safely move Rotation servo in set stepsize (speed)
bool MoveRotationServo(byte pos, byte stepSize) {
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
    delay(50);
    //}

  } else if (RotStoredPos >= pos + stepSize) {  //curent position > GoToPosition
    //move backward
    RotStoredPos -= stepSize;
    //for (byte interimPos = RotStoredPos; interimPos > pos; interimPos -= stepSize) {
    Rotation.write(RotStoredPos);
    //RotStoredPos = rotInterimPos;
    delay(50);

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

  calculateArmYpos(RotStoredPos, TiltStoredPos);

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
    delay(50);
    //}

  } else if (TiltStoredPos >= pos + stepSize) {
    //move backward
    TiltStoredPos -= stepSize;
    //for (byte interimPos = TiltStoredPos; interimPos > pos; interimPos -= stepSize) {
    Tilt.write(TiltStoredPos);
    //TiltStoredPos = interimPos;
    delay(50);
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


//function to safely move Clamp servo in set stepsize (speed)
void MoveClampServo(byte pos, byte stepSize) {
  //Serial.println(pos);

  if (!ServoPositionKnown) {
    ClampStoredPos = EEPROM.read(ClampEEPROMAddress);
  }

  if (ClampStoredPos < pos) {
    //move forward
    for (byte interimPos = ClampStoredPos; interimPos < pos; interimPos += stepSize) {
      Clamp.write(interimPos);
      ClampStoredPos = interimPos;
      delay(50);
    }

  } else if (ClampStoredPos > pos) {
    //move backward
    for (byte interimPos = ClampStoredPos; interimPos > pos; interimPos -= stepSize) {
      Clamp.write(interimPos);
      ClampStoredPos = interimPos;
      delay(50);
    }
  }
  //write pos last time in case stepSize does not end at pos exactly
  Clamp.write(pos);

  ClampStoredPos = pos;
  //write position back to EEPROM after move
  EEPROM.update(ClampEEPROMAddress, ClampStoredPos);
  //Serial.print("Position stored: ");
  //Serial.println(ClampStoredPos);
}


//function to safely move Arm Height servo in set stepsize (speed)
void MoveArmHeightServo(byte pos, byte stepSize) {
  //Serial.println(pos);

  if (!ServoPositionKnown) {
    ArmHeightStoredPos = EEPROM.read(ArmHeightEEPROMAddress);
  }

  if (ArmHeightStoredPos < pos) {
    //move forward
    for (byte interimPos = ArmHeightStoredPos; interimPos < pos; interimPos += stepSize) {
      toneArmHeight.write(interimPos);
      ClampStoredPos = interimPos;
      delay(50);
    }

  } else if (ArmHeightStoredPos > pos) {
    //move backward
    for (byte interimPos = ArmHeightStoredPos; interimPos > pos; interimPos -= stepSize) {
      toneArmHeight.write(interimPos);
      ArmHeightStoredPos = interimPos;
      delay(50);
    }
  }
  //write pos last time in case stepSize does not end at pos exactly
  toneArmHeight.write(pos);

  ArmHeightStoredPos = pos;
  //write position back to EEPROM after move
  EEPROM.update(ArmHeightEEPROMAddress, ArmHeightStoredPos);
  //Serial.print("Position stored: ");
  //Serial.println(ArmHeightStoredPos);
}


//function to safely move Arm Height servo in set stepsize (speed)
void MoveArmPosServo(byte pos, byte stepSize) {
  //Serial.println(pos);

  if (!ServoPositionKnown) {
    ArmPosStoredPos = EEPROM.read(ArmPosEEPROMAddress);
  }

  if (ArmPosStoredPos < pos) {
    //move forward
    for (byte interimPos = ArmPosStoredPos; interimPos < pos; interimPos += stepSize) {
      toneArmPos.write(interimPos);
      ClampStoredPos = interimPos;
      delay(50);
    }

  } else if (ArmPosStoredPos > pos) {
    //move backward
    for (byte interimPos = ArmPosStoredPos; interimPos > pos; interimPos -= stepSize) {
      toneArmPos.write(interimPos);
      ArmPosStoredPos = interimPos;
      delay(50);
    }
  }
  //write pos last time in case stepSize does not end at pos exactly
  toneArmPos.write(pos);

  ArmPosStoredPos = pos;
  //write position back to EEPROM after move
  EEPROM.update(ArmPosEEPROMAddress, ArmPosStoredPos);
  //Serial.print("Position stored: ");
  //Serial.println(ArmPosStoredPos);
}


void ShakeRotation(int pos) {
  while(!MoveRotationServo(pos - 5, 2)){}
  while(MoveRotationServo(pos + 5, 2)){}
}


void calculateArmYpos(byte Rot, byte Tilt) {
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
  //Serial.println(yPos);
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
