// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef MAVLINK_PROTOCOL_H_
#define MAVLINK_PROTOCOL_H_

#include <Arduino.h>

class Mavlink_protocol {
public:
    typedef uint16_t Packet_len_size_t;
    static constexpr Packet_len_size_t MAX_FRAME_LENGTH = 263;

    typedef struct {
        uint32_t time_since_boot;   // milliseconds
        int32_t lat;                // 100nanodegrees
        int32_t lon;                // 100nanodegrees
        int32_t alt;                // millimeters
        int32_t relative_alt;       // millimeters
        int16_t vx;                 // cm/s
        int16_t vy;                 // cm/s
        int16_t vz;                 // cm/s
        uint16_t hdg;               // centidegrees
    } Message_gps_int;

    virtual ~Mavlink_protocol();

    void
    On_data_byte(uint8_t data);

private:
    typedef enum {
        HEARTBEAT = 0,
        GLOBAL_POSITION_INT = 33,
    } Message_type;

    void
    On_message();

    bool
    Calc_checksum(uint8_t* data, bool save_into_packet = false);

    void
    Slice_to_next_start_byte(Packet_len_size_t start = 0);

    static bool
    Get_extra_crc_data(uint8_t* packet, uint8_t& extra_byte);

    static constexpr uint8_t START_BYTE = 0xfe;
    /** frame footer length (csum, csum, trailer byte). */
    static constexpr Packet_len_size_t FOOTER_LENGTH = 2;
    /** header length (address, type, id, len). */
    static constexpr Packet_len_size_t HEADER_LENGTH = 5;
    /** Minimal length of frame (assembly_addr, type, id, len, csum, csum, stopbyte). */
    static constexpr Packet_len_size_t MIN_FRAME_LENGTH = HEADER_LENGTH + FOOTER_LENGTH;

    /** Current protocol state. */
    bool receiver_waiting_start_byte = true;

    uint8_t packet[MAX_FRAME_LENGTH];
    Packet_len_size_t packet_len = 0;
};

#endif /* MAVLINK_PROTOCOL_H_ */
