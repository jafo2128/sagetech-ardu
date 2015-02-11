// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "sagetech_protocol.h"
#include "common.h"

Sagetech_protocol::~Sagetech_protocol()
{
}

void
Sagetech_protocol::Cms_to_knots(long v, uint8_t *buf)
{
    char tmp[10];
    uint8_t len;
    if (v < 0) {
        v = 0;  // must be positive
    }
    if (v > 51444) {
        v = 51444;  // max supported speed. (~1.5 MACH)
    }
    v = (v * 19438) / 10000;
    long full = v / 100;
    long frac = v - (full * 100);

    buf[0] = '0';
    buf[1] = '0';
    buf[2] = 'z';
    buf[3] = '.';
    buf[4] = '0';
    buf[5] = 'z';

    ltoa(full, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 3 - len, tmp, len);

    ltoa(frac, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 6 - len, tmp, len);
}

void
Sagetech_protocol::Cog(long v, uint8_t *buf)
{
    char tmp[10];
    uint8_t len;
    if (v < 0) {
        v = 0;  // must be positive
    }
    if (v > 36000) {
        v = 0;  // max angle
    }
    long full = v / 100;
    long frac = v - (full * 100);

    buf[0] = '0';
    buf[1] = '0';
    buf[2] = 'z';
    buf[3] = '.';
    buf[4] = '0';
    buf[5] = 'z';
    buf[6] = '0';
    buf[7] = '0';

    ltoa(full, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 3 - len, tmp, len);

    ltoa(frac, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 6 - len, tmp, len);
}

void
Sagetech_protocol::Gps_int_to_gprmc(long v, uint8_t *buf)
{
    char tmp[10];
    uint8_t len;
    if (v < 0) {
        v = -v;
    }
    long deg = v / 10000000;
    long min = (v - (deg * 10000000)) * 6 / 10;
    long dec = (min - ((min / 100000) * 100000));
    min = min / 100000;

    buf[0]  = '0';
    buf[1]  = '0';
    buf[2]  = 'z';
    buf[3]  = '0';
    buf[4]  = 'z';
    buf[5]  = '.';
    buf[6]  = '0';
    buf[7]  = '0';
    buf[8]  = '0';
    buf[9]  = '0';
    buf[10] = 'z';

    ltoa(deg, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 3 - len, tmp, len);

    ltoa(min, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 5 - len, tmp, len);

    ltoa(dec, tmp, 10);
    len = strlen(tmp);
    memcpy(buf + 11 - len, tmp, len);
}

Sagetech_protocol::Packet_len_size_t
Sagetech_protocol::Build_frame(Sagetech_protocol::Message_type type, uint8_t* frame)
{
    Packet_len_size_t payload_len, frame_len;
    switch (type) {
    case Message_type::PREFLIGHT_DATA:
        payload_len = 10;
        break;
    case Message_type::OPERATING:
        payload_len = 8;
        break;
    case Message_type::GPS_DATA:
        payload_len = 52;
        break;
    default:
        break;
    }
    frame_len = HEADER_LENGTH + payload_len + FOOTER_LENGTH;
    frame[0] = START_BYTE;
    frame[1] = ASSEMBLY_ADDRESS_TRANSPONDER;
    frame[2] = static_cast<uint8_t>(type);
    frame[3] = Get_next_id();
    frame[4] = payload_len;

    Calc_checksum(
            frame + 1,
            HEADER_LENGTH - 1 + payload_len,
            frame[frame_len - 3],
            frame[frame_len - 2]);
    frame[frame_len - 1] = STOP_BYTE;

    return frame_len;
}

Sagetech_protocol::Packet_len_size_t
Sagetech_protocol::Create_preflight_message(const char* flight_id, uint8_t* buffer)
{
    uint8_t i;
    if (flight_id) {
        for (i = 0; i < 8, flight_id[i]; i++) {
            buffer[5 + i] = flight_id[i];
        }
        for (; i < 8; i++) {
            buffer[5 + i] = ' ';
        }
    } else {
        for (i = 0; i < 8; i++) {
            buffer[5 + i] = 0;
        }
    }

    buffer[13] = 0;
    buffer[14] = 0;

    return Build_frame(Message_type::PREFLIGHT_DATA, buffer);
}

Sagetech_protocol::Packet_len_size_t
Sagetech_protocol::Create_operating_message(Operating_message m, uint8_t* buffer)
{
    buffer[5] = (m.squawk_code & 0xf00) >> 8;
    buffer[6] = (m.squawk_code & 0xff);
    buffer[7] = (m.pressure_altitude & 0xff00) >> 8;
    buffer[8] = (m.pressure_altitude & 0xff);
    buffer[9] = static_cast<uint8_t>(m.transponder_mode) | (static_cast<uint8_t>(m.power_up_mode) << 3);
    if (m.start_ident) {
        buffer[9] |= 4;
    }
    buffer[10] = 0;
    buffer[11] = 0;
    buffer[12] = 0;

    return Build_frame(Message_type::OPERATING, buffer);
}

Sagetech_protocol::Packet_len_size_t
Sagetech_protocol::Create_gps_message(Gps_message m, uint8_t* buffer)
{
    // put latitude first because it has 2 digit integer part
    Gps_int_to_gprmc(m.lat, buffer + 15);
    // this overwrites the first digit (0) of lat.
    Gps_int_to_gprmc(m.lon, buffer + 5);
    Cms_to_knots(m.speed_over_ground, buffer + 26);
    Cog(m.course_over_ground, buffer + 32);
    buffer[40] = 0;
    if (m.lat >= 0) {
        buffer[40] |= 0x01; // North hemisphere
    }
    if (m.lon >= 0) {
        buffer[40] |= 0x02; // East hemisphere
    }

    // We do not know the gps time.
    // Docs say there it should be filled with spaces (0x20) but
    // that does not work. Transponder responds with error.
    // So, we fill with invalid data: 99:99:99.999
    for (int i = 41; i < 51; i++) {
        buffer[i] = 0x39;
    }
    buffer[47] = 0x2e;

    // Zero last 6 bytes (reserved)
    for (int i = 51; i < 57; i++) {
        buffer[i] = 0x00;
    }

    return Build_frame(Message_type::GPS_DATA, buffer);
}

void
Sagetech_protocol::On_message()
{
    // We need to process only ack message for now.
    if (Get_message_type() == Message_type::ACKNOWLEDGE) {
        Acknowledge_message m;
        if (incoming_packet[3] != 9) {
            return;
        }
        m.acked_type = static_cast<Message_type>(*(incoming_packet + 4));
        m.acked_id = *(incoming_packet + 5);
        m.transponder_error = *(incoming_packet + 6) & 0x01;
        m.altitude_source_external = *(incoming_packet + 6) & 0x02;
        m.gps_error = *(incoming_packet + 6) & 0x04;
        m.icao_error = *(incoming_packet + 6) & 0x08;
        m.temperature_error = *(incoming_packet + 6) & 0x10;
        m.extended_squitter_error = *(incoming_packet + 6) & 0x20;
        m.transponder_mode = static_cast<Transponder_mode>(*(incoming_packet + 6) >> 6);
        m.pressure_altitude =
                static_cast<int>(*(incoming_packet + 7)) * 65536 +
                static_cast<int>(*(incoming_packet + 8)) * 256 +
                static_cast<int>(*(incoming_packet + 9));
        m.transponder_type = static_cast<Transponder_type>(*(incoming_packet + 10));
        m.power_up_mode = static_cast<Power_up_mode>((*(incoming_packet + 11) & 0x20) >> 5);
        m.ident_active = (*(incoming_packet + 11) & 0x10);
        m.squawk_code = (*(incoming_packet + 11) & 0x07) * 265 + *(incoming_packet + 12);
        On_sagetech_ack(m);
    }
}

void
Sagetech_protocol::On_data_byte(uint8_t data_byte)
{
    if (receiver_waiting_start_byte) {
        if (data_byte == START_BYTE) {
            receiver_waiting_start_byte = false;
        }
        return;
    }

    incoming_packet[incoming_packet_len] = data_byte;
    incoming_packet_len++;

    while (incoming_packet_len) {

        // need at least minimal packet before processing;
        if (incoming_packet_len < MIN_FRAME_LENGTH) {
            return;
        }

        // packet must fit in our buffer and must have 0x01 as first byte.
        if (    incoming_packet_len > MAX_FRAME_LENGTH - MIN_FRAME_LENGTH
            ||  incoming_packet[0] != ASSEMBLY_ADDRESS_TRANSPONDER) {
            Slice_to_next_start_byte();
            continue;
        }

        auto payload_length = incoming_packet[3];

        // need the whole packet before processing;
        if (incoming_packet_len < HEADER_LENGTH - 1 + payload_length + FOOTER_LENGTH) {
            return;
        }

        uint8_t cs1, cs2;
        Calc_checksum(incoming_packet, payload_length + 4, cs1, cs2);

        // packet footer must have valid checksum and stop byte.
        if (    incoming_packet[payload_length + 4] == cs1
            &&  incoming_packet[payload_length + 5] == cs2
            &&  incoming_packet[payload_length + 6] == STOP_BYTE) {
            On_message();
            Slice_to_next_start_byte(HEADER_LENGTH - 1 + payload_length + FOOTER_LENGTH);
        } else {
            Slice_to_next_start_byte();
        }
    }
    receiver_waiting_start_byte = true;
}

void
Sagetech_protocol::Slice_to_next_start_byte(Packet_len_size_t startindex)
{
    Packet_len_size_t sbindex;
    for (sbindex = startindex; sbindex < incoming_packet_len; sbindex++) {
        if (incoming_packet[sbindex] == START_BYTE) {
            sbindex++;
            incoming_packet_len -= sbindex;
            for (Packet_len_size_t i = 0; i < incoming_packet_len; i++) {
                incoming_packet[i] = incoming_packet[sbindex + i];
            }
            return;
        }
    }
    incoming_packet_len = 0;
}

void
Sagetech_protocol::Calc_checksum(const uint8_t* data, Packet_len_size_t len, uint8_t& fletcher, uint8_t& additive)
{
    additive = 0xa5;
    fletcher = 0xa5;
    for (Packet_len_size_t i = 0; i < len; i++)
    {
        additive += data[i];
        fletcher += additive;
    }
}
