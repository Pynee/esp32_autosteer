#include <stdint.h>
#include "configuration.h"

// Steer switch button  ***********************************************************************************************************
uint8_t pulseCount = 0; // Steering Wheel Encoder
// pwm variables
struct SteerAngleDeviation
{
  float value;
  float proportional;
  float absolute;
};
SteerAngleDeviation steerAngleDeviation;

int16_t pwmDrive = 0;
void calcSteeringPID();
void EncoderFunc();
void calcSteerAngle();
void motorDrive();

void autosteerLoop()
{
  // If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
  if (watchdogTimer++ > 250)
    watchdogTimer = WATCHDOG_FORCE_VALUE;
  if (state.guidanceStatus == 1 && (ReceivedData.gpsSpeed < 0.1))
  {
    watchdogTimer = 0;                // reset watchdog
    if (steerConfig.SteerSwitch == 1) // steer switch on - off
    {
      state.autoSteerEnabled = !hardwareSwitches.steerSwitch.getState();
    }
    else if (steerConfig.SteerButton == 1) // steer Button momentary
    {
      if (hardwareSwitches.steerSwitch.stateChanged() && !hardwareSwitches.steerSwitch.getState())
      {
        state.autoSteerEnabled = !state.autoSteerEnabled;
      }
    }
    if (steerConfig.ShaftEncoder)
    {
      if (hardwareSwitches.remoteSwitch.stateChanged() && hardwareSwitches.remoteSwitch.getState())
      {
        pulseCount++;
        if (pulseCount >= steerConfig.PulseCountMax)
        {
          state.autoSteerEnabled = false;
        }
      }
    }
  }
  else
  {
    state.autoSteerEnabled = false;
  }

  if (state.autoSteerEnabled)
  {
    calcSteerAngle();
    calcSteeringPID(); // do the pid
  }

  if (watchdogTimer < WATCHDOG_THRESHOLD)
  {
    // Enable H Bridge for IBT2
    digitalWrite(DIR1_RL_ENABLE_PIN, HIGH);

    steerAngleDeviation.value = dataToSend.currentSteerAngle - ReceivedData.steerTargetAngle; // calculate the steering error
    // if (abs(steerAngleError)< steerSettings.lowPWM) steerAngleError = 0;
  }
  else
  {

    digitalWrite(DIR1_RL_ENABLE_PIN, LOW); // IBT2

    pwmDrive = 0; // turn off steering motor
    pulseCount = 0;
  }             // end of timed loop
  motorDrive(); // out to motors the pwm value

} // end of main loop

void calcSteerAngle()
{
  steeringPosition = sensors.wheelAngleSensor.getValue();
  steeringPosition = (steeringPosition << 2);              // bit shift by 2 so 0 to 13610 is 0 to 5v
  dataToSend.helloSteerPosition = steeringPosition - 6800; //??
  // DETERMINE ACTUAL STEERING POSITION

  // convert position to steer angle. 32 counts per degree of steer pot position in my case
  //   ***** make sure that negative steer angle makes a left turn and positive value is a right turn *****
  if (steerConfig.InvertWAS)
  {
    steeringPosition = (steeringPosition - 6805 - steerSettings.wasOffset); // 1/2 of full scale
    dataToSend.currentSteerAngle = (float)(steeringPosition) / -steerSettings.steerSensorCounts;
  }
  else
  {
    steeringPosition = (steeringPosition - 6805 + steerSettings.wasOffset); // 1/2 of full scale
    dataToSend.currentSteerAngle = (float)(steeringPosition) / steerSettings.steerSensorCounts;
  }

  // Ackerman fix
  if (dataToSend.currentSteerAngle < 0)
    dataToSend.currentSteerAngle *= steerSettings.AckermanFix;
}

void calcSteeringPID(void)
{
  // Proportional only
  steerAngleDeviation.proportional = steerSettings.Kp * steerAngleDeviation.value;
  pwmDrive = (int16_t)steerAngleDeviation.proportional;

  steerAngleDeviation.absolute = abs(steerAngleDeviation.value);
  int16_t newMax = 0;

  if (steerAngleDeviation.absolute < LOW_HIGH_DEGREES)
  {
    newMax = (steerAngleDeviation.absolute * ReceivedData.highLowPerDeg) + steerSettings.lowPWM;
  }
  else
    newMax = steerSettings.highPWM;

  // add min throttle factor so no delay from motor resistance.
  if (pwmDrive < 0)
    pwmDrive -= steerSettings.minPWM;
  else if (pwmDrive > 0)
    pwmDrive += steerSettings.minPWM;

  // Serial.print(newMax); //The actual steering angle in degrees
  // Serial.print(",");

  // limit the pwm drive
  if (pwmDrive > newMax)
    pwmDrive = newMax;
  if (pwmDrive < -newMax)
    pwmDrive = -newMax;

  if (steerConfig.MotorDriveDirection)
    pwmDrive *= -1;

  if (steerConfig.IsDanfoss)
  {
    // Danfoss: PWM 25% On = Left Position max  (below Valve=Center)
    // Danfoss: PWM 50% On = Center Position
    // Danfoss: PWM 75% On = Right Position max (above Valve=Center)
    pwmDrive = (constrain(pwmDrive, -250, 250));

    // Calculations below make sure pwmDrive values are between 65 and 190
    // This means they are always positive, so in motorDrive, no need to check for
    // steerConfig.isDanfoss anymore
    pwmDrive = pwmDrive >> 2; // Divide by 4
    pwmDrive += 128;          // add Center Pos.

    // pwmDrive now lies in the range [65 ... 190], which would be great for an ideal opamp
    // However the TLC081IP is not ideal. Approximating from fig 4, 5 TI datasheet, @Vdd=12v, T=@40Celcius, 0 current
    // Voh=11.08 volts, Vol=0.185v
    // (11.08/12)*255=235.45
    // (0.185/12)*255=3.93
    // output now lies in the range [67 ... 205], the center position is now 136
    // pwmDrive = (map(pwmDrive, 4, 235, 0, 255));
  }
}

// #########################################################################################

void motorDrive(void)
{

  // Used with Cytron MD30C Driver
  // Steering Motor
  // Dir + PWM Signal
  if (steerConfig.CytronDriver)
  {
    // Cytron MD30C Driver Dir + PWM Signal
    if (pwmDrive >= 0)
    {
      digitalWrite(DIR1_RL_ENABLE_PIN, HIGH);
      // write out the 0 to 255 value
      analogWrite(PWM1_LPWM, pwmDrive);
    }
    else
    {
      digitalWrite(DIR1_RL_ENABLE_PIN, LOW);
      // write out the 0 to 255 value
      analogWrite(PWM1_LPWM, -pwmDrive);
    }
  }
  else
  {
    // IBT 2 Driver Dir1 connected to BOTH enables
    // PWM Left + PWM Right Signal

    if (pwmDrive > 0)
    {
      analogWrite(PWM2_RPWM, 0); // Turn off before other one on
      analogWrite(PWM1_LPWM, pwmDrive);
    }
    else
    {
      analogWrite(PWM1_LPWM, 0); // Turn off before other one on
      analogWrite(PWM2_RPWM, -pwmDrive);
    }
  }
  pwmDisplay = abs(pwmDrive);
}
