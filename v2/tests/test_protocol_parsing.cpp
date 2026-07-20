/*
Unit tests for the pure protocol-parsing logic in RTE7 and Oven5R6900 --
checksums and byte/response parsing. No real hardware is touched: these
methods never read hSerial, only the byte/string buffers passed in.

Each test constructs its device object with a COM port name ("COM999")
that doesn't exist, so the constructor's serial-open attempt fails
harmlessly (it logs to cerr, it doesn't throw) and hSerial stays unused
for the rest of the test.

Run with: build/tests/Release/run_tests.exe
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "controls/RTE7.h"
#include "controls/Oven5R6900.h"

TEST_SUITE("RTE7 protocol parsing") {

    TEST_CASE("chillerChecksum matches a hand-computed value") {
        RTE7 bath("COM999");
        std::vector<uint8_t> data = {0xCA, 0x00, 0x01, 0x20, 0x02, 0x10, 0x00, 0xFF};
        // sum(data[1..]) = 0x00+0x01+0x20+0x02+0x10+0x00+0xFF = 0x132 -> low byte 0x32 -> XOR 0xFF = 0xCD
        CHECK(bath.chillerChecksum(data) == 0xCD);
    }

    TEST_CASE("hexStringToBytes converts a spaced hex string") {
        RTE7 bath("COM999");
        std::vector<uint8_t> bytes = bath.hexStringToBytes("CA 00 01");
        REQUIRE(bytes.size() == 3);
        CHECK(bytes[0] == 0xCA);
        CHECK(bytes[1] == 0x00);
        CHECK(bytes[2] == 0x01);
    }

    TEST_CASE("parse_float_response decodes a valid, checksum-correct reply") {
        RTE7 bath("COM999");
        // Layout: 0xCA, addr, ?, command, data_len, qualifier, data..., checksum
        // qualifier 0x10 -> precision 10^1 = 10; data 0x00 0xFF -> raw 255 -> 255/10 = 25.5
        std::vector<uint8_t> response = {0xCA, 0x00, 0x01, 0x20, 0x02, 0x10, 0x00, 0xFF, 0xCD};
        float temp = bath.parse_float_response(response);
        CHECK(temp == doctest::Approx(25.5));
    }

    TEST_CASE("parse_float_response returns -999 on a checksum mismatch") {
        RTE7 bath("COM999");
        // Same as above but the last data byte is corrupted (0xFE instead of 0xFF),
        // so the stale checksum byte 0xCD no longer matches.
        std::vector<uint8_t> response = {0xCA, 0x00, 0x01, 0x20, 0x02, 0x10, 0x00, 0xFE, 0xCD};
        float temp = bath.parse_float_response(response);
        CHECK(temp == -999);
    }

    TEST_CASE("parse_float_response decodes a negative temperature (signed, not unsigned)") {
        RTE7 bath("COM999");
        // Manual's own worked example: -10.5C, qualifier 0x11 (precision 10, C units),
        // raw bytes 0xFF 0x97 = -105 as signed 16-bit -> -105/10 = -10.5.
        std::vector<uint8_t> response = {0xCA, 0x00, 0x01, 0x20, 0x03, 0x11, 0xFF, 0x97, 0x34};
        float temp = bath.parse_float_response(response);
        CHECK(temp == doctest::Approx(-10.5));
    }

    TEST_CASE("parse_float_response returns -999 on a Bad Command error frame") {
        RTE7 bath("COM999");
        // Bath's "Bad Command" response: CA 00 01 0F 02 01 <echoed cmd> cs -- must not be
        // decoded as if the error-code/echoed-command bytes were a real reading.
        std::vector<uint8_t> response = {0xCA, 0x00, 0x01, 0x0F, 0x02, 0x01, 0x20, 0xCC};
        float temp = bath.parse_float_response(response);
        CHECK(temp == -999);
    }
}

TEST_SUITE("Oven5R6900 protocol parsing") {

    TEST_CASE("checksum matches a hand-computed value") {
        Oven5R6900 tc("COM999");
        // sum(ASCII of "12345678") = 0x31+0x32+..+0x38 = 420 = 0x1A4 -> low byte 0xA4 -> "a4"
        CHECK(tc.checksum("*12345678") == "a4");
    }

    TEST_CASE("hexStringToBytes converts a spaced hex string") {
        Oven5R6900 tc("COM999");
        std::vector<uint8_t> bytes = tc.hexStringToBytes("2A 00 01");
        REQUIRE(bytes.size() == 3);
        CHECK(bytes[0] == 0x2A);
        CHECK(bytes[1] == 0x00);
        CHECK(bytes[2] == 0x01);
    }

    TEST_CASE("parse_response decodes a valid, checksum-correct reply") {
        Oven5R6900 tc("COM999");
        std::string response = "*12345678a4\r";   // checksum "a4" matches "*12345678"
        int32_t value = tc.parse_response(response);
        CHECK(value == 0x12345678);
    }

    TEST_CASE("parse_response returns -999 on a checksum mismatch") {
        Oven5R6900 tc("COM999");
        std::string response = "*12345678a5\r";   // wrong checksum (should be "a4")
        int32_t value = tc.parse_response(response);
        CHECK(value == -999);
    }
}
