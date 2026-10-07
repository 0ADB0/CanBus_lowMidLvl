#include <string.h>
#include "can_validator.h"
#include "can_codec.h"

void can_validator_init(CanValidatorState *state){
    if(state != NULL){
        memset(state,0,sizeof(*state));
    }
}

CanStatus can_validate_frame(CanValidatorState *state,
                            const CanFrame *frame,
                            const CanMessageDef **def_out){

        if(state == NULL ||frame == NULL || def_out ==NULL){
            return CAN_ERR_NULL_POINTER;
        }
        
        //Don't check the content if the ID is unknown.
        const CanMessageDef *def = can_find_message(frame->id);
        if(def == NULL){
            return CAN_ERR_UNKNOWN_ID;
        }

        if (frame->dlc != def->dlc){
            return CAN_ERR_BAD_DLC;
        }

        uint8_t expected_cs = can_checksum(frame->data ,(uint8_t)(def->dlc - 1u));
        if(frame->data[def->dlc - 1u] != expected_cs){
            return CAN_ERR_BAD_CHECKSUM;
        }

        size_t idx = (size_t)(def - CAN_MESSAGE_TABLE);
        if(idx >= CAN_MAX_MESSAGES){
            return CAN_ERR_UNKNOWN_ID;
        }

        uint8_t counter = frame->data[def->dlc - 2u];
        if(counter > 0x0Fu){
            return CAN_ERR_BAD_COUNTER;
        }

        if(state->has_last[idx]){
            uint8_t expected = (uint8_t)((state->last_counter[idx] + 1u) & 0x0Fu);
            if(counter != expected) {
                return CAN_ERR_BAD_COUNTER;
            } 
        }

        state->last_counter[idx] = counter;
        state->has_last[idx] = 1;

        *def_out = def;
        return CAN_OK;
    }