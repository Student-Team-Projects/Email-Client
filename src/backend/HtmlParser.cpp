#include "HtmlParser.h"
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <stdexcept>

// Helper function to recursively extract text content
void extractTextFromNode(xmlNode* node, std::string& text) {
    for (xmlNode* current = node; current != nullptr; current = current->next) {
        if (current->type == XML_TEXT_NODE) {
            text += reinterpret_cast<const char*>(current->content);
            text += " "; // Add space between text nodes
        }
        extractTextFromNode(current->children, text);
    }
}

std::string HtmlParser::extractText(const std::string& html) {
    // Parse HTML content
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == nullptr) {
        throw std::runtime_error("Failed to parse HTML");
    }

    // Extract text
    std::string plainText;
    xmlNode* root = xmlDocGetRootElement(doc);
    extractTextFromNode(root, plainText);

    // Free document
    xmlFreeDoc(doc);

    // Trim extra spaces and return
    return plainText;
}
