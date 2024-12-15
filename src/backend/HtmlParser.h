#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <string>

class HtmlParser {
public:
    // Function to extract text from HTML content
    static std::string extractText(const std::string& html);
};

#endif // HTML_PARSER_H
