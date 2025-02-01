#include "html_parser.h"

#include <libxml/HTMLparser.h>
#include <libxml/tree.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <regex>

#include "mail_types.h"

void extract_text_from_node(xmlNode* node, std::string& text) {
    for (xmlNode* current = node; current != nullptr; current = current->next) {
        if (current->type == XML_TEXT_NODE) {
            text += reinterpret_cast<const char*>(current->content);
            text += " "; // Add space between text nodes
        }
        extract_text_from_node(current->children, text);
    }
}

std::string HtmlParser::decode_quoted_printable(const std::string& input) {
    std::ostringstream output;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '=' && i + 2 < input.length() && 
            isxdigit(input[i + 1]) && isxdigit(input[i + 2])) {
            std::string hex = input.substr(i + 1, 2);
            char decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
            output << decodedChar;
            i += 2;  // Skip the two hex digits
        } else {
            output << input[i];
        }
    }
    return output.str();
}

void HtmlParser::decode_quoted_printable(MessageHeader &header)
{
    header.recipient = HtmlParser::decode_quoted_printable(header.recipient);
    header.sender = HtmlParser::decode_quoted_printable(header.sender);
    header.subject = HtmlParser::decode_quoted_printable(header.subject);
    // body = HtmlParser::decode_quoted_printable(body);
}

std::string remove_leftovers(const std::string& input) {
    std::string result = input;

    // Remove @media (...) { ... } sections
    result = std::regex_replace(result, std::regex(R"(@media\s*\([^\)]*\)\s*\{[^\}]*\})"), "");

    // Remove .container {...} parts
    result = std::regex_replace(result, std::regex(R"(\.container\s*\{[^\}]*\})"), "");

    return result;
}

std::string clean_whitespace(const std::string& input) {
    std::regex whitespaceBlockPattern(R"(\s+)");
    std::string result;

    std::sregex_iterator begin(input.begin(), input.end(), whitespaceBlockPattern);
    std::sregex_iterator end;

    std::string::const_iterator lastPos = input.begin();
    for (auto it = begin; it != end; ++it) {
        // Append text before the whitespace block
        result.append(lastPos, input.begin() + it->position());

        // Determine the replacement based on the content of the whitespace block
        std::string block = it->str();
        result += (block.find('\n') != std::string::npos) ? "\n" : " ";

        // Update lastPos to continue from the end of the current match
        lastPos = input.begin() + it->position() + it->length();
    }
    // Append any remaining text after the last match
    result.append(lastPos, input.end());

    // Optional: Trim leading and trailing whitespace
    result = std::regex_replace(result, std::regex(R"(^[ \t\n]+|[ \t\n]+$)"), "");

    return result;
}

std::string clean_equal_signs(const std::string& input) {
    // Remove '=XX' where XX is a hexadecimal value
    std::regex eqPattern(R"(=([0-9A-Fa-f]{2}))");
    std::string cleaned = std::regex_replace(input, eqPattern, "");

    // Remove '=' just before newlines, and also remove the newline
    cleaned = std::regex_replace(cleaned, std::regex(R"(=\n)"), "");

    return cleaned;
}

std::string naive_wrap_text(const std::string& input, size_t maxLength) {
    std::stringstream result;
    size_t currentLineLength = 0;

    for (size_t i = 0; i < input.length(); ++i) {
        char ch = input[i];

        if (currentLineLength >= maxLength && ch != '\n') {
            result << "\n";
            currentLineLength = 0;
        }

        result << ch;
        currentLineLength++;

        if (ch == '\n') {
            currentLineLength = 0;
        }
    }

    return result.str();
}

std::string HtmlParser::extract_text(const std::string& html) {
    // handle empty input
    if (html.empty()) {
        return "";
    }

    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == nullptr) {
        std::cerr << "Failed to parse HTML" << std::endl;
        std::cerr << html << std::endl;
        throw std::runtime_error("Failed to parse HTML");
    }

    // Extract text
    std::string plainText;
    xmlNode* root = xmlDocGetRootElement(doc);
    extract_text_from_node(root, plainText);

    // Free document
    xmlFreeDoc(doc);

    // Decode Quoted-Printable
    plainText = decode_quoted_printable(plainText);

    // Remove leftovers
    plainText = remove_leftovers(plainText);

    // Clean whitespace
    plainText = clean_whitespace(plainText);

    // Clean equal signs
    plainText = clean_equal_signs(plainText);

    // Temporary: Naive text wrapping
    plainText = naive_wrap_text(plainText, 80);

    return plainText;
}
