#pragma once
#include <ftxui/component/component.hpp>
namespace ftxui {
    /*
    *   Functions returning SelectableInput and SelectableText Component
    *   
    *   SelectableInput is a Component that works like ftxui's Input
    *   and on top of it allows for mouse selection of text and copying.
    *   
    *   SelectableText is just a SelectableInput with character input disabled. 
    */
    Component SelectableInput(StringRef, StringRef, InputOption); 
    Component SelectableInput(StringRef, InputOption);
    Component SelectableInput(InputOption);

    Component SelectableText(InputOption);
    Component SelectableText(StringRef, InputOption);
}