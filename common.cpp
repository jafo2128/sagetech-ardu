// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "common.h"

void
Set_le_i16(long val, uint8_t* data)
{
    *(data + 0) = ((val >> 0) & 0xff);
    *(data + 1) = ((val >> 8) & 0xff);
}

void
Set_le_i32(long val, uint8_t* data)
{
    *(data + 0) = ((val >>  0) & 0xff);
    *(data + 1) = ((val >>  8) & 0xff);
    *(data + 2) = ((val >> 16) & 0xff);
    *(data + 3) = ((val >> 24) & 0xff);
}

long
Get_le_i16(uint8_t* data)
{
    return  (static_cast<long>(*(data + 0)) <<  0) +
            (static_cast<long>(*(data + 1)) <<  8);
}

long
Get_le_i32(uint8_t* data)
{
    return  (static_cast<long>(*(data + 0)) <<  0) +
            (static_cast<long>(*(data + 1)) <<  8) +
            (static_cast<long>(*(data + 2)) << 16) +
            (static_cast<long>(*(data + 3)) << 24);
}

unsigned long
Get_le_u16(uint8_t* data)
{
    return  (static_cast<unsigned long>(*(data + 0)) <<  0) +
            (static_cast<unsigned long>(*(data + 1)) <<  8);
}

unsigned long
Get_le_u32(uint8_t* data)
{
    return  (static_cast<unsigned long>(*(data + 0)) <<  0) +
            (static_cast<unsigned long>(*(data + 1)) <<  8) +
            (static_cast<unsigned long>(*(data + 2)) << 16) +
            (static_cast<unsigned long>(*(data + 3)) << 24);
}

void
Dump_data(uint8_t* data, size_t len, HardwareSerial& port)
{
    for (auto i=0; i< len; i++) {
        port.print(" ");
        if (data[i] < 32) {
            port.print("~");
        } else {
            port.write(data[i]);
        }
    }
    port.println();
    for (auto i=0; i< len; i++) {
        if (data[i] < 16) {
            port.print(" ");
        }
        port.print(data[i], 16);
    }
    port.println();
}
