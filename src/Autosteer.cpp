#include "AutoSteer.h"
uint8_t watchdogTimer = WATCHDOG_FORCE_VALUE;
double pidOutput = 0;
AutoPID pid(&(dataToSend.currentSteerAngle),
            &(receivedData.steerTargetAngle),
            &(dataToSend.currentSteerAngle),
            -255.0,
            255.0,
            (double)steerSettings.Kp,
            (double)steerSettings.Ki,
            (double)steerSettings.Kd);

void initAutosteer()
{

    ledcSetup(0, PWM_Frequency, 8);
    ledcAttachPin(PWM1_LPWM, 0);
    ledcWrite(0, 0);

    ledcSetup(1, PWM_Frequency, 8);
    ledcAttachPin(PWM2_RPWM, 1);
    ledcWrite(0, 0);

    pinMode(DIR1_RL_ENABLE_PIN, OUTPUT);

    xTaskCreate(autosteerWorker, "autosteerWorker", 3096, NULL, 3, NULL);
}

void autosteerWorker(void *z)
{
    constexpr TickType_t xFrequency = 10;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    pid.setTimeStep(LOOP_TIME);

    // auto queue = xQueueCreate(16, sizeof(CborPackageBase *));
    // cborQueueSelector.addQueue(steerConfig.qogChannelIdAutosteerEnable, queue);
    // cborQueueSelector.addQueue(steerConfig.qogChannelIdSetpointSteerAngle, queue);

    for (;;)
    {
        time_t timeoutPoint = millis() - Timeout;

        // check for timeout and data from AgOpenGPS
        if (watchdogTimer < WATCHDOG_THRESHOLD && state.autoSteerEnabled)
        {
            // steerSetpoints.enabled = false;
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

            digitalWrite(DIR1_RL_ENABLE_PIN, LOW);
        }
        else
        {
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
            pid.run();

            if (pidOutput)
            {
                double motorOutput = steerConfig.InvertWAS ? pidOutput : -pidOutput;

                if (motorOutput < 0 && motorOutput > -steerSettings.minPWM)
                {
                    motorOutput = -steerSettings.minPWM;
                }

                if (motorOutput > 0 && motorOutput < steerSettings.minPWM)
                {
                    motorOutput = steerSettings.minPWM;
                }

                else if (steerConfig.CytronDriver) // If Cytron
                {
                    if (motorOutput >= 0)
                    {
                        ledcWrite(1, 255);
                    }
                    else
                    {
                        ledcWrite(0, 255);
                        motorOutput = -motorOutput;
                    }

                    ledcWrite(0, motorOutput);
                    digitalWrite(DIR1_RL_ENABLE_PIN, HIGH);
                }
                else if (steerConfig.IsDanfoss) // If Danfoss hydraylic steering
                {
                    // go from 25% on: max left, 50% on: center, 75% on: right max
                    if (motorOutput > 250)
                    {
                        motorOutput = 250;
                    }

                    if (motorOutput < -250)
                    {
                        motorOutput = -250;
                    }

                    motorOutput /= 4;
                    motorOutput += 128;
                    ledcWrite(0, motorOutput);
                }
                else // default is IBT2
                {
                    if (motorOutput >= 0)
                    {
                        ledcWrite(0, motorOutput);
                        ledcWrite(1, 0);
                    }

                    if (motorOutput < 0)
                    {
                        ledcWrite(0, 0);
                        ledcWrite(1, -motorOutput);
                    }
                    digitalWrite(DIR1_RL_ENABLE_PIN, HIGH);
                }
            }
            else
            {
                ledcWrite(0, 0);
                ledcWrite(1, 0);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, LOOP_TIME / portTICK_PERIOD_MS);
    }
}
