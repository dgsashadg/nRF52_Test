#include "UserFunctions.h"

uint32_t write(uint8_t *bufr, uint32_t size){
    uint32_t i = 0x00000000;
    uint8_t BufrOut;
    bufr  = bufr + 5;
    while(i++ < (size - 5)){
        BufrOut = *bufr;
        bufr++;
    }
    return size;
}

uint32_t read(uint8_t *bufr){
    uint8_t EndDataFlags = 0x00;
    uint32_t timeout = 0x00000000;
    uint32_t DataSize = 0x00000000;
    uint8_t BufrIn = 0x00;
    while((EndDataFlags != 0x07) && (++timeout < 0xFFFF)){
        if(EndDataFlags == 0x00){
            EndDataFlags = (*bufr == 0x0D)? 0x01 : 0x00;
        }
        else if(EndDataFlags == 0x01){
            EndDataFlags = (*bufr == 0x0A)? 0x03 : 0x00;
        }
        else if(EndDataFlags == 0x03){
            EndDataFlags = (*bufr == 0x00)? 0x07 : 0x00;
        }
        if(BufrIn){
            *bufr = BufrIn++;
            DataSize++;
        }
        bufr++;
    }
    return DataSize;
}

uint32_t ParseTerminalCMD(uint8_t *bufr){
    if(*(bufr + 0) == 'h' & *(bufr + 1) == 't' & *(bufr + 2) == 't' & *(bufr + 3) == 'p'){
          if(*(bufr + 5) == 'G' & *(bufr + 6) == 'E' & *(bufr + 7) == 'T'){
              return 0x01;
          }
          else if (*(bufr + 5) == 'P' & *(bufr + 6) == 'O' & *(bufr + 7) == 'S' & *(bufr + 8) == 'T'){
              return 0x02;
          }
          else{
              return 0x00;
          }
    }
    else{
        return 0x00;  
    }
}