#pragma once
#include <string>

class MessageHeader;

class HtmlParser {
public:
    // Function to extract text from HTML content
    static std::string extract_text(const std::string& html);
    static std::string decode_quoted_printable(const std::string& input);
    static void decode_quoted_printable(MessageHeader& header);
};
