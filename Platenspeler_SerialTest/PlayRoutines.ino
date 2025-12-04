
int toneArmPosition = 0;
void startPlay() {

  switch (subroutineSteps) {
    case 0:
      Serial.println("0");
      //move arm down, to allow positioning to go to base without interference
      //toneArmHeightEnum = DOWN;
      if (MoveArmHeightServo(DOWN, 2)) {
        //toneArmHeight.write(DOWN);
        subroutineMillis = millis();
        subroutineSteps = 1;
      }
      break;

    case 1:
      Serial.println("1");

      //if (millis() - subroutineMillis >= 1000) {
        subroutineSteps = 2;
      //}
      break;

    case 2:
      Serial.println("2");

      //move pos to base
      //toneArmPosEnum = BASE;
      if (MoveArmPosServo(BASE, 2)) {
        //toneArmPos.write(BASE);
        toneArmPosition = BASE;
        subroutineMillis = millis();
        subroutineSteps = 3;
      }
      break;

    case 3:
      Serial.println("3");

      //if (millis() - subroutineMillis >= 100) {
        subroutineSteps = 4;
      //}
      break;

    case 4:
      Serial.println("4");

      //arm back up
      //toneArmHeightEnum = UP;
      if (MoveArmHeightServo(UP, 1)) {
        //toneArmHeight.write(UP);
        subroutineMillis = millis();
        subroutineSteps = 5;
      }
      break;

    case 5:
      Serial.println("5");

      //if (millis() - subroutineMillis >= 2000) {
        subroutineSteps = 6;
      //}

      break;

    case 6:
      Serial.println("6");

      //slowly move arm to START pos
      //for (int i = BASE; i >= START; i--) {
      if (millis() - subroutineMillis >= 100 && toneArmPosition > START) {
        toneArmPosition--;
        if (MoveArmPosServo(toneArmPosition, 1)) {
          //toneArmPos.write(toneArmPosition);
          subroutineMillis = millis();
        }
      } else if (toneArmPosition <= START) {
        subroutineSteps = 7;
      }
      break;
      //delay(100);
      //}

    case 7:
      Serial.println("7");

      //arm back down
      //toneArmHeightEnum = DOWN;
      if (MoveArmHeightServo(DOWN, 2)) {
        //toneArmHeight.write(DOWN);
        subroutineSteps = 8;
      }
      break;

    case 8:
      Serial.println("8");
      analogReadActive = true;
      subroutineDone = true;
      //if (transit) {
      subroutineSteps = 0;
      //}
      break;
  }
}

void StopPlay() {
  switch (subroutineSteps) {
    case 0:
      //Serial.println("Stop 0");
      analogReadActive = false;
      //move arm down, to allow positioning to go to base without interference
      if (MoveArmPosServo(END, 3)) {
        //toneArmPos.write(END);
        toneArmPosition = END;
        subroutineMillis = millis();
        subroutineSteps = 1;
      }
      break;

    case 1:
      //Serial.println("1");

      //if (millis() - subroutineMillis >= 100) {
        subroutineSteps = 2;
      //}
      break;

    case 2:
      //Serial.println("2");

      if (MoveArmHeightServo(UP, 1)) {
        //toneArmHeight.write(UP);
        subroutineMillis = millis();
        subroutineSteps = 3;
      }
      break;

    case 3:
      //Serial.println("3");

      //if (millis() - subroutineMillis >= 2000) {
        subroutineSteps = 4;
      //}
      break;

    case 4:
      //Serial.println("4");

      //from end to start pos
      if (millis() - subroutineMillis >= 50 && toneArmPosition < START) {
        toneArmPosition++;
        if (MoveArmPosServo(toneArmPosition, 1)) {
          //toneArmPos.write(toneArmPosition);
          subroutineMillis = millis();
        }
      } else if (toneArmPosition >= START) {
        subroutineSteps = 5;
      }
      break;

    case 5:
      //Serial.println("5");

      //to holder pos
      if (millis() - subroutineMillis >= 150 && toneArmPosition < 130) {
        toneArmPosition++;
        if (MoveArmPosServo(toneArmPosition, 1)) {
          //toneArmPos.write(toneArmPosition);
          subroutineMillis = millis();
        }
      } else if (toneArmPosition >= 130) {
        subroutineSteps = 6;
      }
      break;

    case 6:
      //Serial.println("6");

      toneArmHeightEnum = DOWN;
      if (MoveArmHeightServo(DOWN, 2)) {
        //toneArmHeight.write(DOWN);
        subroutineMillis = millis();
        subroutineSteps = 7;
      }
      break;

    case 7:
      //Serial.println("7");

      //if (millis() - subroutineMillis >= 1000) {
        subroutineSteps = 8;
      //}
      break;

    case 8:
      //Serial.println("8");

      if (millis() - subroutineMillis >= 100 && toneArmPosition < BASE) {
        toneArmPosition++;
        if (MoveArmPosServo(toneArmPosition, 1)) {
          //toneArmPos.write(toneArmPosition);
          subroutineMillis = millis();
        }
      } else if (toneArmPosition >= BASE) {
        subroutineSteps = 9;
      }
      break;

    case 9:
      //Serial.println("9");

      //end
      subroutineDone = true;
      subroutineSteps = 0;
      break;
  }
}
