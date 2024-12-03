#pragma once
#include <ftxui/component/component.hpp>
namespace ftxui {
    Component SelectableInput(StringRef, StringRef, InputOption); 
    Component SelectableInput(StringRef, InputOption);
    Component SelectableInput(InputOption);

    Component SelectableText(InputOption);
    Component SelectableText(StringRef, InputOption);
}