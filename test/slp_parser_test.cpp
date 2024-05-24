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

TEST(parseHeaderTest, ValidLangTagLength)
{
    // Language Tag Length that is valid
    slp::buffer testData{0x00,0x01,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x04,0x00,0x00,
                         0x00,0x00};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_EQ(rc, 0);
}

TEST(parseHeaderTest, InvalidLangTagLength)
{
    // Language Tag Length that is too big
    slp::buffer testData{0x00,0x01,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x05,0x00,0x00,
                         0x00,0x00};

    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);

    EXPECT_NE(rc, 0);
}

  /*  0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |      Service Location header (function = SrvTypeRqst = 9)     |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |        length of PRList       |        <PRList> String        \
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |   length of Naming Authority  |   <Naming Authority String>   \
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |     length of <scope-list>    |      <scope-list> String      \
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
TEST(parseSrvTypeRqst, BasicGoodPath)
{
    // Basic buffer with 0 for all lengths
    slp::buffer testData{0x00,0x09,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00};
    slp::Message req;
    int rc = slp::SUCCESS;
    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);

    EXPECT_EQ(rc, 0);
}

TEST(parseSrvTypeRqst, GoodPathWithData)
{
    // Basic buffer with some valid lengths and data
    slp::buffer testData{0x00,0x09,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x04,     /* Lang Length */
                         'L','A','N','G',
                         0x00,0x04,     /* PRlist length*/
                         'P','R','L','T',
                         0x00,0x08,     /* Naming auth length */
                         'N','A','M','E',
                         'A','U','T','H',
                         0x00,0x05,     /* Scope length*/
                         'S','C','O','P',
                         'E'};
    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(req.header.langtag, "LANG");
    EXPECT_EQ(req.header.langtagLen, 4);

    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(req.body.srvtyperqst.prList, "PRLT");
    EXPECT_EQ(req.body.srvtyperqst.namingAuth, "NAMEAUTH");
    EXPECT_EQ(req.body.srvtyperqst.scopeList, "SCOPE");
}

TEST(parseSrvTypeRqst, GoodPathMatchSlptoolFindSrvTypes)
{
    // This matches what "slptool -u <server> findsrvtypes" sends
    slp::buffer testData{0x02,0x09,0x00,0x00,
                         0x1d,0x00,0x00,0x00,
                         0x00,0x00,0x74,0xe2,
                         0x00,0x02,     /* Lang Length */
                         'e','n',
                         0x00,0x00,     /* PRlist length*/
                         0xff,0xff,     /* Naming auth length */
                         0x00,0x07,     /* Scope length*/
                         'D','E','F','A',
                         'U','L','T'};
    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(req.header.langtag, "en");
    EXPECT_EQ(req.header.langtagLen, 2);

    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(req.body.srvtyperqst.namingAuth, "");
    EXPECT_EQ(req.body.srvtyperqst.scopeList, "DEFAULT");
}

TEST(parseSrvTypeRqst, BadPathSizes)
{
    // Basic buffer with some valid lengths and data
    slp::buffer testData{0x00,0x09,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x00,0x00,0x00,
                         0x00,0x04,     /* Lang Length */
                         'L','A','N','G',
                         0x00,0x05,     /* PRlist length*/
                         'P','R','L','T',
                         0x00,0x08,     /* Naming auth length */
                         'N','A','M','E',
                         'A','U','T','H',
                         0x00,0x05,     /* Scope length*/
                         'S','C','O','P',
                         'E'};
    slp::Message req;
    int rc = slp::SUCCESS;
    std::tie(rc, req) = slp::parser::internal::parseHeader(testData);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(req.header.langtag, "LANG");
    EXPECT_EQ(req.header.langtagLen, 4);

    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);
    EXPECT_NE(rc, 0);

    // Fix PR, make Name invalid
    testData[19] = 4;
    testData[25] = 5;
    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);
    EXPECT_NE(rc, 0);

    // Fix Name, make Scope invalid
    testData[25] = 8;
    testData[35] = 10;
    rc = slp::parser::internal::parseSrvTypeRqst(testData,req);
    EXPECT_NE(rc, 0);
}