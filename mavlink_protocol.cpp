// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "mavlink_protocol.h"
#include "common.h"

Mavlink_protocol::~Mavlink_protocol()
{
}

void
Mavlink_protocol::On_message()
{
    Message_gps_int msg_gps;
    switch (packet[4]) {
    case HEARTBEAT:
        break;
    case GLOBAL_POSITION_INT:
        msg_gps.time_since_boot = Get_le_u32(packet + 5);
        msg_gps.lat = Get_le_i32(packet + 9);
        msg_gps.lon = Get_le_i32(packet + 13);
        msg_gps.alt = Get_le_i32(packet + 17);
        msg_gps.relative_alt = Get_le_i32(packet + 21);
        msg_gps.vx = Get_le_i16(packet + 25);
        msg_gps.vy = Get_le_i16(packet + 27);
        msg_gps.vz = Get_le_i16(packet + 29);
        msg_gps.hdg = Get_le_u16(packet + 31);
        On_mavlink_gps(msg_gps);
        break;
    default:
        return;
    }
}

void
Mavlink_protocol::On_data_byte(uint8_t data_byte)
{
    if (receiver_waiting_start_byte) {
        if (data_byte == START_BYTE) {
            receiver_waiting_start_byte = false;
        }
        return;
    }

    packet[packet_len] = data_byte;
    packet_len++;

    while (packet_len) {

        auto payload_length = packet[0];

        // need the whole packet before processing;
        if (packet_len < HEADER_LENGTH + payload_length + FOOTER_LENGTH) {
            return;
        }

        // packet must fit in our buffer.
        if (packet_len == MAX_FRAME_LENGTH) {
            Slice_to_next_start_byte();
            continue;
        }

        // packet footer must have valid checksum.
        if (Calc_checksum(packet)) {
            On_message();
            Slice_to_next_start_byte(HEADER_LENGTH + payload_length + FOOTER_LENGTH);
        } else {
            Slice_to_next_start_byte();
        }
    }
    receiver_waiting_start_byte = true;
}

void
Mavlink_protocol::Slice_to_next_start_byte(Packet_len_size_t startindex)
{
    Packet_len_size_t sbindex;
    for (sbindex = startindex; sbindex < packet_len; sbindex++) {
        if (packet[sbindex] == START_BYTE) {
            sbindex++;
            packet_len -= sbindex;
            for (Packet_len_size_t i = 0; i < packet_len; i++) {
                packet[i] = packet[sbindex + i];
            }
            return;
        }
    }
    packet_len = 0;
}

bool
Mavlink_protocol::Get_extra_crc_data(uint8_t* packet, uint8_t& extra_data_out)
{
    Packet_len_size_t extra_len = 0, extra_data;
    switch (packet[4]) {
    case HEARTBEAT:
        extra_len = 9;
        extra_data = 50;
        break;
    case GLOBAL_POSITION_INT:
        extra_len = 28;
        extra_data = 104;
        break;
    default:
        return false;
    }
    if (packet[0] != extra_len) {
        return false;
    }
    extra_data_out = extra_data;
    return true;
}

bool
Mavlink_protocol::Calc_checksum(uint8_t* packet, bool save_into_packet)
{
    uint16_t csum = 0xffff;
    Packet_len_size_t len = HEADER_LENGTH + packet[0];
    uint8_t tmp;
    uint8_t* data = packet;

    uint8_t extra_crc_byte;

    if (!Get_extra_crc_data(packet, extra_crc_byte)) {
        return false;
    }

    while (len--) {
        tmp = *(data++) ^ (csum &0xff);
        tmp ^= (tmp<<4);
        csum = (csum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
    }

    tmp = extra_crc_byte ^ (csum &0xff);
    tmp ^= (tmp<<4);
    csum = (csum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);

    if (save_into_packet) {
        *(data++) = csum & 0xff;
        *(data++) = (csum >> 8) & 0xff;
    } else {
        return (*(data++) == (csum & 0xff) && *(data++) == ((csum >> 8) & 0xff));
    }
    return true;
}
