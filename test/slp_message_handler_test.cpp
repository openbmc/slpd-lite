#include "slp.hpp"
#include "slp_meta.hpp"

#include <gtest/gtest.h>

// Header
/*  0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Version    |  Function-ID  |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Length, contd.|O|F|R|       reserved          |Next Ext Offset|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Next Extension Offset, contd.|              XID              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Language Tag Length      |         Language Tag          \
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

// Error response
/*  0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        Service Location header (function = SrvRply = 2)       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        Error Code             |        URL Entry count        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |       <URL Entry 1>          ...       <URL Entry N>          \
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/

TEST(processError, BasicGoodPath)
{
    // Basic buffer with valid Function-ID
    slp::buffer testData{0x02, 0x01, 0x00, 0x00, 0x0E, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_EQ(rc, 0);

    // Verify all expected fields show up in response buffer
    std::vector<uint8_t> resp = slp::handler::processError(
        req, static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));

    EXPECT_EQ(resp.size(), slp::header::MIN_LEN + slp::response::SIZE_ERROR);
    EXPECT_EQ(resp[slp::header::OFFSET_VERSION], 2);
    EXPECT_EQ(resp[slp::header::OFFSET_FUNCTION], 2);
    EXPECT_EQ(resp[slp::header::MIN_LEN + 1],
              static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));
}

TEST(processError, InvalidLangTag)
{
    // Basic buffer with valid Function-ID
    slp::buffer testData{0x02, 0x01, 0x00, 0x00, 0x0E, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10,
                         0x00, 0x01, 0x02, 0x03, 0x04};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_NE(rc, 0);

    // Verify all expected fields show up in response buffer even
    // with an inavlid langugage tag size in the header
    std::vector<uint8_t> resp = slp::handler::processError(
        req, static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));

    EXPECT_EQ(resp.size(), slp::header::MIN_LEN + slp::response::SIZE_ERROR);
    EXPECT_EQ(resp[slp::header::OFFSET_VERSION], 2);
    EXPECT_EQ(resp[slp::header::OFFSET_FUNCTION], 2);
    EXPECT_EQ(resp[slp::header::MIN_LEN + 1],
              static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));
}

TEST(processError, InvalidEverything)
{
    // Basic buffer with valid Function-ID
    slp::buffer testData{0x03, 0x99, 0x99, 0x99, 0xA0, 0xA0, 0xA1,
                         0xA2, 0xB9, 0x55, 0x44, 0x33, 0x21, 0x90,
                         0x78, 0x1,  0x02, 0x03, 0x04};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_NE(rc, 0);

    // Verify all expected fields show up in response buffer even
    // with an inavlid langugage tag size in the header
    std::vector<uint8_t> resp = slp::handler::processError(
        req, static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));

    EXPECT_EQ(resp.size(), slp::header::MIN_LEN + slp::response::SIZE_ERROR);
    EXPECT_EQ(resp[slp::header::OFFSET_VERSION], 3);
    EXPECT_EQ(resp[slp::header::OFFSET_FUNCTION], 0x9A);
    EXPECT_EQ(resp[slp::header::MIN_LEN + 1],
              static_cast<uint8_t>(slp::Error::MSG_NOT_SUPPORTED));
}
