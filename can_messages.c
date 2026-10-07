#include "can_messages.h"

/*
    The last two bytes of every message follow a common rule:
        data[dlc-2]: alive counter
        data[dlc-1]: checksum
    Thanks to this rule, validator can check every message in the same way. 
*/

const CanMessagesDef CAN_MESSAGE_TABLE[] = {
    {CAN_ID_VEHICLE_SPEED, 4, "VehicleSpeed","km/h",{0,2,0.01,0.0,0.0,300.0}},
    {CAN_ID_ENGINE_RPM, 4, "EngineRpm", "rpm", {0,2,0.25,0.0,0.0,9000.0},}
    {CAN_ID_BRAKE_STATUS, 3, "BrakePressure", "%",{0,1,1.0,0.0,0.0,100.0},}

};

//Instead of manually setting the table size, we have the compiler to calculate it.
const size_t CAN_MESSAGE_COUNT = sizeof(CAN_MESSAGE_TABLE) / sizeof(CAN_MESSAGE_TABLE[0]);

const CanMessageDef *can_find_message(uint32_t id){
    for ( size_t i = 0; i < CAN_MESSAGE_COUNT;i++){
        if(CAN_MESSAGE_TABLE[i].id == id){
            return &CAN_MESSAGE_TABLE[i];
        }
    }
    return  NULL;
}