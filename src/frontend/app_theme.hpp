#include <tvision/tv.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include "TPasswordInputLine.hpp"
#include "commands.hpp"
#include "logging/logging.hpp"
#include "app.hpp"

#include <tvision/tv.h>

namespace AppTheme {

    namespace ColorPalette {
        using RGB = TColorRGB;

        // Tła
        constexpr RGB DeepNavy      { 20,  40,  80 };  // Ciemne tło (główne)
        constexpr RGB DarkerNavy    { 30,  50, 100 };  // Tło ramek pasywnych
        constexpr RGB PanelDark     { 30,  50,  90 };  // Info panel
        constexpr RGB ButtonBg      { 50,  70, 120 };  // Tło przycisku
        constexpr RGB InputBg       { 30,  50, 110 };  // Tło pola wpisywania
        
        // Teksty
        constexpr RGB White         { 255, 255, 255 };
        constexpr RGB OffWhite      { 240, 240, 255 }; // Lekko niebieskawy biały
        constexpr RGB SoftBlue      { 230, 230, 255 }; // Etykiety
        constexpr RGB PastelBlue    { 180, 200, 255 }; // Pasywna ramka
        
        // Akcenty i Wyroznienia
        constexpr RGB Highlight     { 60,  90, 150 };  // Zaznaczenie (Focus)
        constexpr RGB ActiveBlue    { 40,  70, 130 };  // Aktywna ramka/strzałki
        constexpr RGB Selection     { 90, 120, 180 };  // Mocniejsze zaznaczenie
        constexpr RGB SelectionText { 200, 220, 255 }; // Tekst zaznaczony w input

        // Inne
        constexpr RGB Amber         { 255, 220, 140 }; // Skróty klawiszowe (Alt+...)
        constexpr RGB Disabled      { 150, 150, 150 }; // Nieaktywne elementy
        constexpr RGB DisabledBg    { 50,  50,  50 };
        constexpr RGB Black         { 0,   0,   0   };
    }

    namespace Idx {
        const uchar FramePassive      = 1;
        const uchar FrameActive       = 2;
        const uchar StaticText        = 6;
        const uchar LabelNormal       = 7;
        const uchar LabelSelected     = 8;
        const uchar LabelShortcut     = 9;
        const uchar ButtonNormal      = 10;
        const uchar ButtonDefault     = 11;
        const uchar ButtonSelected    = 12;
        const uchar ButtonDisabled    = 13;
        const uchar ButtonShortcut    = 14;
        const uchar ButtonShadow      = 15;
        const uchar ClusterNormal     = 16;
        const uchar ClusterSelected   = 17;
        const uchar ClusterShortcut   = 18;
        const uchar InputLineNormal   = 19;
        const uchar InputLineSelected = 20;
        const uchar InputLineArrows   = 21;
        const uchar HistoryArrow      = 22;
        const uchar HistorySides      = 23;
        const uchar ScrollBarPage     = 24;
        const uchar ScrollBarControls = 25;
        const uchar ListViewerNormal  = 26;
        const uchar ListViewerFocused = 27;
        const uchar ListViewerSelect  = 28;
        const uchar InfoPanel         = 30;
        const uchar WindowScrollPage  = 40;
        const uchar WindowScrollIcons = 41;
    }

    TColorAttr getColor(uchar index, TColorAttr defaultAttr);
}