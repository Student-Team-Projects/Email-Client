#include "app_theme.hpp"

namespace AppTheme {

    TColorAttr getColor(uchar index, TColorAttr defaultAttr) {
        using namespace ColorPalette; 

        auto attr = [](TColorRGB fg, TColorRGB bg) -> TColorAttr {
            return { fg, bg };
        };

        switch (index) {
            // Ramki
            case Idx::FramePassive:      return attr(PastelBlue, DarkerNavy);
            case Idx::FrameActive:       return attr(White, ActiveBlue);
            
            // Teksty
            case Idx::StaticText:        return attr(OffWhite, DeepNavy);
            case Idx::LabelNormal:       return attr(SoftBlue, DeepNavy);
            case Idx::LabelSelected:     return attr(White, Highlight);
            case Idx::LabelShortcut:     return attr(Amber, DeepNavy);
            
            // Przyciski
            case Idx::ButtonNormal:      return attr(OffWhite, ButtonBg);
            case Idx::ButtonDefault:     return attr(White, ActiveBlue);
            case Idx::ButtonSelected:    return attr(White, Selection);
            case Idx::ButtonDisabled:    return attr(Disabled, DisabledBg);
            case Idx::ButtonShortcut:    return attr(Amber, ButtonBg);
            case Idx::ButtonShadow:      return attr(DeepNavy, DeepNavy);
            
            // Checkboxy
            case Idx::ClusterNormal:     return attr(SoftBlue, DeepNavy);
            case Idx::ClusterSelected:   return attr(White, Highlight);
            case Idx::ClusterShortcut:   return attr(Amber, DeepNavy);
            
            // Pola tekstowe
            case Idx::InputLineNormal:   return attr(White, InputBg);
            case Idx::InputLineSelected: return attr(Black, SelectionText);
            case Idx::InputLineArrows:   return attr(White, ActiveBlue);
            
            // Historia
            case Idx::HistoryArrow:      return attr(White, ActiveBlue);
            case Idx::HistorySides:      return attr(White, DarkerNavy);

            case Idx::ScrollBarPage:     return attr(DarkerNavy, DeepNavy);
            case Idx::ScrollBarControls: return attr(White, ActiveBlue);
            
            // Listy
            case Idx::ListViewerNormal:  return attr(OffWhite, DeepNavy);
            case Idx::ListViewerFocused: return attr(White, Highlight);
            case Idx::ListViewerSelect:  return attr(White, Selection);
            
            // Inne
            case Idx::InfoPanel:         return attr(SelectionText, PanelDark);

            // ScrollBar
            case Idx::WindowScrollPage:  return attr(DarkerNavy, DeepNavy);
            case Idx::WindowScrollIcons: return attr(White, ActiveBlue);
            
            default:
                return defaultAttr;
        }
    }
}