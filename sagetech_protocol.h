// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef SAGETECH_PROTOCOL_H_
#define SAGETECH_PROTOCOL_H_
#include "Arduino.h"

class Sagetech_protocol {
public:
    typedef uint8_t Packet_len_size_t;

    enum class Message_type: uint8_t {
        INVALID                 = 0x00,
        INSTALLATION            = 0x01,
        PREFLIGHT_DATA          = 0x02,
        OPERATING               = 0x03,
        GPS_DATA                = 0x04,
        DATA_REQUEST            = 0x05,
        ACKNOWLEDGE             = 0x80,
        INSTALLATION_RESPONSE   = 0x81,
        PREFLIGHT_DATA_RESPONSE = 0x82,
        STATUS_RESPONSE         = 0x83
    };

    enum class Transponder_mode: uint8_t {
        OFF     = 0x00,
        STANDBY = 0x01,
        ON      = 0x02,
        ALT     = 0x03
    };

    enum class Transponder_type: uint8_t {
        MODE_C              = 0x00,
        MODE_S_ADSB_OUT     = 0x01,
        MODE_S_ADSB_IN_OUT  = 0x02
    };

    enum class Power_up_mode: uint8_t {
        OFF_SQUAWK_1200 = 0x00,
        USE_NVRAM_MODE  = 0x01
    };

    typedef struct {
        Message_type acked_type;
        uint8_t acked_id;
        bool transponder_error;
        bool altitude_source_external;
        bool gps_error;
        bool icao_error;
        bool temperature_error;
        bool extended_squitter_error;
        Transponder_mode transponder_mode;
        int pressure_altitude;
        Transponder_type transponder_type;
        Power_up_mode power_up_mode;
        bool ident_active;
        int squawk_code;
    } Acknowledge_message;

    typedef struct {
        int32_t lat;                  // 100nanodegrees
        int32_t lon;                  // 100nanodegrees
        int32_t alt;                  // millimeters
        int32_t speed_over_ground;    // centimeters/second
        int32_t course_over_ground;   // centidegrees from North
    } Gps_message;

    typedef struct {
        long squawk_code;
        long pressure_altitude;
        Transponder_mode transponder_mode;
        Power_up_mode power_up_mode;
        bool start_ident;
    } Operating_message;

    static constexpr Packet_len_size_t MAX_FRAME_LENGTH = 100;
    static constexpr uint32_t ALTITUDE_USE_BUILT_IN_SENSOR = 0x8000;

    virtual
    ~Sagetech_protocol();

    void On_data_byte(uint8_t data);

    // assumes buffer has enough space.
    Packet_len_size_t
    Create_gps_message(Gps_message msg, uint8_t* buffer);

    // assumes buffer has enough space.
    Packet_len_size_t
    Create_operating_message(Operating_message msg, uint8_t* buffer);

    // assumes buffer has enough space.
    Packet_len_size_t
    Create_preflight_message(const char* flight_id, uint8_t* buffer);

    // Convert coordinates from 100nanodegrees to Sagetech ascii format.
    static void Gps_int_to_gprmc(long v, uint8_t *b);

    // Convert speed from centimeters/s to Sagetech ascii format.
    static void Cms_to_knots(long v, uint8_t *b);

    // Convert course over ground from centidegrees to Sagetech ascii format.
    static void Cog(long v, uint8_t *b);

private:

    void
    On_message();

    Message_type
    Get_message_type() {return static_cast<Message_type>(incoming_packet[1]);};

    void
    Calc_checksum(const uint8_t* data, Packet_len_size_t len, uint8_t& fletcher, uint8_t& additive);

    void
    Slice_to_next_start_byte(Packet_len_size_t start = 0);

    uint8_t
    Get_next_id() {return next_message_id++;};

    Packet_len_size_t
    Build_frame(Message_type type, uint8_t* frame);

    static constexpr uint8_t START_BYTE = 0xa5;
    static constexpr uint8_t STOP_BYTE  = 0x5a;
    static constexpr uint8_t ASSEMBLY_ADDRESS_TRANSPONDER = 0x01;
    /** frame footer length (csum, csum, trailer byte). */
    static constexpr Packet_len_size_t FOOTER_LENGTH = 3;
    /** header length (address, type, id, len). */
    static constexpr Packet_len_size_t HEADER_LENGTH = 5;
    /** Minimal length of frame (assembly_addr, type, id, len, csum, csum, stopbyte). */
    static constexpr Packet_len_size_t MIN_FRAME_LENGTH = HEADER_LENGTH + FOOTER_LENGTH;

    /** Current protocol state. */
    bool receiver_waiting_start_byte = true;

    uint8_t incoming_packet[MAX_FRAME_LENGTH];
    Packet_len_size_t incoming_packet_len = 0;

    uint8_t next_message_id = 0;
};

#endif /* SAGETECH_PROTOCOL_H_ */
