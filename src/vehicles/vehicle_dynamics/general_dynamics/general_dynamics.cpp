#include "general_dynamics.h"



void GeneralDynamics::sensorFusionThread() {

    if (!interval.isTimeToRun()) return;


    if (IMU::getDeviceStatus() != DeviceStatus::DEVICE_RUNNING) return;


    if (IMU::gyroAvailable()) {
        
        //Get IMU data
        Vector rotationVector;
        uint32_t timestamp;
        IMU::getGyro(&rotationVector, &timestamp);

        //Check if gyro initialised
        if (gyroInitialized) {

            //Predict system state
            float dt = float(timestamp - lastGyroTimestamp)/1000000.0f;
            lastGyroTimestamp = timestamp;

            if (!rotationVector.isZeroVector()) {

                Quaternion rotationQuat = Quaternion(rotationVector, rotationVector.magnitude()*dt);

                _attitude = _attitude*rotationQuat;

            }

        } else {

            //Gyro filter initialisation
            gyroInitialized = true;
            lastGyroTimestamp = timestamp;

        }

    }


    if (IMU::accelAvailable()) {

        //Get IMU data
        Vector accelVector;
        uint32_t timestamp;
        IMU::getAccel(&accelVector, &timestamp);

        //Check if accelerometer initialised
        if (!accelInitialized) {
            
            //Correct state prediction
            float dt = float(timestamp - lastAccelTimestamp)/1000000.0f;
            lastAccelTimestamp = timestamp;

            float beta = 0.1f;

            //Z-Axis correction
            Vector zAxisIs = Vector(0,0,1);
            Vector zAxisSet = (_attitude*accelVector*_attitude.copy().conjugate()).toVector();

            Vector zAxisRotationAxis = zAxisSet.cross(zAxisIs);
            float zAxisRotationAngle = zAxisSet.getAngleTo(zAxisIs);

            Quaternion zAxisCorrectionQuat = Quaternion(zAxisRotationAxis, zAxisRotationAngle*beta*dt);


            //Apply state correction and normalise attitude quaternion 
            _attitude = zAxisCorrectionQuat*_attitude;
            _attitude.normalize(true);

        } else if (gyroInitialized) {
            
            //Accel filter initialisation
            accelInitialized = true;
            lastAccelTimestamp = timestamp;

            //Set Attitude
            Vector zAxisIs = Vector(0,0,1);
            Vector zAxisSet = (_attitude*accelVector*_attitude.copy().conjugate()).toVector();

            Vector zAxisRotationAxis = zAxisSet.cross(zAxisIs);
            float zAxisRotationAngle = zAxisSet.getAngleTo(zAxisIs);

            Quaternion zAxisCorrectionQuat = Quaternion(zAxisRotationAxis, zAxisRotationAngle);

            //Apply state correction and normalise attitude quaternion 
            _attitude = zAxisCorrectionQuat*_attitude;
            _attitude.normalize(true);

        }

    }


    if (IMU::magAvailable()) {

        //Get IMU data
        Vector magVector;
        uint32_t timestamp;
        IMU::getMag(&magVector, &timestamp);

        if (magInitialized) {

            //Correct state prediction
            float dt = float(timestamp - lastMagTimestamp)/1000000.0f;
            lastMagTimestamp = timestamp;

            float gamma = 0.1f;

            //X-Axis correction
            Vector xAxisIs(1,0,0);
            Vector xAxisSet = (_attitude*magVector*_attitude.copy().conjugate()).toVector();
            xAxisSet.z = 0;
            xAxisSet.normalize();

            Vector xAxisRotationAxis = Vector(0,0,1);
            float xAxisRotationAngle = -atan2(xAxisSet.y, xAxisSet.x);

            Quaternion xAxisCorrectionQuat = Quaternion(xAxisRotationAxis, xAxisRotationAngle*gamma*dt);


            //Apply state correction and normalise attitude quaternion 
            _attitude = xAxisCorrectionQuat*_attitude;
            _attitude.normalize(true);

        } else if (accelInitialized) {

            //Magnetometer filter initialisation
            magInitialized = true;
            lastMagTimestamp = timestamp;

            //Set heading
            Vector xAxisIs(1,0,0);
            Vector xAxisSet = (_attitude*magVector*_attitude.copy().conjugate()).toVector();
            xAxisSet.z = 0;
            xAxisSet.normalize();

            Vector xAxisRotationAxis = Vector(0,0,1);
            float xAxisRotationAngle = -atan2(xAxisSet.y, xAxisSet.x);

            Quaternion xAxisCorrectionQuat = Quaternion(xAxisRotationAxis, xAxisRotationAngle);

            //Apply state correction and normalise attitude quaternion 
            _attitude = xAxisCorrectionQuat*_attitude;
            _attitude.normalize(true);

        }

    }

}