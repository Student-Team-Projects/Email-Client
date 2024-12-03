// Slightly modified version of ftxui Input.cpp class, that allows cursor selection

// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <algorithm>   // for max, min
#include <cstddef>     // for size_t
#include <cstdint>     // for uint32_t
#include <functional>  // for function
#include <memory>   // for allocator, shared_ptr, allocator_traits<>::value_type
#include <sstream>  // for basic_istream, stringstream
#include <string>   // for string, basic_string, operator==, getline
#include <utility>  // for move
#include <vector>   // for vector

#include "ftxui/component/captured_mouse.hpp"     // for CapturedMouse
#include "ftxui/component/component.hpp"          // for Make, Input
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/event.hpp"  // for Event, Event::ArrowDown, Event::ArrowLeft, Event::ArrowLeftCtrl, Event::ArrowRight, Event::ArrowRightCtrl, Event::ArrowUp, Event::Backspace, Event::Delete, Event::End, Event::Home, Event::Return
#include "ftxui/component/mouse.hpp"  // for Mouse, Mouse::Left, Mouse::Pressed
#include "ftxui/component/screen_interactive.hpp"  // for Component
#include "ftxui/dom/elements.hpp"  // for operator|, reflect, text, Element, xflex, hbox, Elements, frame, operator|=, vbox, focus, focusCursorBarBlinking, select
#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/string.hpp"           // for string_width

#include "select_input.hpp" 

#ifndef FTXUI_SCREEN_STRING_INTERNAL_HPP
#define FTXUI_SCREEN_STRING_INTERNAL_HPP

#include <cstdint>

namespace ftxui {

bool EatCodePoint(const std::string& input,
                  size_t start,
                  size_t* end,
                  uint32_t* ucs);
bool EatCodePoint(const std::wstring& input,
                  size_t start,
                  size_t* end,
                  uint32_t* ucs);

bool IsCombining(uint32_t ucs);
bool IsFullWidth(uint32_t ucs);
bool IsControl(uint32_t ucs);

size_t GlyphPrevious(const std::string& input, size_t start);
size_t GlyphNext(const std::string& input, size_t start);

// Return the index in the |input| string of the glyph at |glyph_offset|,
// starting at |start|
size_t GlyphIterate(const std::string& input,
                    int glyph_offset,
                    size_t start = 0);

// Returns the number of glyphs in |input|.
int GlyphCount(const std::string& input);

// Properties from:
// https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/WordBreakProperty.txt
enum class WordBreakProperty : int8_t {
  ALetter,
  CR,
  Double_Quote,
  Extend,
  ExtendNumLet,
  Format,
  Hebrew_Letter,
  Katakana,
  LF,
  MidLetter,
  MidNum,
  MidNumLet,
  Newline,
  Numeric,
  Regional_Indicator,
  Single_Quote,
  WSegSpace,
  ZWJ,
};
WordBreakProperty CodepointToWordBreakProperty(uint32_t codepoint);
std::vector<WordBreakProperty> Utf8ToWordBreakProperty(
    const std::string& input);

bool IsWordBreakingCharacter(const std::string& input, size_t glyph_index);
}  // namespace ftxui

#endif /* end of include guard: FTXUI_SCREEN_STRING_INTERNAL_HPP */

#include "ftxui/util/ref.hpp"                // for StringRef, Ref


// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
// Similar to std::clamp, but allow hi to be lower than lo.
template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return v < lo ? lo : hi < v ? hi : v;
}

namespace ftxui {

namespace {

std::vector<std::string> Split(const std::string& input) {
  std::vector<std::string> output;
  std::stringstream ss(input);
  std::string line;
  while (std::getline(ss, line)) {
    output.push_back(line);
  }
  if (input.back() == '\n') {
    output.emplace_back("");
  }
  return output;
}

size_t GlyphWidth(const std::string& input, size_t iter) {
  uint32_t ucs = 0;
  if (!EatCodePoint(input, iter, &iter, &ucs)) {
    return 0;
  }
  if (IsFullWidth(ucs)) {
    return 2;
  }
  return 1;
}

bool IsWordCodePoint(uint32_t codepoint) {
  switch (CodepointToWordBreakProperty(codepoint)) {
    case WordBreakProperty::ALetter:
    case WordBreakProperty::Hebrew_Letter:
    case WordBreakProperty::Katakana:
    case WordBreakProperty::Numeric:
      return true;

    case WordBreakProperty::CR:
    case WordBreakProperty::Double_Quote:
    case WordBreakProperty::LF:
    case WordBreakProperty::MidLetter:
    case WordBreakProperty::MidNum:
    case WordBreakProperty::MidNumLet:
    case WordBreakProperty::Newline:
    case WordBreakProperty::Single_Quote:
    case WordBreakProperty::WSegSpace:
    // Unexpected/Unsure
    case WordBreakProperty::Extend:
    case WordBreakProperty::ExtendNumLet:
    case WordBreakProperty::Format:
    case WordBreakProperty::Regional_Indicator:
    case WordBreakProperty::ZWJ:
      return false;
  }
  return false;  // NOT_REACHED();
}

bool IsWordCharacter(const std::string& input, size_t iter) {
  uint32_t ucs = 0;
  if (!EatCodePoint(input, iter, &iter, &ucs)) {
    return false;
  }

  return IsWordCodePoint(ucs);
}

// An input box. The user can type text into it.
class SelectableInputBase : public ComponentBase, public InputOption {
 public:
  // NOLINTNEXTLINE
  SelectableInputBase(InputOption option) : InputOption(std::move(option)) {}
  bool static_content = false;

 private:
  // Component implementation:

  enum class Selection{
    IDLE,
    MOUSE_SELECTION,
    SHIFT_SELECTION,
    SELECTED
  };

  Ref<int> selection_start = 0;
  Ref<int> selection_end = 0;
  Selection selection_state = Selection::IDLE;

  Element Render() override {
    const bool is_focused = Focused();
    const auto focused =
        (is_focused || hovered_) ? focusCursorBarBlinking : select;

    auto transform_func =
        transform ? transform : InputOption::Default().transform;

    // placeholder.
    if (content->empty()) {
      auto element = text(placeholder()) | xflex | frame;
      if (is_focused) {
        element |= focus;
      }

      return transform_func({
                 std::move(element), hovered_, is_focused,
                 true  // placeholder
             }) |
             reflect(box_);
    }

    Elements elements;
    const std::vector<std::string> lines = Split(*content);


    cursor_position() = clamp(cursor_position(), 0, (int)content->size());

    // Find the line and index of the cursor.
    int cursor_line = 0;
    int cursor_char_index = cursor_position();
    for (const auto& line : lines) {
      if (cursor_char_index <= (int)line.size()) {
        break;
      }

      cursor_char_index -= line.size() + 1;
      cursor_line++;
    }

    if (lines.empty()) {
      elements.push_back(text("") | focused);
    }

    elements.reserve(lines.size());
    
    if(selection_state != Selection::IDLE){
      // int selection_left = selection_start();
      // int selection_right = selection_end();

      // if(selection_state != Selection::SELECTED){
      //   selection_right = cursor_position();
      // }

      // if(selection_left > selection_right){
      //   std::swap(selection_left, selection_right);
      // }
      
      // int char_range_start = 0;

      // for(const std::string& line : lines){
      //   // [char_range_start, char_range_end) ends on \n
      //   int char_range_end = char_range_start + line.size() + 1;

      //   std::vector<int> special_points = {cursor_position(), selection_left, selection_right, char_range_start, char_range_end};
      //   std::sort(special_points.begin(), special_points.end());
      //   special_points.erase(
      //     std::unique(special_points.begin(), special_points.end()),
      //     special_points.end()
      //   );
      //   special_points.push_back(cursor_position());
      //   std::sort(special_points.begin(), special_points.end());
      //   special_points.erase(
      //     std::remove_if(special_points.begin(), special_points.end(), [&](int point){
      //       return point < char_range_start || point > char_range_end;
      //     }),
      //     special_points.end()
      //   );
        
      //   int prev_point = char_range_start;
      //   ftxui::Elements line_elements = {};
      //   for(auto point : special_points){
      //     auto highlight_selected = [&](Element e){
      //       if(selection_left <= prev_point && point <= selection_right){
      //         return e | bgcolor(Color::GrayLight);
      //       }
      //       return e;
      //     };
          
      //     if(prev_point == point){
      //       line_elements.push_back(text("") | focused | reflect(cursor_box_) | highlight_selected);
      //       continue;
      //     }
      //     line_elements.push_back(Text(line.substr(prev_point - char_range_start, point - prev_point), highlight_selected));
      //     prev_point = point;
      //   }
      //   elements.push_back(hbox(std::move(line_elements)) | xflex);

      //   char_range_start += line.size() + 1;
      if(selection_state != Selection::SELECTED) selection_end() = cursor_position();
      int selection_left = std::min(selection_start(), selection_end());
      int selection_right = std::max(selection_start(), selection_end());
      std::string sb = "";
      ftxui::Elements row_elements = {};
      bool highlight = false;
      auto highlight_selected = [&highlight](ftxui::Element e){
          if(highlight){
            return e | bgcolor(Color::GrayLight);
          }
          return e;
      };

      for(size_t i = 0; i < content().size(); ++i){
        char c = content()[i];
        
        if(i == selection_left){
          row_elements.push_back(Text(sb));
          highlight = true;
          
          sb = "";
        }

        if(i == cursor_position()){
          row_elements.push_back(Text(sb) | highlight_selected);
          if(c == '\n'){
            row_elements.push_back(text(" ") | focused | reflect(cursor_box_) | highlight_selected);
          }else{
            sb = c;
            row_elements.push_back(Text(sb) | focused | reflect(cursor_box_) | highlight_selected);
          }

          sb = "";
        }

        if(c == '\n'){
          row_elements.push_back(Text(sb) | highlight_selected);
          elements.push_back(hbox(row_elements));

          sb = "";
          row_elements = {};
        }

        if(i != cursor_position() && c != '\n'){
          sb += c;
        }

        if(i == selection_right){
          row_elements.push_back(Text(sb) | highlight_selected);
          highlight = false;
          
          sb = "";
        }
      }
      
      if(sb.size() > 0){
        row_elements.push_back(Text(sb) | highlight_selected);
        elements.push_back(hbox(row_elements));
      }
      
      auto element = vbox(std::move(elements)) | frame;
      return transform_func({
               std::move(element), hovered_, is_focused,
               false  // placeholder
           }) |
           xflex | reflect(box_);
    }
    
    
    for (size_t i = 0; i < lines.size(); ++i) {
      const std::string& line = lines[i];

      // This is not the cursor line.
      if (int(i) != cursor_line) {
        elements.push_back(Text(line));
        continue;
      }

      // The cursor is at the end of the line.
      if (cursor_char_index >= (int)line.size()) {
        elements.push_back(hbox({
                               Text(line),
                               text(" ") | focused | reflect(cursor_box_),
                           }) |
                           xflex);
        continue;
      }

      // The cursor is on this line.
      const int glyph_start = cursor_char_index;
      const int glyph_end = GlyphNext(line, glyph_start);
      const std::string part_before_cursor = line.substr(0, glyph_start);
      const std::string part_at_cursor =
          line.substr(glyph_start, glyph_end - glyph_start);
      const std::string part_after_cursor = line.substr(glyph_end);
      auto element = hbox({
                         Text(part_before_cursor),
                         Text(part_at_cursor) | focused | reflect(cursor_box_),
                         Text(part_after_cursor),
                     }) |
                     xflex;
      elements.push_back(element);
    }

    auto element = vbox(std::move(elements)) | frame;
    return transform_func({
               std::move(element), hovered_, is_focused,
               false  // placeholder
           }) |
           xflex | reflect(box_);
  }

  Element Text(const std::string& input, std::function<Element(Element)> wrapper = [](Element e){return e;}){
    if (!password()) {
      return text(input) | wrapper ;
    }

    std::string out;
    out.reserve(10 + input.size() * 3 / 2);
    for (size_t i = 0; i < input.size(); ++i) {
      out += "•";
    }
    return text(out) | wrapper;
  }

  bool HandleBackspace() {
    selection_state = Selection::IDLE;
    if (cursor_position() == 0) {
      return false;
    }
    if (static_content) return true;
    const size_t start = GlyphPrevious(content(), cursor_position());
    const size_t end = cursor_position();
    content->erase(start, end - start);
    cursor_position() = start;
    return true;
  }

  bool HandleDelete() {
    selection_state = Selection::IDLE;
    if (cursor_position() == (int)content->size()) {
      return false;
    }
    if (static_content) return true;
    const size_t start = cursor_position();
    const size_t end = GlyphNext(content(), cursor_position());
    content->erase(start, end - start);
    return true;
  }

  bool HandleArrowLeft() {
    if (cursor_position() == 0) {
      return false;
    }

    cursor_position() = GlyphPrevious(content(), cursor_position());
    return true;
  }

  bool HandleArrowRight() {
    if (cursor_position() == (int)content->size()) {
      return false;
    }

    cursor_position() = GlyphNext(content(), cursor_position());
    return true;
  }

  size_t CursorColumn() {
    size_t iter = cursor_position();
    int width = 0;
    while (true) {
      if (iter == 0) {
        break;
      }
      iter = GlyphPrevious(content(), iter);
      if (content()[iter] == '\n') {
        break;
      }
      width += GlyphWidth(content(), iter);
    }
    return width;
  }

  // Move the cursor `columns` on the right, if possible.
  void MoveCursorColumn(int columns) {
    while (columns > 0) {
      if (cursor_position() == (int)content().size() ||
          content()[cursor_position()] == '\n') {
        return;
      }

      columns -= GlyphWidth(content(), cursor_position());
      cursor_position() = GlyphNext(content(), cursor_position());
    }
  }

  bool HandleArrowUp() {
    if (cursor_position() == 0) {
      return false;
    }

    const size_t columns = CursorColumn();

    // Move cursor at the beginning of 2 lines above.
    while (true) {
      if (cursor_position() == 0) {
        return true;
      }
      const size_t previous = GlyphPrevious(content(), cursor_position());
      if (content()[previous] == '\n') {
        break;
      }
      cursor_position() = previous;
    }
    cursor_position() = GlyphPrevious(content(), cursor_position());
    while (true) {
      if (cursor_position() == 0) {
        break;
      }
      const size_t previous = GlyphPrevious(content(), cursor_position());
      if (content()[previous] == '\n') {
        break;
      }
      cursor_position() = previous;
    }

    MoveCursorColumn(columns);
    return true;
  }

  bool HandleArrowDown() {
    if (cursor_position() == (int)content->size()) {
      return false;
    }

    const size_t columns = CursorColumn();

    // Move cursor at the beginning of the next line
    while (true) {
      if (content()[cursor_position()] == '\n') {
        break;
      }
      cursor_position() = GlyphNext(content(), cursor_position());
      if (cursor_position() == (int)content().size()) {
        return true;
      }
    }
    cursor_position() = GlyphNext(content(), cursor_position());

    MoveCursorColumn(columns);
    return true;
  }

  bool HandleHome() {
    cursor_position() = 0;
    return true;
  }

  bool HandleEnd() {
    cursor_position() = content->size();
    return true;
  }

  bool HandleReturn() {
    if (multiline()) {
      HandleCharacter("\n");
    }
    on_enter();
    return true;
  }

  bool HandleCharacter(const std::string& character) {
    selection_state = Selection::IDLE;
    if (static_content) return true;
    content->insert(cursor_position(), character);
    cursor_position() += character.size();
    on_change();

    return true;
  }

  bool HandleCopy(){
    if(selection_state == Selection::IDLE){
      return false;
    }
    int start = std::min(selection_start(), selection_end());
    int end = std::max(selection_start(), selection_end());

    std::string command = "echo '" + content().substr(start, end - start) + "' | xclip -selection clipboard";
    std::system(command.c_str());

    selection_state = Selection::IDLE;
    return true;
  }

  bool HandleCtrlX(){
    if(selection_state != Selection::SELECTED){
      return false;
    }

    int start = std::min(selection_start(), selection_end());
    int end = std::max(selection_start(), selection_end());

    int cs = content().size();
    if(start < 0 || cs < start || cs < end){
      selection_state = Selection::IDLE;
      return true;
    }

    std::string command = "echo '" + content().substr(start, end - start) + "' | xclip -selection clipboard";
    std::system(command.c_str());

    content().erase(start, end - start);

    selection_state = Selection::IDLE;
    return true;
  }

  bool OnEvent(Event event) override {
    cursor_position() = clamp(cursor_position(), 0, (int)content->size());

    if (event == Event::Return) {
      return HandleReturn();
    }
    if (event.is_character()) {
      return HandleCharacter(event.character());
    }
    if (event.is_mouse()) {
      return HandleMouse(event);
    }
    if (event == Event::Backspace) {
      return HandleBackspace();
    }
    if (event == Event::Delete) {
      return HandleDelete();
    }
    if (event == Event::ArrowLeft) {
      return HandleArrowLeft();
    }
    if (event == Event::ArrowRight) {
      return HandleArrowRight();
    }
    if (event == Event::ArrowUp) {
      return HandleArrowUp();
    }
    if (event == Event::ArrowDown) {
      return HandleArrowDown();
    }
    if (event == Event::Home) {
      return HandleHome();
    }
    if (event == Event::End) {
      return HandleEnd();
    }
    if (event == Event::ArrowLeftCtrl) {
      return HandleLeftCtrl();
    }
    if (event == Event::ArrowRightCtrl) {
      return HandleRightCtrl();
    }
    if (event == Event::Special("\x19")){
      return HandleCtrlX();
    }
    return false;
  }

  bool HandleMouseRight(Event event){
    hovered_ = box_.Contain(event.mouse().x, event.mouse().y)
               && CaptureMouse(event);
    if (!hovered_) {
      return false;
    }

    if (event.mouse().motion != Mouse::Pressed) {
      return false;
    }

    return HandleCopy();
  }

  bool HandleMouseLeft(Event event){
    // DESELECTION CODE
    if (
      event.mouse().motion == Mouse::Released
      && selection_state == Selection::MOUSE_SELECTION   
    ) {
      selection_state = Selection::SELECTED;
      selection_end = cursor_position();
      if(selection_start() > selection_end()){
        std::swap(selection_start(), selection_end());
      }
      if(selection_start() == selection_end()){
        selection_state = Selection::IDLE;
      }
    }
    //

    hovered_ = box_.Contain(event.mouse().x, event.mouse().y)
               && CaptureMouse(event);
    if (!hovered_) {
      return false;
    }
    
    if (event.mouse().motion != Mouse::Pressed) {
      return false;
    }

    TakeFocus();

    if (content->empty()) {
      cursor_position() = 0;
      return true;
    }

    // Find the line and index of the cursor.
    std::vector<std::string> lines = Split(*content);
    int cursor_line = 0;
    int cursor_char_index = cursor_position();
    for (const auto& line : lines) {
      if (cursor_char_index <= (int)line.size()) {
        break;
      }

      cursor_char_index -= line.size() + 1;
      cursor_line++;
    }
    const int cursor_column =
        string_width(lines[cursor_line].substr(0, cursor_char_index));

    int new_cursor_column = cursor_column + event.mouse().x - cursor_box_.x_min;
    int new_cursor_line = cursor_line + event.mouse().y - cursor_box_.y_min;

    // Fix the new cursor position:
    new_cursor_line = std::max(std::min(new_cursor_line, (int)lines.size()), 0);

    const std::string empty_string;
    const std::string& line = new_cursor_line < (int)lines.size()
                                  ? lines[new_cursor_line]
                                  : empty_string;
    new_cursor_column = clamp(new_cursor_column, 0, string_width(line));

    // Convert back the new_cursor_{line,column} toward cursor_position:
    cursor_position() = 0;
    for (int i = 0; i < new_cursor_line; ++i) {
      cursor_position() += lines[i].size() + 1;
    }
    while (new_cursor_column > 0) {
      new_cursor_column -= GlyphWidth(content(), cursor_position());
      cursor_position() = GlyphNext(content(), cursor_position());
    }

    // SELECTION CODE
    if (
      event.mouse().button == Mouse::Left 
      && event.mouse().motion == Mouse::Motion::Pressed
      && selection_state != Selection::MOUSE_SELECTION
    ) {
      selection_state = Selection::MOUSE_SELECTION;
      selection_start = cursor_position();
    }
    //

    on_change();

    if (new_cursor_column == cursor_column &&  //
        new_cursor_line == cursor_line) {
      return false;
    }

    return true;
  }

  bool HandleLeftCtrl() {
    if (cursor_position() == 0) {
      return false;
    }

    // Move left, as long as left it not a word.
    while (cursor_position()) {
      const size_t previous = GlyphPrevious(content(), cursor_position());
      if (IsWordCharacter(content(), previous)) {
        break;
      }
      cursor_position() = previous;
    }
    // Move left, as long as left is a word character:
    while (cursor_position()) {
      const size_t previous = GlyphPrevious(content(), cursor_position());
      if (!IsWordCharacter(content(), previous)) {
        break;
      }
      cursor_position() = previous;
    }
    return true;
  }

  bool HandleRightCtrl() {
    if (cursor_position() == (int)content().size()) {
      return false;
    }

    // Move right, until entering a word.
    while (cursor_position() < (int)content().size()) {
      cursor_position() = GlyphNext(content(), cursor_position());
      if (IsWordCharacter(content(), cursor_position())) {
        break;
      }
    }
    // Move right, as long as right is a word character:
    while (cursor_position() < (int)content().size()) {
      const size_t next = GlyphNext(content(), cursor_position());
      if (!IsWordCharacter(content(), cursor_position())) {
        break;
      }
      cursor_position() = next;
    }

    return true;
  }

  bool HandleMouse(Event event) {
    if(event.mouse().button == Mouse::Right){
      return HandleMouseRight(std::move(event));
    }

    if(event.mouse().button == Mouse::Left){
      return HandleMouseLeft(std::move(event));
    }

    return false;
  }

  bool Focusable() const final { return true; }

  bool hovered_ = false;

  Box box_;
  Box cursor_box_;
};

}  // namespace

Component SelectableInput(InputOption option) {
  return Make<SelectableInputBase>(std::move(option));
}

Component SelectableInput(StringRef content, InputOption option) {
  option.content = std::move(content);
  return Make<SelectableInputBase>(std::move(option));
}

Component SelectableInput(StringRef content, StringRef placeholder, InputOption option) {
  option.content = std::move(content);
  option.placeholder = std::move(placeholder);
  return Make<SelectableInputBase>(std::move(option));
}

Component SelectableText(StringRef content, InputOption option) {
  option.content = std::move(content);
  auto comp = Make<SelectableInputBase>(std::move(option));
  comp->static_content = true;
  return comp;
}

Component SelectableText(InputOption option) {
  auto comp = Make<SelectableInputBase>(std::move(option));
  comp->static_content = true;
  return comp;
}
}  // namespace ftxui
