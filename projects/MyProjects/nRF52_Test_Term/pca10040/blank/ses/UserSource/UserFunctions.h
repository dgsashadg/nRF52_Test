#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint32_t write(uint8_t *bufr, uint32_t size);             // put data to server
uint32_t read(uint8_t *bufr);                             // get data from server

uint32_t ParseTerminalCMD(uint8_t *bufr);                 // Check received request