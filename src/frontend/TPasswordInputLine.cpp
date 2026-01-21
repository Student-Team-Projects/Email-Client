#include "TPasswordInputLine.hpp"
#include <cstring>

void TPasswordInputLine::draw() {
    if (data == nullptr) {
        TInputLine::draw();
        return;
    }

    char* originalData = data;
    int len = strlen(originalData);
    char* maskBuffer = new char[maxLen + 1];
    memset(maskBuffer, '*', len);
    maskBuffer[len] = '\0';
    data = maskBuffer;

    try {
        TInputLine::draw();
    } catch (...) {
        data = originalData;
        delete[] maskBuffer;
        throw;
    }

    data = originalData;
    delete[] maskBuffer;
}