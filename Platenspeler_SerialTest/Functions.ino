//Servo EEPROM move functions

//function to safely move Rotation servo in set stepsize (speed)
void MoveRotationServo(byte pos, byte stepSize) {

  if (!ServoPositionKnown) {
    RotStoredPos = EEPROM.read(RotationEEPROMAddress);
  }

  if (RotStoredPos < pos) {
    //move forward
    for (byte interimPos = RotStoredPos; interimPos < pos; interimPos += stepSize) {
      Rotation.write(interimPos);
      RotStoredPos = interimPos;
      delay(50);
    }

  } else if (RotStoredPos > pos) {
    //move backward
    for (byte interimPos = RotStoredPos; interimPos > pos; interimPos -= stepSize) {
      Rotation.write(interimPos);
      RotStoredPos = interimPos;
      delay(50);
    }
  }
  //write pos last time in case stepSize does not end at pos exactly
  Rotation.write(pos);

  RotStoredPos = pos;
  //write position back to EEPROM after move
  EEPROM.update(RotationEEPROMAddress, RotStoredPos);
  Serial.print("Position stored: ");
  Serial.println(RotStoredPos);
}




//function to safely move Tilt servo in set stepsize (speed)
void MoveTiltServo(byte pos, byte stepSize) {
  //Serial.println(pos);

  if (!ServoPositionKnown) {
    TiltStoredPos = EEPROM.read(TiltEEPROMAddress);
  }

  if (TiltStoredPos < pos) {
    //move forward
    for (byte interimPos = TiltStoredPos; interimPos < pos; interimPos += stepSize) {
      Tilt.write(interimPos);
      TiltStoredPos = interimPos;
      delay(50);
    }

  } else if (TiltStoredPos > pos) {
    //move backward
    for (byte interimPos = TiltStoredPos; interimPos > pos; interimPos -= stepSize) {
      Tilt.write(interimPos);
      TiltStoredPos = interimPos;
      delay(50);
    }
  }
  //write pos last time in case stepSize does not end at pos exactly
  Tilt.write(pos);

  TiltStoredPos = pos;
  //write position back to EEPROM after move
  EEPROM.update(TiltEEPROMAddress, TiltStoredPos);
  Serial.print("Position stored: ");
  Serial.println(TiltStoredPos);
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
  Serial.print("Position stored: ");
  Serial.println(ClampStoredPos);
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
  Serial.print("Position stored: ");
  Serial.println(ArmHeightStoredPos);
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
  Serial.print("Position stored: ");
  Serial.println(ArmPosStoredPos);
}


void ShakeRotation(int pos) {
  MoveRotationServo(pos - 5, 2);
  MoveRotationServo(pos + 5, 2);
}
