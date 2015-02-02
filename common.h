// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef UTIL_H_
#define UTIL_H_

#include "mavlink_protocol.h"
#include "sagetech_protocol.h"
#include "Arduino.h"

void
Set_le_i16(long val, uint8_t* data);

void
Set_le_i32(long val, uint8_t* data);

long
Get_le_i16(uint8_t* data);

long
Get_le_i32(uint8_t* data);

unsigned long
Get_le_u16(uint8_t* data);

unsigned long
Get_le_u32(uint8_t* data);

void
Dump_data(uint8_t* data, size_t len, HardwareSerial& port);

void
On_mavlink_gps(Mavlink_protocol::Message_gps_int msg);

void
On_sagetech_ack(Sagetech_protocol::Acknowledge_message msg);

#endif /* UTIL_H_ */
