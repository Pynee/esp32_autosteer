#include "AutosteerHandler.h"

AutosteerHandler::AutosteerHandler() : pid(&(dataToSend.currentSteerAngle),
                                           &(receivedData.steerTargetAngle),
                                           &(pidOutput),
                                           -255,
                                           255,
                                           (double)steerSettings.Kp,
                                           (double)steerSettings.Ki,
                                           (double)steerSettings.Kd)
{
}

void AutosteerHandler::startTaskImpl(void *_this)
{
    ((AutosteerHandler *)_this)->autosteerTask(_this);
}

void AutosteerHandler::init()
{
    ledcSetup(0, PWM_Frequency, 8);
    ledcAttachPin(PWM1_LPWM, 0);
    ledcWrite(0, 0);

    ledcSetup(1, PWM_Frequency, 8);
    ledcAttachPin(PWM2_RPWM, 1);
    ledcWrite(1, 0);

    pinMode(DIR1_RL_ENABLE_PIN, OUTPUT);
    gpio_set_direction(gpio_num_t(DIR1_RL_ENABLE_PIN), GPIO_MODE_OUTPUT);

    xTaskCreatePinnedToCore(startTaskImpl, "autosteerTask", 4096, this, 3, NULL, 1);
}

void AutosteerHandler::autosteerTask(void *z)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    pid.setTimeStep(LOOP_TIME);

    for (;;)
    {

        // check for timeout and data from AgOpenGPS
        if (!state.guidanceStatus || millis() > state.lastAOGUpdate + AOGTIMEOUT || hardwareSwitches.steerSwitch.getState() || receivedData.gpsSpeed < 0.1)
        {
            // Serial.println("Disabled autosteer");
            state.autoSteerEnabled = false;

            if (steerConfig.IsDanfoss)
            {
                ledcWrite(0, 128);
                ledcWrite(1, 0);
            }
            else
            {
                ledcWrite(0, 0);
                ledcWrite(1, 0);
            }
            gpio_set_level(gpio_num_t(DIR1_RL_ENABLE_PIN), 0);
        }
        else
        {
            state.autoSteerEnabled = true;
            pid.setGains(
                (double)steerSettings.Kp,
                (double)steerSettings.Ki,
                (double)steerSettings.Kd);

            // if (steerConfig.steeringPidAutoBangOnFactor)
            // {
            // pid.setBangBang(((double)0xFF / steerSettings.Kp) *
            //                    steerConfig.steeringPidAutoBangOnFactor,
            //                steerConfig.steeringPidBangOff);
            // }
            // else
            //{
            //    pid.setBangBang(steerConfig.steeringPidBangOn, steerConfig.steeringPidBangOff);
            //}

            // here comes the magic: executing the PID loop
            // the values are given by pointers, so the AutoPID gets them automaticaly
            // pid.run();

            double motorOutput = steerConfig.InvertWAS ? pidOutput : -pidOutput;

            if (steerConfig.IsDanfoss)
            {
                // go from 25% on: max left, 50% on: center, 75% on: right max
                // motorOutput = constrain(motorOutput, -250.0, 250.0);// Constrain output max/min values for some reason?
                map(motorOutput, -255.0, 255.0, 65.0, 190.0); // map (-250;250) range to (65;190);
            }
            else
            {
                // Constrains the PWM value so it doesn't go below min or over max
                if (motorOutput < 0)
                {
                    motorOutput = min(motorOutput, (double)-steerSettings.minPWM);
                }
                else
                {
                    motorOutput = max(motorOutput, (double)steerSettings.minPWM);
                }
                motorOutput = constrain(motorOutput, (double)-steerSettings.highPWM, (double)steerSettings.highPWM);
            }

            if (steerConfig.CytronDriver) // If Cytron
            {
                if (motorOutput < 0)
                {
                    motorOutput = -motorOutput;
                    gpio_set_level(gpio_num_t(DIR1_RL_ENABLE_PIN), 1);
                }
                else
                {
                    gpio_set_level(gpio_num_t(DIR1_RL_ENABLE_PIN), 0);
                }
                ledcWrite(0, motorOutput);
            }
            else // we can use same outputs for IBT2 and hydraulic because hydraulic is always positive
            {

                if (motorOutput < 0)
                {
                    ledcWrite(0, 0);
                    ledcWrite(1, -motorOutput);
                }
                else
                {
                    ledcWrite(1, 0);
                    ledcWrite(0, motorOutput);
                }
                gpio_set_level(gpio_num_t(DIR1_RL_ENABLE_PIN), 1);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, LOOP_TIME / portTICK_PERIOD_MS);
    }
}
