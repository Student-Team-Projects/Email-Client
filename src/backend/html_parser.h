#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <string>

class HtmlParser {
public:
    // Function to extract text from HTML content
    static std::string extract_text(const std::string& html);

    static std::string decode_quoted_printable(const std::string& input);
};

#endif // HTML_PARSER_H
