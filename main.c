#include <stdio.h>
#include <stdlb.h>
#include <string.h>

#include "can_types.h"
#include "can_messages.h"
#include "can_codec.h"
#include "can_validator.h"

/*
    Simulation : Normal traffic + Attack scenarios

    Project Structure :
        can_types.* Common types and error codes
        can_messages.* Message defination table
        can_codec.* Physical value <-> Raw byte conversion
        can_validator.* Verifying the incoming frame     
*/
//Prints the frame as hex
static void print_frame(const CanFrame *f){
    printf(" ID=0x%03X DLC=%u DATA=",(unsigned)f->id, (unsigned)f->dlc);

    for(uint8_t = 0;i < f->dlc && i < CAN_MAX_DLC;i++){
        printf("%02X", f->data[i]);
    }
    printf("\n");
}

//Processes a message like a "recipient ECU"
static void receive(CanValidatorState *state,const CanFrame *frame,const char *label){
    printf("[%s]\n",label);
    print_frame(frame);
    
    const CanMessageDef *def = NULL;
    CanStatus st = can_validate_frame(state,frame,&def);
    if(st != CAN_OK){
        printf(" -> REJECTED: %s\n\n",can_status_str(st));
        return;
    }

    double value = 0.0;
    st = can_decode(def,frame,&value);
    if(st!=CAN_OK){
        printf(" ->REJECTED(decode):%s\n\n,can_status_str(st)");
        return;
    }

    printf(" -> ACCEPT: %s = %.2f %s\n\n",def->name,value,def->unit);
}

/Establishes a legitimate framework for the test; if it fails the program stops.
static CanFrame build(uint32_t id, double value,uint8_t counter){
    CanFrame f;
    memset(&f,0,sizeof(f));
    const CanMessageDef *def = can_find_message(id);
    if(def == NULL || can_encode(def,value,counter,&f)!=CAN_OK){
        fprintf(stderr,"build(): Frame NOT Created!\n");
        exit(EXIT_FAILURE);
    }
    return f;
}

int main(void){
    CanValidatorState state;
    can_validator_init(&state);

    printf("---NORMAL TRAFFIC---\n\n");
    CanFrame speed0 = build(CAN_ID_VEHICLE_SPEED,120.5,0);
    CanFrame rpm0 = build(CAN_ID_ENGINE_RPM,2500.0,0);
    CanFrame brake0 = build(CAN_ID_BRAKE_STATUS,30.0,0);
    CanFrame speed1 = build(CAN_ID_VEHICLE_SPEED,121.00,1);
    receive(&state,&speed0,"Speed, Odometer 0");
    receive(&state,&rpm0,"RPM, Odometer 0");
    receive(&state,&brake0,"Brake, Odometer 0");
    receive(&state,&speed1,"Speed, Odometer 1");

    printf("---ATTACK SCENARIO---\n\n");

    //ID that's not in the table
    CanFrame unknown = build(CAN_ID_VEHICLE_SPEED,80.0,2);
    unknown.id = 0x7FF;
    receive(&state,&unknown,"Attack: Unknown ID!");

    //False lenght
    CanFrame bad_dlc = build(CAN_ID_VEHICLE_SPEED,80.0,2);
    bad_dlc.dlc = 3;
    receive(&state,&bad_dlc,"Attack: Wrong DLC!");

    //Tampered data
    CanFrame tampered = build(CAN_ID_VEHICLE_SPEED,80.0,2);
    tampered.data[0] ^= 0x40;
    receive(&state,&tampered,"Attack: Tampered data!");

    //Replay: sends the same frame that's already accepted 
    receive(&state, &speed1 ,"Attack: replay");

    //True checksum but absurd data
    CanFrame absurd;
    memset(&absurd,0,sizeof(absurd));
    absurd.id = CAN_ID_VEHICLE_SPEED;
    absurd.dlc = 4;
    absurd.data[0] = 0xc3; /* 0xC350 = 50000 -> 500.00 km/h */
    absurd.data[1] = 0x50;
    absurd.data[2] = 2;
    absurd.data[3] = can_checksum(absurd.data,3);
    receive(&state,&absurd,"Attack: Valid checksum absurd data!");

    printf("--- Traffic Becomes Normal ---\n\n");
    CanFrame speed3 = build(CAN_ID_VEHICLE_SPEED,122.25,3);
    receive(&state,&speed3,"Speed Odometer 3");

    return 0;
}
