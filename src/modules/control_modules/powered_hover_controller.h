#ifndef HOVER_CONTROLLER_H
#define HOVER_CONTROLLER_H



#include "lib/Simple-Schedule/src/task_autorun_class.h"

#include "modules/module_abstract.h"

#include "data_containers/control_data.h"
#include "data_containers/navigation_data.h"
#include "data_containers/vehicle_data.h"

#include "modules/control_modules/control_interface.h"
#include "modules/navigation_modules/navigation_interface.h"


class HoverController: public Control_Interface, public Module_Abstract, public Task_Abstract {
public:

    /**
     * Creates a module based class and automatically adds it to the scheduler.
     * 
     * @param rate is the rate at which it will be ran at.
     * @param priority is the priority the module will have.
     */
    HoverController(Guidance_Interface* guidanceModule, Navigation_Interface* navigationModule) : Task_Abstract(1000, eTaskPriority_t::eTaskPriority_High, true) {
        controlSetpoint_ = guidanceModule->getControlSetpointPointer();
        navigationData_ = navigationModule->getNavigationDataPointer();
    }

    /**
     * This is where all calculations are done.
     *
     * @param values none.
     * @return none.
     */
    inline void thread();

    /**
     * Sets the control modules guidance module.
     * @param guidanceModule Pointer to module to use.
     */
    inline void setGuidanceModule(Guidance_Interface* guidanceModule) {controlSetpoint_ = guidanceModule->getControlSetpointPointer();}

    /**
     * Sets the control modules navigation module.
     * @param navigationModule Pointer to module to use.
     */
    inline void setGuidanceModule(Navigation_Interface* navigationModule) {navigationData_ = navigationModule->getNavigationDataPointer();}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     *
     * @param values factor.
     * @return none.
     */
    inline void setAngularAccelerationPIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1) {angAccelPF_ = factorP; angAccelIF_ = factorI; angAccelDF_ = factorD; angAccelLimit_ = limit;}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     * 
     * If passThrough is set to true then the output of this controller will be added to controller modules output. Otherwise its output will be added to the next controllers setpoint.
     *
     * @param values factor.
     * @return none.
     */
    inline void setAngularVelocityPIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1, const bool &passThrough = false) {angVelPF_ = factorP; angVelIF_ = factorI; angVelDF_ = factorD; angVelLimit_ = limit, angVelPassThrough_ = passThrough;}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     * 
     * If passThrough is set to true then the output of this controller will be added to controller modules output. Otherwise its output will be added to the next controllers setpoint.
     *
     * @param values factor.
     * @return none.
     */
    inline void setAttitudePIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1, const bool &passThrough = false) {attitudePF_ = factorP; attitudeIF_ = factorI; attitudeDF_ = factorD; attitudeLimit_ = limit, attitudePassThrough_ = passThrough;}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     *
     * @param values factor.
     * @return none.
     */
    inline void setAccelerationPIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1) {accelPF_ = factorP; accelIF_ = factorI; accelDF_ = factorD; accelLimit_ = limit;}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     * 
     * If passThrough is set to true then the output of this controller will be added to controller modules output. Otherwise its output will be added to the next controllers setpoint.
     *
     * @param values factor.
     * @return none.
     */
    inline void setVelocityPIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1, const bool &passThrough = false) {velocityPF_ = factorP; velocityIF_ = factorI; velocityDF_ = factorD; velocityLimit_ = limit, velocityPassThrough_ = passThrough;}

    /**
     * Sets the control factor.
     * 
     * Limit is the value that if reached, the intergrator will stop integrating and also reduce for anti-windup
     * 
     * If passThrough is set to true then the output of this controller will be added to controller modules output. Otherwise its output will be added to the next controllers setpoint.
     *
     * @param values factor.
     * @return none.
     */
    inline void setPositionPIDFactors(const Vector &factorP = 0, const Vector &factorI = 0, const Vector &factorD = 0, const Vector &limit = 1, const bool &passThrough = false) {positionPF_ = factorP; positionIF_ = factorI; positionDF_ = factorD; positionLimit_ = limit, positionPassThrough_ = passThrough;}

    /**
     * Returns the kinematics the system needs to achieve the desired
     * guidance inputs.
     *
     * @param values none.
     * @return kinematicSetpoint.
     */
    inline DynamicData getDynamicsOutput() {return controlOutput_;};

    /**
     * Returns the a pointer to a struct with the kinematics the system 
     * needs to achieve the desired guidance inputs.
     *
     * @param values none.
     * @return kinematicSetpoint pointer.
     */
    inline DynamicData* getDynamicsOutputPointer() {return &controlOutput_;}

private:

    ControlData* controlSetpoint_;
    NavigationData* navigationData_;

    DynamicData controlOutput_;

    //P factor for angular acceleration 
    Vector angAccelPF_ = 0;
    //I factor for angular acceleration 
    Vector angAccelIF_ = 0;
    //D factor for angular acceleration 
    Vector angAccelDF_ = 0;
    //I limit for angular acceleration 
    Vector angAccelLimit_ = 1;
    //I value for angular acceleration
    Vector angAccelIValue_ = 0; 

    //P factor for angular velocity 
    Vector angVelPF_ = 0;
    //I factor for angular velocity 
    Vector angVelIF_ = 0;
    //D factor for angular velocity 
    Vector angVelDF_ = 0;
    //I limit for angular velocity 
    Vector angVelLimit_ = 1;
    //I value for angular velocity
    Vector angVelIValue_ = 0; 
    //If true, then this controller will directly add its output to the controllers output
    bool angVelPassThrough_ = false; 

    //P factor for attitude 
    Vector attitudePF_ = 0;
    //I factor for attitude 
    Vector attitudeIF_ = 0;
    //D factor for attitude  
    Vector attitudeDF_ = 0;
    //I limit for attitude
    Vector attitudeLimit_ = 1;
    //I value for attitude
    Vector attitudeIValue_ = 0; 
    //If true, then this controller will directly add its output to the controllers output
    bool attitudePassThrough_ = false; 

    //P factor for acceleration 
    Vector accelPF_ = 0;
    //I factor for acceleration 
    Vector accelIF_ = 0;
    //D factor for acceleration  
    Vector accelDF_ = 0;
    //I limit for acceleration
    Vector accelLimit_ = 1;
    //I value for acceleration
    Vector accelIValue_ = 0; 

    //P factor for velocity 
    Vector velocityPF_ = 0;
    //I factor for velocity 
    Vector velocityIF_ = 0;
    //D factor for velocity  
    Vector velocityDF_ = 0;
    //I limit for velocity
    Vector velocityLimit_ = 1;
    //I value for velocity
    Vector velocityIValue_ = 0; 
    //If true, then this controller will directly add its output to the controllers output
    bool velocityPassThrough_ = false; 

    //P factor for position 
    Vector positionPF_ = 0;
    //I factor for position 
    Vector positionIF_ = 0;
    //D factor for position  
    Vector positionDF_ = 0;
    //I limit for position
    Vector positionLimit_ = 1;
    //I value for position
    Vector positionIValue_ = 0; 
    //If true, then this controller will directly add its output to the controllers output
    bool positionPassThrough_ = false; 


    
};





#endif