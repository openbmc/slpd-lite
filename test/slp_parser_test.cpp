#include "slp.hpp"
#include "slp_meta.hpp"
#include <gtest/gtest.h>

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

TEST(parseHeaderTest, BasicGoodPath)
{
    // Basic buffer with valid Function-ID
    slp::buffer testData{0x00,0x01,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_EQ(rc, 0);
}

TEST(parseHeaderTest, InvalidBufferSize)
{
    // 1 byte too small
    slp::buffer testData{0x00,0x01,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_NE(rc, 0);
}
