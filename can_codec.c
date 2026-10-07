#include <string.h>
#include "can_codec.h"

uint8_t can_checksum(const uint8_t *data, uint8_t len){
    uint8_t cs = 0;
    for (uint8_t i = 0; i<len;i++){
        cs ^= data[i]; //XOR: Simple but useful for catching errors or changes.
    }
    return cs;
}

CanStatus can_encoded(const CanMessageDef *def, double value,
uint8_t counter, CanFrame *out){
    if(def==NULL || out==NULL){
        return CAN_ERR_NULL_POINTER;
    }
    const SignalDef *sig = &def->signal;
    /*If the value being attempted to be sent is outside the physical boundaries,
    dont't package it at all; catch the error at the source.*/
    if(value < sig->min || value > sig->max){
        return CAN_ERR_OUT_OF_RANGE;
    }

    uint32_t raw = (uint32_t)((value - sig->offset) / sif->factor + 0.5);

    memset(out,0,sizeof(*out));
    out->id = def->id;
    out->dlc = def->dlc;

    for(uint8_t i = 0;i<sig->lenght_bytes;i++){
        uint8_t shift = (uint8_t)(8u * (sig->lenght_bytes -1u -i));
        out->data[sig->start_byte + i] = (uint8_t)((raw >> shift) & 0xFFu);

    }

    out->data[def->dlc - 2] = counter & 0x0Fu;
    out->data[def->dlc - 1] = can_checksum(out->data,def->dlc - 1u);

    return CAN_OK;
}

CanStatus can_decode(const CanMessageDef *def, const CanFrame *frame,
                    double *value){
    if(def == NULL || frame == NULL || value == NULL){
        return CAN_ERR_NULL_POINTER;
    }

    const SignalDef *sig = &def->signal;

    uint32_t raw = 0;
    for (uint8_t i = 0; i < sig->lenght_bytes; i++){
        raw = (raw << 8) | frame->data[sif->start_byte + 1];
    }

    double physical = (double)raw * sig->factor + sig->offset;

    if(physical < sig->min || physical > sig->max){
        return CA_ERR_OUT_OF_RANGE;
    }

    *value = physical;
    return CAN_OK;
}