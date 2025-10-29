/**
 * @file test_bej.cpp
 * @brief Unit tests for BEJparser using Google Test framework
 * @details 7 main suits and 1 integration
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>

extern "C" {
#include "../src/bej.h"
}

// ============================================================================
// NNINT Reading Tests
// ============================================================================

class BejNNINTTest : public ::testing::Test {
protected:
    uint8_t buffer[256];
    size_t offset;
    uint32_t value;
};

TEST_F(BejNNINTTest, ReadSingleByte_ZeroValue) {
    // NNINT format: length=0, no additional bytes
    buffer[0] = 0x00;
    offset = 0;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0);
    EXPECT_EQ(offset, 1);
}

TEST_F(BejNNINTTest, ReadSingleByte_SmallValue) {
    // nnint: length=1, value=0x42 (66 decimal)
    buffer[0] = 0x01;
    buffer[1] = 0x42;
    offset = 0;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0x42);
    EXPECT_EQ(offset, 2);
}

TEST_F(BejNNINTTest, ReadTwoBytes_LittleEndian) {
    // nnint: length=2, value=0x3412 (little-endian)
    buffer[0] = 0x02;
    buffer[1] = 0x12;
    buffer[2] = 0x34;
    offset = 0;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0x3412);
    EXPECT_EQ(offset, 3);
}

TEST_F(BejNNINTTest, ReadThreeBytes) {
    // nnint: length=3, value=0x563412
    buffer[0] = 0x03;
    buffer[1] = 0x12;
    buffer[2] = 0x34;
    buffer[3] = 0x56;
    offset = 0;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0x563412);
    EXPECT_EQ(offset, 4);
}

TEST_F(BejNNINTTest, ReadFourBytes) {
    // nnint: length=4, value=0x78563412
    buffer[0] = 0x04;
    buffer[1] = 0x12;
    buffer[2] = 0x34;
    buffer[3] = 0x56;
    buffer[4] = 0x78;
    offset = 0;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0x78563412);
    EXPECT_EQ(offset, 5);
}

TEST_F(BejNNINTTest, ReadAtOffset) {
    buffer[5] = 0x01;
    buffer[6] = 0xFF;
    offset = 5;
    
    ASSERT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), SUCCESS);
    EXPECT_EQ(value, 0xFF);
    EXPECT_EQ(offset, 7);
}

TEST_F(BejNNINTTest, ReadOutOfBounds) {
    buffer[0] = 0x05;  // claims 5 bytes
    offset = 254;      // only 2 bytes available
    
    EXPECT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), FAILURE);
}

TEST_F(BejNNINTTest, ReadAtEndOfBuffer) {
    offset = 256;  // At end
    
    EXPECT_EQ(bej_read_nnint(buffer, &offset, sizeof(buffer), &value), FAILURE);
}

// ============================================================================
// Sequence Number Tests
// ============================================================================

TEST(BejSequenceTest, DecodeSequenceNumber_EvenValue) {
    uint8_t data[] = {0x01, 0x0A};  // nnint: length=1, value=0x0A (10)
    size_t offset = 0;
    uint32_t seqnum = 0;
    uint8_t selector = 0;
    
    ASSERT_EQ(bej_read_sequence_number(data, &offset, sizeof(data), &seqnum, &selector), SUCCESS);
    EXPECT_EQ(seqnum, 5);      // 10 >> 1 = 5
    EXPECT_EQ(selector, 0);    // 10 & 1 = 0 (major schema)
    EXPECT_EQ(offset, 2);
}

TEST(BejSequenceTest, DecodeSequenceNumber_OddValue) {
    uint8_t data[] = {0x01, 0x0B};  // nnint: value=0x0B (11)
    size_t offset = 0;
    uint32_t seqnum = 0;
    uint8_t selector = 0;
    
    ASSERT_EQ(bej_read_sequence_number(data, &offset, sizeof(data), &seqnum, &selector), SUCCESS);
    EXPECT_EQ(seqnum, 5);      // 11 >> 1 = 5
    EXPECT_EQ(selector, 1);    // 11 & 1 = 1 (annotation)
    EXPECT_EQ(offset, 2);
}

TEST(BejSequenceTest, DecodeSequenceNumber_Zero) {
    uint8_t data[] = {0x00};  // nnint: length=0, value=0
    size_t offset = 0;
    uint32_t seqnum = 0;
    uint8_t selector = 0;
    
    ASSERT_EQ(bej_read_sequence_number(data, &offset, sizeof(data), &seqnum, &selector), SUCCESS);
    EXPECT_EQ(seqnum, 0);
    EXPECT_EQ(selector, 0);
}

// ============================================================================
// Format Reading Tests
// ============================================================================

TEST(BejFormatTest, ReadFormat_SetNoFlags) {
    uint8_t data[] = {0x00};  // format=0 (Set), Flags=0
    size_t offset = 0;
    uint8_t format = 0, flags = 0;
    
    ASSERT_EQ(bej_read_format(data, &offset, sizeof(data), &format, &flags), SUCCESS);
    EXPECT_EQ(format, BEJ_FORMAT_SET);
    EXPECT_EQ(flags, 0);
    EXPECT_EQ(offset, 1);
}

TEST(BejFormatTest, ReadFormat_IntegerWithFlags) {
    uint8_t data[] = {0x23};  // format=2 (Integer), Flags=3
    size_t offset = 0;
    uint8_t format = 0, flags = 0;
    
    ASSERT_EQ(bej_read_format(data, &offset, sizeof(data), &format, &flags), SUCCESS);
    EXPECT_EQ(format, BEJ_FORMAT_INTEGER);
    EXPECT_EQ(flags, 0x03);
    EXPECT_EQ(offset, 1);
}

TEST(BejFormatTest, ReadFormat_String) {
    uint8_t data[] = {0x40};  // format=4 (String), fags=0
    size_t offset = 0;
    uint8_t format = 0, flags = 0;
    
    ASSERT_EQ(bej_read_format(data, &offset, sizeof(data), &format, &flags), SUCCESS);
    EXPECT_EQ(format, BEJ_FORMAT_STRING);
    EXPECT_EQ(flags, 0);
}

TEST(BejFormatTest, ReadFormat_OutOfBounds) {
    uint8_t data[] = {0x40};
    size_t offset = 10;  // past end
    uint8_t format = 0, flags = 0;
    
    EXPECT_EQ(bej_read_format(data, &offset, sizeof(data), &format, &flags), FAILURE);
}

// ============================================================================
// Dictionary Parsing Tests
// ============================================================================

class BejDictionaryTest : public ::testing::Test {
protected:
    bej_dictionary_context_t dict;
    uint8_t dict_data[256];
    
    void SetUp() override {
        memset(&dict, 0, sizeof(dict));
        memset(dict_data, 0, sizeof(dict_data));
    }
};

TEST_F(BejDictionaryTest, ParseValidDictionary) {
    // minimal valid dictionary header (12 bytes)
    dict_data[0] = 0x00;  // ver tag
    dict_data[1] = 0x00;  // truncation flag
    dict_data[2] = 0x03;  // entry count (LE) = 3
    dict_data[3] = 0x00;
    dict_data[4] = 0x01;  // schema version (LE) = 0x01000000
    dict_data[5] = 0x00;
    dict_data[6] = 0x00;
    dict_data[7] = 0x00;
    
    ASSERT_EQ(bej_parse_dict(&dict, dict_data, 12), SUCCESS);
    EXPECT_EQ(dict.version_tag, 0x00);
    EXPECT_EQ(dict.truncation_flag, 0x00);
    EXPECT_EQ(dict.entry_count, 3);
    EXPECT_EQ(dict.schema_version, 0x01);
    EXPECT_EQ(dict.data, dict_data);
    EXPECT_EQ(dict.data_size, 12);
}

TEST_F(BejDictionaryTest, ParseDictionary_BigEndianEntryCount) {
    dict_data[2] = 0xAB;  // entry count = 0x00AB (171 dec)
    dict_data[3] = 0x00;
    
    ASSERT_EQ(bej_parse_dict(&dict, dict_data, 12), SUCCESS);
    EXPECT_EQ(dict.entry_count, 0xAB);
}

TEST_F(BejDictionaryTest, ParseDictionary_TooSmall) {
    EXPECT_EQ(bej_parse_dict(&dict, dict_data, 11), FAILURE);  // < 12 bytes
}

TEST_F(BejDictionaryTest, ParseDictionary_NullPointer) {
    EXPECT_EQ(bej_parse_dict(nullptr, dict_data, 12), FAILURE);
    EXPECT_EQ(bej_parse_dict(&dict, nullptr, 12), FAILURE);
}

// ============================================================================
// Dictionary Entry Lookup Tests: TODO
// ============================================================================


// ============================================================================
// Integer Decoding Tests
// ============================================================================

class BejIntegerTest : public ::testing::Test {
protected:
    bej_context_t ctx;
    FILE *output;
    char output_buf[256];
    
    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        output = fmemopen(output_buf, sizeof(output_buf), "w");
        ctx.output = output;
    }
    
    void TearDown() override {
        if (output) fclose(output);
    }
    
    std::string GetOutput() {
        fflush(output);
        return std::string(output_buf);
    }
};

TEST_F(BejIntegerTest, DecodePositiveInteger_OneByte) {
    uint8_t value[] = {0x2A};  // 42
    ASSERT_EQ(decode_integer(&ctx, value, 1), SUCCESS);
    EXPECT_EQ(GetOutput(), "42");
}

TEST_F(BejIntegerTest, DecodePositiveInteger_TwoBytes) {
    uint8_t value[] = {0x00, 0x01};  // 256 (little-endian)
    ASSERT_EQ(decode_integer(&ctx, value, 2), SUCCESS);
    EXPECT_EQ(GetOutput(), "256");
}

TEST_F(BejIntegerTest, DecodeNegativeInteger_OneByte) {
    uint8_t value[] = {0xFF};  // -1
    ASSERT_EQ(decode_integer(&ctx, value, 1), SUCCESS);
    EXPECT_EQ(GetOutput(), "-1");
}

TEST_F(BejIntegerTest, DecodeNegativeInteger_TwoBytes) {
    uint8_t value[] = {0xFF, 0xFF};  // -1 (little-endian)
    ASSERT_EQ(decode_integer(&ctx, value, 2), SUCCESS);
    EXPECT_EQ(GetOutput(), "-1");
}

TEST_F(BejIntegerTest, DecodeZero) {
    uint8_t value[] = {0x00};
    ASSERT_EQ(decode_integer(&ctx, value, 1), SUCCESS);
    EXPECT_EQ(GetOutput(), "0");
}

TEST_F(BejIntegerTest, DecodeInvalidLength_Zero) {
    uint8_t value[] = {0x00};
    EXPECT_EQ(decode_integer(&ctx, value, 0), FAILURE);
}

TEST_F(BejIntegerTest, DecodeInvalidLength_TooLarge) {
    uint8_t value[9] = {0};
    EXPECT_EQ(decode_integer(&ctx, value, 9), FAILURE);
}

// ============================================================================
// String Decoding Tests
// ============================================================================

class BejStringTest : public ::testing::Test {
protected:
    bej_context_t ctx;
    FILE *output;
    char output_buf[256];
    
    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        output = fmemopen(output_buf, sizeof(output_buf), "w");
        ctx.output = output;
    }
    
    void TearDown() override {
        if (output) fclose(output);
    }
    
    std::string GetOutput() {
        fflush(output);
        return std::string(output_buf);
    }
};

TEST_F(BejStringTest, DecodeSimpleString) {
    uint8_t value[] = {'H', 'e', 'l', 'l', 'o', '\0'};
    ASSERT_EQ(decode_string(&ctx, value, 6), SUCCESS);
    EXPECT_EQ(GetOutput(), "\"Hello\"");
}

TEST_F(BejStringTest, DecodeStringWithoutNullTerminator) {
    uint8_t value[] = {'T', 'e', 's', 't'};
    ASSERT_EQ(decode_string(&ctx, value, 4), SUCCESS);
    EXPECT_EQ(GetOutput(), "\"Test\"");
}

TEST_F(BejStringTest, DecodeEmptyString) {
    uint8_t value[] = {'\0'};
    ASSERT_EQ(decode_string(&ctx, value, 1), SUCCESS);
    EXPECT_EQ(GetOutput(), "\"\"");
}

TEST_F(BejStringTest, DecodeStringWithEscapes) {
    uint8_t value[] = {'\"', '\\', '\n', '\t', '\0'};
    ASSERT_EQ(decode_string(&ctx, value, 5), SUCCESS);
    EXPECT_EQ(GetOutput(), "\"\\\"\\\\\\n\\t\"");
}

TEST_F(BejStringTest, DecodeStringZeroLength) {
    uint8_t value[] = {'T'};
    ASSERT_EQ(decode_string(&ctx, value, 0), SUCCESS);
    EXPECT_EQ(GetOutput(), "\"\"");
}

// ============================================================================
// BEJ Header Validation Tests
// ============================================================================

class BejHeaderTest : public ::testing::Test {
protected:
    bej_context_t ctx;
    uint8_t bej_data[256];
    FILE *output;
    
    void SetUp() override {
        memset(&ctx, 0, sizeof(ctx));
        memset(bej_data, 0, sizeof(bej_data));
        output = tmpfile();
        ctx.output = output;
        ctx.bej_data = bej_data;
        ctx.bej_size = sizeof(bej_data);
    }
    
    void TearDown() override {
        if (output) fclose(output);
    }
};

TEST_F(BejHeaderTest, ValidHeader_Version1_1_0) {
    bej_data[0] = 0x00;
    bej_data[1] = 0xF0;
    bej_data[2] = 0xF1;
    bej_data[3] = 0xF1;
    bej_data[4] = 0x00;
    bej_data[5] = 0x00;
    bej_data[6] = 0x00;  // major schema
    
    bej_decode(&ctx);
}

TEST_F(BejHeaderTest, ValidHeader_Version1_0_0) {
    bej_data[0] = 0x00;
    bej_data[1] = 0xF0;
    bej_data[2] = 0xF0;  // different minor version
    bej_data[3] = 0xF1;
    bej_data[4] = 0x00;
    bej_data[5] = 0x00;
    bej_data[6] = 0x00;
    
    bej_decode(&ctx);
}

TEST_F(BejHeaderTest, InvalidHeader_WrongVersion) {
    bej_data[0] = 0x01;  // wrong version byte
    bej_data[1] = 0xF0;
    bej_data[2] = 0xF1;
    bej_data[3] = 0xF1;
    
    EXPECT_EQ(bej_decode(&ctx), FAILURE);
}

TEST_F(BejHeaderTest, InvalidHeader_TooSmall) {
    ctx.bej_size = 6;  // less than 7 bytes required
    
    EXPECT_EQ(bej_decode(&ctx), FAILURE);
}

TEST_F(BejHeaderTest, InvalidSchemaClass_Error) {
    bej_data[0] = 0x00;
    bej_data[1] = 0xF0;
    bej_data[2] = 0xF1;
    bej_data[3] = 0xF1;
    bej_data[4] = 0x00;
    bej_data[5] = 0x00;
    bej_data[6] = 0x04;  // error schema class (not supported)
    
    EXPECT_EQ(bej_decode(&ctx), FAILURE);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(BejIntegrationTest, DecodeSimpleInteger) {
    // minimal dictionary
    uint8_t dict_data[256] = {0};
    dict_data[0] = 0x00;   // version
    dict_data[1] = 0x00;   // truncation
    dict_data[2] = 0x01;   // entry count = 1
    dict_data[3] = 0x00;
    dict_data[4] = 0x01;   // schema version
    dict_data[5] = 0x00;
    dict_data[6] = 0x00;
    dict_data[7] = 0x00;
    
    // entry at offset 12
    dict_data[12] = 0x02;  // format: Integer
    dict_data[13] = 0x00;  // sequence: 0
    dict_data[14] = 0x00;
    dict_data[15] = 0x00;  // child offset
    dict_data[16] = 0x00;
    dict_data[17] = 0x00;  // child count
    dict_data[18] = 0x00;
    dict_data[19] = 0x05;  // name length
    dict_data[20] = 0x64;  // name offset: 100
    dict_data[21] = 0x00;
    
    memcpy(&dict_data[100], "Value", 5);
    
    // BEJ data: header + SFLV
    uint8_t bej_data[] = {
        0x00, 0xF0, 0xF1, 0xF1,  // version
        0x00, 0x00,               // flags
        0x00,                     // schema class
        0x00,                     // sequence (nnint: length=0, value=0)
        0x20,                     //format: Integer (0x02 << 4)
        0x01, 0x2A                // length (nnint)): 1, Value: 42
    };
    
    FILE *output = tmpfile();
    bej_context_t ctx;
    
    ASSERT_EQ(bej_init_context(&ctx, dict_data, sizeof(dict_data),
                               bej_data, sizeof(bej_data), output), SUCCESS);
    
    EXPECT_EQ(bej_decode(&ctx), SUCCESS);
    
    rewind(output);     // read output
    char buffer[256];
    size_t read = fread(buffer, 1, sizeof(buffer) - 1, output);
    buffer[read] = '\0';
    
    EXPECT_NE(strstr(buffer, "42"), nullptr);
    
    fclose(output);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}