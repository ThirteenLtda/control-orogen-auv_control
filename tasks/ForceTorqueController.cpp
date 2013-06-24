/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "ForceTorqueController.hpp"

using namespace auv_control;

ForceTorqueController::ForceTorqueController(std::string const& name, TaskCore::TaskState initial_state)
    : ForceTorqueControllerBase(name, initial_state)
{
}

ForceTorqueController::ForceTorqueController(std::string const& name, RTT::ExecutionEngine* engine, TaskCore::TaskState initial_state)
    : ForceTorqueControllerBase(name, engine, initial_state)
{
}

ForceTorqueController::~ForceTorqueController()
{
}

bool ForceTorqueController::startHook()
{
    ForceTorqueControllerBase::startHook();

    //take the calibration-matrix from the calibration property
    calibration = _calibration.get();
    return true;
}
void ForceTorqueController::updateHook()
{
    ForceTorqueControllerBase::updateHook();

    base::Vector6d input_vector;
    base::VectorXd output_vector;
    base::LinearAngular6DCommand cmd;
    base::actuators::Command actuators_command;
    base::samples::RigidBodyState pose_sample;
    double roll;
    double pitch;
    base::Quaterniond rotation;
    //if ther are datas on the Input port, make one vector with force and torque
    if(_cascade.read(cmd) != RTT::NoData){ 
        if(_pose_sample.read(pose_sample) != RTT::NoData){
            //rotate the linear Vector, to get better Values 
            roll = base::getRoll(pose_sample.orientation);
            pitch = base::getPitch(pose_sample.orientation);
            rotation = base::Quaterniond(Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY())) * base::Quaterniond(Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX()));

            for(int i=0; i<3 ; i++){
            	if (base::isUnset(cmd.linear(i)))
            		cmd.linear(i) = 0;
            }

            cmd.linear = rotation.conjugate() * cmd.linear;
        }

        input_vector(0) = cmd.linear(0);
        input_vector(1) = cmd.linear(1);
        input_vector(2) = cmd.linear(2);
        input_vector(3) = cmd.angular(0);
        input_vector(4) = cmd.angular(1);
        input_vector(5) = cmd.angular(2);
        
        //check for unset values and set it on 0, else the following multiplication with
        //the  matrix would not work.
        for(int i = 0; i < 6; i++){
            if(base::isUnset(input_vector(i))){
                input_vector(i) = 0;
            }
        }

        output_vector = (input_vector.transpose() * calibration).transpose();
        std::vector<double> cut_off;
        cut_off = _cut_off.get();
        int cut_off_size = cut_off.size();
        cut_off.resize(output_vector.size());
        for(int i = cut_off_size; i < output_vector.size(); i++){
            cut_off[i] = 1;
        }

        //Cut values at 1 (or -1) if they are over 1 (or under -1). That shault not happend if the calibration
        //matrix is right.
        for(int i = 0; i < output_vector.size(); i++){
            if(output_vector(i) > cut_off[i]){
                output_vector(i) = cut_off[i];
            } else if(output_vector(i) < -cut_off[i]){
                output_vector(i) = -cut_off[i];
            }
        }

        //Resize the actuatos command on the Size of the output_vector. The size of the output_vetor are
        //the same like the calibration-matrix have collumns.
        actuators_command.resize(output_vector.size());
        for(int i = 0; i < output_vector.size(); i++){
            actuators_command.mode.at(i) = base::actuators::DM_PWM;
            actuators_command.target.at(i) = output_vector(i);
        }
        
        //write the command on the output port
        _motor_command.write(actuators_command);
    }
    return;
}

