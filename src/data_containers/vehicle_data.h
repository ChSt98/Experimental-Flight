#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H



/**
 * Enum containing all vehicle modes.
 */
enum VEHICLE_MODE {
    //When this is set, the vehicle turns everything off due to a critical error in a subsystem. Should only be resetable from power reset.
    MODE_ERROR,
    //When this is set, the vehicle turns everything off. Should be resetable from vehicle.
    MODE_FAILSAFE,
    //Same as failsafe but indicates no error and puts vehicle into safe mode.
    MODE_DISARM,
    //Vehicle prepares for flight. Move actuators into position.
    MODE_PREPARE,
    //When this is set, the vehicle turns everything on and follows flight commands.
    MODE_ARM
};



/**
 * Struct containing all vehicle data.
 */
struct VehicleData {

    VEHICLE_MODE vehicleMode = VEHICLE_MODE::MODE_DISARM;

};




#endif