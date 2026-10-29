#include "backend/html_parser.h"

#include <gtest/gtest.h>

// Test suite name: the tested class or file + "Test".
// Test name: what the code should do, written as a sentence.
TEST(HtmlParserTest, DecodeQuotedPrintableDecodesHexEscapes) {
    // EXPECT_* reports a failure and continues, ASSERT_* stops the test.
    // Use ASSERT_* only when the rest of the test makes no sense after a failure.
    EXPECT_EQ(HtmlParser::decode_quoted_printable("caf=C3=A9"), "caf\xC3\xA9");
    EXPECT_EQ(HtmlParser::decode_quoted_printable("1+1=3D2"), "1+1=2");
    EXPECT_EQ(HtmlParser::decode_quoted_printable("no escapes"), "no escapes");
}
