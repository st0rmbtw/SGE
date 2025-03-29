// VERSION: 0.12

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// SIMD includes on supported platforms
#if !defined(CLAY_DISABLE_SIMD) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
#include <emmintrin.h>
#elif !defined(CLAY_DISABLE_SIMD) && defined(__aarch64__)
#include <arm_neon.h>
#endif

// -----------------------------------------
// HEADER DECLARATIONS ---------------------
// -----------------------------------------

#ifndef CLAY_HEADER_CPP
#define CLAY_HEADER_CPP

#include "clay.h"

#if !( \
    (defined(__cplusplus) && __cplusplus >= 202002L) || \
    (defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
    defined(_MSC_VER) \
)
#error "Clay requires C99, C++20, or MSVC"
#endif

#ifdef CLAY
#undef CLAY

#define CLAY(...)                                                                                                                                           \
    for (                                                                                                                                                   \
        CLAY__ELEMENT_DEFINITION_LATCH = (Clay__OpenElement(), Clay__ConfigureOpenElement(*reinterpret_cast<Clay_ElementDeclaration*>(&CLAY__CONFIG_WRAPPER(ElementDeclaration, __VA_ARGS__)), 0);  \
        CLAY__ELEMENT_DEFINITION_LATCH < 1;                                                                                                                 \
        ++CLAY__ELEMENT_DEFINITION_LATCH, Clay__CloseElement()                                                                                              \
    )

#endif

#ifdef CLAY_SIZING_GROW
#undef CLAY_SIZING_GROW
#define CLAY_SIZING_GROW(...) (clay::SizingAxis::grow(__VA_ARGS__))
#endif

#ifdef CLAY_SIZING_FIXED
#undef CLAY_SIZING_FIXED
#define CLAY_SIZING_FIXED(fixedSize) (SizingAxis::fixed(fixedSize, fixedSize))
#endif

#ifdef CLAY_SIZING_PERCENT
#undef CLAY_SIZING_PERCENT
#define CLAY_SIZING_PERCENT(percentOfParent) (SizingAxis::percent(percentOfParent))
#endif


#ifdef CLAY_CORNER_RADIUS
#undef CLAY_CORNER_RADIUS
#define CLAY_CORNER_RADIUS(radius) (CornerRadius { radius, radius, radius, radius })
#endif

namespace clay {

// Utility Structs -------------------------

// Note: String is not guaranteed to be null terminated. It may be if created from a literal C string,
// but it is also used to represent slices.
using String = Clay_String;

// StringSlice is used to represent non owning string slices, and includes
// a baseChars field which points to the string this slice is derived from.
struct StringSlice {
    int32_t length;
    const char *chars;
    const char *baseChars; // The source string / char* that this slice was derived from
};

using Context = Clay_Context;
using Arena = Clay_Arena;

struct Dimensions {
    float width = 0.0f;
    float height = 0.0f;
};

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;
};

// Internally clay conventionally represents colors as 0-255, but interpretation is up to the renderer.
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

struct BoundingBox {
    float x, y, width, height;
};

// Primarily created via the CLAY_ID(), CLAY_IDI(), CLAY_ID_LOCAL() and CLAY_IDI_LOCAL() macros.
// Represents a hashed string ID used for identifying and finding specific clay UI elements, required
// by functions such as PointerOver() and GetElementData().
struct ElementId {
    uint32_t id; // The resulting hash generated from the other fields.
    uint32_t offset; // A numerical offset applied after computing the hash from stringId.
    uint32_t baseId; // A base hash value to start from, for example the parent element ID is used when calculating CLAY_ID_LOCAL().
    String stringId; // The string id to hash.
};

// Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
// The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
struct CornerRadius {
    float topLeft = 0.0f;
    float topRight = 0.0f;
    float bottomLeft = 0.0f;
    float bottomRight = 0.0f;
};

// Element Configs ---------------------------

// Controls the direction in which child elements will be automatically laid out.
enum class LayoutDirection : uint8_t {
    // (Default) Lays out child elements from left to right with increasing x.
    LeftToRight,
    // Lays out child elements from top to bottom with increasing y.
    TopToBottom,
};

// Controls the alignment along the x axis (horizontal) of child elements.
enum class LayoutAlignmentX : uint8_t {
    // (Default) Aligns child elements to the left hand side of this element, offset by padding.width.left
    AlignXLeft,
    // Aligns child elements to the right hand side of this element, offset by padding.width.right
    AlignXRight,
    // Aligns child elements horizontally to the center of this element
    AlignXCenter,
};

// Controls the alignment along the y axis (vertical) of child elements.
enum class LayoutAlignmentY : uint8_t {
    // (Default) Aligns child elements to the top of this element, offset by padding.width.top
    AlignYTop,
    // Aligns child elements to the bottom of this element, offset by padding.width.bottom
    AlignYBottom,
    // Aligns child elements vertiically to the center of this element
    AlignYCenter,
};

// Controls how the element takes up space inside its parent container.
enum class SizingType : uint8_t {
    // (default) Wraps tightly to the size of the element's contents.
    Fit,
    // Expands along this axis to fill available space in the parent element, sharing it with other GROW elements.
    Grow,
    // Expects 0-1 range. Clamps the axis size to a percent of the parent container's axis size minus padding and child gaps.
    Percent,
    // Clamps the axis size to an exact size in pixels.
    Fixed,
};

// Controls how child elements are aligned on each axis.
struct ChildAlignment {
    LayoutAlignmentX x; // Controls alignment of children along the x axis.
    LayoutAlignmentY y; // Controls alignment of children along the y axis.
};

// Controls the minimum and maximum size in pixels that this element is allowed to grow or shrink to,
// overriding sizing types such as FIT or GROW.
struct SizingMinMax {
    float min; // The smallest final size of the element on this axis will be this value in pixels.
    float max; // The largest final size of the element on this axis will be this value in pixels.
};

// Controls the sizing of this element along one axis inside its parent container.
struct SizingAxis {
    union {
        SizingMinMax minMax; // Controls the minimum and maximum size in pixels that this element is allowed to grow or shrink to, overriding sizing types such as FIT or GROW.
        float percent; // Expects 0-1 range. Clamps the axis size to a percent of the parent container's axis size minus padding and child gaps.
    } size;
    SizingType type; // Controls how the element takes up space inside its parent container.

    static SizingAxis grow(float min = 0.0f, float max = 0.0f) {
        return SizingAxis {
            .size = { .minMax = { .min = min, .max = max }},
            .type = SizingType::Grow
        };
    }

    inline static SizingAxis fixed(float min = 0.0f, float max = 0.0f) {
        return SizingAxis {
            .size = { .minMax = { .min = min, .max = max }},
            .type = SizingType::Fixed
        };
    }

    inline static SizingAxis percent(float percent) {
        return SizingAxis {
            .size = { .percent = percent },
            .type = SizingType::Percent
        };
    }
};

// Controls the sizing of this element along one axis inside its parent container.
struct Sizing {
    SizingAxis width; // Controls the width sizing of the element, along the x axis.
    SizingAxis height;  // Controls the height sizing of the element, along the y axis.
};

// Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children
// will be placed.
struct Padding {
    uint16_t left = 0;
    uint16_t right = 0;
    uint16_t top = 0;
    uint16_t bottom = 0;
};

// Controls various settings that affect the size and position of an element, as well as the sizes and positions
// of any child elements.
struct LayoutConfig {
    Sizing sizing = {}; // Controls the sizing of this element inside it's parent container, including FIT, GROW, PERCENT and FIXED sizing.
    Padding padding = {}; // Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children will be placed.
    uint16_t childGap = 0; // Controls the gap in pixels between child elements along the layout axis (horizontal gap for LEFT_TO_RIGHT, vertical gap for TOP_TO_BOTTOM).
    ChildAlignment childAlignment = {}; // Controls how child elements are aligned on each axis.
    LayoutDirection layoutDirection = {}; // Controls the direction in which child elements will be automatically laid out.
};

extern LayoutConfig CLAY_LAYOUT_DEFAULT;
CornerRadius CornerRadius_DEFAULT = {};

// Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
enum class TextWrapMode : uint8_t {
    // (default) breaks on whitespace characters.
    Words = 0,
    // Don't break on space characters, only on newlines.
    Newlines,
    // Disable text wrapping entirely.
    None,
};

// Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
enum class TextAlignment : uint8_t {
    // (default) Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
    AlignLeft,
    // Horizontally aligns wrapped lines of text to the center of their bounding box.
    AlignCenter,
    // Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
    AlignRight,
};

// Controls various functionality related to text elements.
struct TextElementConfig {
    // The RGBA color of the font to render, conventionally specified as 0-255.
    Color textColor;
    // An integer transparently passed to MeasureText to identify the font to use.
    // The debug view will pass fontId = 0 for its internal text.
    uint16_t fontId;
    // Controls the size of the font. Handled by the function provided to MeasureText.
    uint16_t fontSize;
    // Controls extra horizontal spacing between characters. Handled by the function provided to MeasureText.
    uint16_t letterSpacing;
    // Controls additional vertical space between wrapped lines of text.
    uint16_t lineHeight;
    // Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
    // CLAY_TEXT_WRAP_WORDS (default) breaks on whitespace characters.
    // CLAY_TEXT_WRAP_NEWLINES doesn't break on space characters, only on newlines.
    // CLAY_TEXT_WRAP_NONE disables wrapping entirely.
    TextWrapMode wrapMode;
    // Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
    // CLAY_TEXT_ALIGN_LEFT (default) - Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
    // CLAY_TEXT_ALIGN_CENTER - Horizontally aligns wrapped lines of text to the center of their bounding box.
    // CLAY_TEXT_ALIGN_RIGHT - Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
    TextAlignment textAlignment;
    // When set to true, clay will hash the entire text contents of this string as an identifier for its internal
    // text measurement cache, rather than just the pointer and length. This will incur significant performance cost for
    // long bodies of text.
    bool hashStringContents;
};

// Image --------------------------------

// Controls various settings related to image elements.
struct ImageElementConfig {
    void* imageData; // A transparent pointer used to pass image data through to the renderer.
    Dimensions sourceDimensions; // The original dimensions of the source image, used to control aspect ratio.
};

// Floating -----------------------------

// Controls where a floating element is offset relative to its parent element.
// Note: see https://github.com/user-attachments/assets/b8c6dfaa-c1b1-41a4-be55-013473e4a6ce for a visual explanation.
enum class FloatingAttachPointType : uint8_t {
    LeftTop,
    LeftCenter,
    LeftBottom,
    CenterTop,
    CenterCenter,
    CenterBottom,
    RightTop,
    RightCenter,
    RightBottom,
};

// Controls where a floating element is offset relative to its parent element.
struct FloatingAttachPoints {
    FloatingAttachPointType element; // Controls the origin point on a floating element that attaches to its parent.
    FloatingAttachPointType parent; // Controls the origin point on the parent element that the floating element attaches to.
};

// Controls how mouse pointer events like hover and click are captured or passed through to elements underneath a floating element.
enum class PointerCaptureMode : uint8_t {
    // (default) "Capture" the pointer event and don't allow events like hover and click to pass through to elements underneath.
    Capture,
    //Parent, TODO pass pointer through to attached parent

    // Transparently pass through pointer events like hover and click to elements underneath the floating element.
    Passthrough,
};

// Controls which element a floating element is "attached" to (i.e. relative offset from).
enum class FloatingAttachToElement : uint8_t {
    // (default) Disables floating for this element.
    None,
    // Attaches this floating element to its parent, positioned based on the .attachPoints and .offset fields.
    Parent,
    // Attaches this floating element to an element with a specific ID, specified with the .parentId field. positioned based on the .attachPoints and .offset fields.
    ElementWithId,
    // Attaches this floating element to the root of the layout, which combined with the .offset field provides functionality similar to "absolute positioning".
    Root,
};

// Controls various settings related to "floating" elements, which are elements that "float" above other elements, potentially overlapping their boundaries,
// and not affecting the layout of sibling or parent elements.
struct FloatingElementConfig {
    // Offsets this floating element by the provided x,y coordinates from its attachPoints.
    Vector2 offset;
    // Expands the boundaries of the outer floating element without affecting its children.
    Dimensions expand;
    // When used in conjunction with .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, attaches this floating element to the element in the hierarchy with the provided ID.
    // Hint: attach the ID to the other element with .id = CLAY_ID("yourId"), and specify the id the same way, with .parentId = CLAY_ID("yourId").id
    uint32_t parentId;
    // Controls the z index of this floating element and all its children. Floating elements are sorted in ascending z order before output.
    // zIndex is also passed to the renderer for all elements contained within this floating element.
    int16_t zIndex;
    // Controls how mouse pointer events like hover and click are captured or passed through to elements underneath / behind a floating element.
    // Enum is of the form CLAY_ATTACH_POINT_foo_bar. See FloatingAttachPoints for more details.
    // Note: see <img src="https://github.com/user-attachments/assets/b8c6dfaa-c1b1-41a4-be55-013473e4a6ce />
    // and <img src="https://github.com/user-attachments/assets/ebe75e0d-1904-46b0-982d-418f929d1516 /> for a visual explanation.
    FloatingAttachPoints attachPoints;
    // Controls how mouse pointer events like hover and click are captured or passed through to elements underneath a floating element.
    // Capture (default) - "Capture" the pointer event and don't allow events like hover and click to pass through to elements underneath.
    // Passthrough - Transparently pass through pointer events like hover and click to elements underneath the floating element.
    PointerCaptureMode pointerCaptureMode;
    // Controls which element a floating element is "attached" to (i.e. relative offset from).
    // None (default) - Disables floating for this element.
    // Parent - Attaches this floating element to its parent, positioned based on the .attachPoints and .offset fields.
    // ElementWithId - Attaches this floating element to an element with a specific ID, specified with the .parentId field. positioned based on the .attachPoints and .offset fields.
    // Root - Attaches this floating element to the root of the layout, which combined with the .offset field provides functionality similar to "absolute positioning".
    FloatingAttachToElement attachTo;
};

// Custom -----------------------------

// Controls various settings related to custom elements.
struct CustomElementConfig {
    // A transparent pointer through which you can pass custom data to the renderer.
    // Generates CUSTOM render commands.
    void* customData;
};

// Scroll -----------------------------

// Controls the axis on which an element switches to "scrolling", which clips the contents and allows scrolling in that direction.
struct ScrollElementConfig {
    bool horizontal; // Clip overflowing elements on the X axis and allow scrolling left and right.
    bool vertical; // Clip overflowing elements on the YU axis and allow scrolling up and down.
};

// Border -----------------------------

// Controls the widths of individual element borders.
struct BorderWidth {
    uint16_t left;
    uint16_t right;
    uint16_t top;
    uint16_t bottom;
    // Creates borders between each child element, depending on the .layoutDirection.
    // e.g. for LEFT_TO_RIGHT, borders will be vertical lines, and for TOP_TO_BOTTOM borders will be horizontal lines.
    // .betweenChildren borders will result in individual RECTANGLE render commands being generated.
    uint16_t betweenChildren;
};

BorderWidth BorderWidth_DEFAULT = {};

// Controls settings related to element borders.
struct BorderElementConfig {
    Color color; // Controls the color of all borders with width > 0. Conventionally represented as 0-255, but interpretation is up to the renderer.
    BorderWidth width; // Controls the widths of individual borders. At least one of these should be > 0 for a BORDER render command to be generated.
};

// Render Command Data -----------------------------

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_TEXT
struct TextRenderData {
    // A string slice containing the text to be rendered.
    // Note: this is not guaranteed to be null terminated.
    StringSlice stringContents;
    // Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
    Color textColor;
    // An integer representing the font to use to render this text, transparently passed through from the text declaration.
    uint16_t fontId;
    uint16_t fontSize;
    // Specifies the extra whitespace gap in pixels between each character.
    uint16_t letterSpacing;
    // The height of the bounding box for this line of text.
    uint16_t lineHeight;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE
struct RectangleRenderData {
    // The solid background color to fill this rectangle with. Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
    Color backgroundColor;
    // Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
    // The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
    CornerRadius cornerRadius;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE
struct ImageRenderData {
    // The tint color for this image. Note that the default value is 0,0,0,0 and should likely be interpreted
    // as "untinted".
    // Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
    Color backgroundColor;
    // Controls the "radius", or corner rounding of this image.
    // The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
    CornerRadius cornerRadius;
    // The original dimensions of the source image, used to control aspect ratio.
    Dimensions sourceDimensions;
    // A pointer transparently passed through from the original element definition, typically used to represent image data.
    void* imageData;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_CUSTOM
struct CustomRenderData {
    // Passed through from .backgroundColor in the original element declaration.
    // Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
    Color backgroundColor;
    // Controls the "radius", or corner rounding of this custom element.
    // The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
    CornerRadius cornerRadius;
    // A pointer transparently passed through from the original element definition.
    void* customData;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_START || commandType == CLAY_RENDER_COMMAND_TYPE_SCISSOR_END
struct ScrollRenderData {
    bool horizontal;
    bool vertical;
};

// Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_BORDER
struct BorderRenderData {
    // Controls a shared color for all this element's borders.
    // Conventionally represented as 0-255 for each channel, but interpretation is up to the renderer.
    Color color;
    // Specifies the "radius", or corner rounding of this border element.
    // The rounding is determined by drawing a circle inset into the element corner by (radius, radius) pixels.
    CornerRadius cornerRadius;
    // Controls individual border side widths.
    BorderWidth width;
};

// A struct union containing data specific to this command's .commandType
union RenderData {
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE
    RectangleRenderData rectangle;
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_TEXT
    TextRenderData text;
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_IMAGE
    ImageRenderData image;
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_CUSTOM
    CustomRenderData custom;
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_BORDER
    BorderRenderData border;
    // Render command data when commandType == CLAY_RENDER_COMMAND_TYPE_SCROLL
    ScrollRenderData scroll;
};

// Miscellaneous Structs & Enums ---------------------------------

// Data representing the current internal state of a scrolling element.
struct ScrollContainerData {
    // Note: This is a pointer to the real internal scroll position, mutating it may cause a change in final layout.
    // Intended for use with external functionality that modifies scroll position, such as scroll bars or auto scrolling.
    Vector2 *scrollPosition;
    // The bounding box of the scroll element.
    Dimensions scrollContainerDimensions;
    // The outer dimensions of the inner scroll container content, including the padding of the parent scroll container.
    Dimensions contentDimensions;
    // The config that was originally passed to the scroll element.
    ScrollElementConfig config;
    // Indicates whether an actual scroll container matched the provided ID or if the default struct was returned.
    bool found;
};

// Bounding box and other data for a specific UI element.
struct ElementData {
    // The rectangle that encloses this UI element, with the position relative to the root of the layout.
    BoundingBox boundingBox;
    // Indicates whether an actual Element matched the provided ID or if the default struct was returned.
    bool found;
};

// Used by renderers to determine specific handling for each render command.
enum class RenderCommandType : uint8_t {
    // This command type should be skipped.
    None,
    // The renderer should draw a solid color rectangle.
    Rectangle,
    // The renderer should draw a colored border inset into the bounding box.
    Border,
    // The renderer should draw text.
    Text,
    // The renderer should draw an image.
    Image,
    // The renderer should begin clipping all future draw commands, only rendering content that falls within the provided boundingBox.
    ScissorStart,
    // The renderer should finish any previously active clipping, and begin rendering elements in full again.
    ScissorEnd,
    // The renderer should provide a custom implementation for handling this render command based on its .customData
    Custom,
};

struct RenderCommand {
    // A rectangular box that fully encloses this UI element, with the position relative to the root of the layout.
    BoundingBox boundingBox;
    // A struct union containing data specific to this command's commandType.
    RenderData renderData;
    // A pointer transparently passed through from the original element declaration.
    void *userData;
    // The id of this element, transparently passed through from the original element declaration.
    uint32_t id;
    // The z order required for drawing this command correctly.
    // Note: the render command array is already sorted in ascending order, and will produce correct results if drawn in naive order.
    // This field is intended for use in batching renderers for improved performance.
    int16_t zIndex;
    // Specifies how to handle rendering of this command.
    // Rectangle - The renderer should draw a solid color rectangle.
    // Border - The renderer should draw a colored border inset into the bounding box.
    // Text - The renderer should draw text.
    // Image - The renderer should draw an image.
    // ScissorStart - The renderer should begin clipping all future draw commands, only rendering content that falls within the provided boundingBox.
    // ScissorEnd - The renderer should finish any previously active clipping, and begin rendering elements in full again.
    // Custom - The renderer should provide a custom implementation for handling this render command based on its .customData
    RenderCommandType commandType;
};

// A sized array of render commands.
struct RenderCommandArray {
    // The underlying max capacity of the array, not necessarily all initialized.
    int32_t capacity;
    // The number of initialized elements in this array. Used for loops and iteration.
    int32_t length;
    // A pointer to the first element in the internal array.
    RenderCommand* internalArray;
};

// Represents the current state of interaction with clay this frame.
enum class PointerDataInteractionState : uint8_t {
    // A left mouse click, or touch occurred this frame.
    PressedThisFrame,
    // The left mouse button click or touch happened at some point in the past, and is still currently held down this frame.
    Pressed,
    // The left mouse button click or touch was released this frame.
    ReleasedThisFrame,
    // The left mouse button click or touch is not currently down / was released at some point in the past.
    Released,
};

// Information on the current state of pointer interactions this frame.
struct PointerData {
    // The position of the mouse / touch / pointer relative to the root of the layout.
    Vector2 position;
    // Represents the current state of interaction with clay this frame.
    // PressedThisFrame - A left mouse click, or touch occurred this frame.
    // Pressed - The left mouse button click or touch happened at some point in the past, and is still currently held down this frame.
    // ReleasedThisFrame - The left mouse button click or touch was released this frame.
    // Released - The left mouse button click or touch is not currently down / was released at some point in the past.
    PointerDataInteractionState state;
};

struct ElementDeclaration {
    // Primarily created via the CLAY_ID(), CLAY_IDI(), CLAY_ID_LOCAL() and CLAY_IDI_LOCAL() macros.
    // Represents a hashed string ID used for identifying and finding specific clay UI elements, required by functions such as PointerOver() and GetElementData().
    ElementId id;
    // Controls various settings that affect the size and position of an element, as well as the sizes and positions of any child elements.
    LayoutConfig layout;
    // Controls the background color of the resulting element.
    // By convention specified as 0-255, but interpretation is up to the renderer.
    // If no other config is specified, .backgroundColor will generate a RECTANGLE render command, otherwise it will be passed as a property to IMAGE or CUSTOM render commands.
    Color backgroundColor;
    // Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
    CornerRadius cornerRadius;
    // Controls settings related to image elements.
    ImageElementConfig image;
    // Controls whether and how an element "floats", which means it layers over the top of other elements in z order, and doesn't affect the position and size of siblings or parent elements.
    // Note: in order to activate floating, .floating.attachTo must be set to something other than the default value.
    FloatingElementConfig floating;
    // Used to create CUSTOM render commands, usually to render element types not supported by Clay.
    CustomElementConfig custom;
    // Controls whether an element should clip its contents and allow scrolling rather than expanding to contain them.
    ScrollElementConfig scroll;
    // Controls settings related to element borders, and will generate BORDER render commands.
    BorderElementConfig border;
    // A pointer that will be transparently passed through to resulting render commands.
    void *userData;
};

CLAY__WRAPPER_STRUCT(ElementDeclaration);

// Represents the type of error clay encountered while computing layout.
enum class ErrorType : uint8_t {
    // A text measurement function wasn't provided using SetMeasureTextFunction(), or the provided function was null.
    TextMeasurementFunctionNotProvided,
    // Clay attempted to allocate its internal data structures but ran out of space.
    // The arena passed to Initialize was created with a capacity smaller than that required by MinMemorySize().
    ArenaCapacityExceeded,
    // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with SetMaxElementCount().
    ElementsCapacityExceeded,
    // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with SetMaxMeasureTextCacheWordCount().
    TextMeasurementCapacityExceeded,
    // Two elements were declared with exactly the same ID within one layout.
    DuplicateId,
    // A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
    FloatingContainerParentNotFound,
    // An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
    PercentageOver1,
    // Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
    InternalError,
};

// Data to identify the error that clay has encountered.
struct ErrorData {
    // Represents the type of error clay encountered while computing layout.
    // TextMeasurementFunctionNotProvided - A text measurement function wasn't provided using SetMeasureTextFunction(), or the provided function was null.
    // ArenaCapacityExceeded - Clay attempted to allocate its internal data structures but ran out of space. The arena passed to Initialize was created with a capacity smaller than that required by MinMemorySize().
    // ElementsCapacityExceeded - Clay ran out of capacity in its internal array for storing elements. This limit can be increased with SetMaxElementCount().
    // TextMeasurementCapacityExceeded - Clay ran out of capacity in its internal array for storing elements. This limit can be increased with SetMaxMeasureTextCacheWordCount().
    // DuplicateId - Two elements were declared with exactly the same ID within one layout.
    // FloatingContainerParentNotFound - A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
    // PercentageOver1 - An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
    // InternalError - Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
    ErrorType errorType;
    // A string containing human-readable error text that explains the error in more detail.
    String errorText;
    // A transparent pointer passed through from when the error handler was first provided.
    void *userData;
};

// A wrapper struct around Clay's error handler function.
struct ErrorHandler {
    // A user provided function to call when Clay encounters an error during layout.
    void (*errorHandlerFunction)(ErrorData errorText);
    // A pointer that will be transparently passed through to the error handler when it is called.
    void *userData;
};

// Function Forward Declarations ---------------------------------

// Public API functions ------------------------------------------

// Returns the size, in bytes, of the minimum amount of memory Clay requires to operate at its current settings.
uint32_t MinMemorySize(void) {
    return Clay_MinMemorySize();
}
// Creates an arena for clay to use for its internal allocations, given a certain capacity in bytes and a pointer to an allocation of at least that size.
// Intended to be used with MinMemorySize in the following way:
// uint32_t minMemoryRequired = MinMemorySize();
// Arena clayMemory = CreateArenaWithCapacityAndMemory(minMemoryRequired, malloc(minMemoryRequired));
Arena CreateArenaWithCapacityAndMemory(size_t capacity, void *memory) {
    return Clay_CreateArenaWithCapacityAndMemory(capacity, memory);
}
// Sets the state of the "pointer" (i.e. the mouse or touch) in Clay's internal data. Used for detecting and responding to mouse events in the debug view,
// as well as for Hovered() and scroll element handling.
void SetPointerState(Vector2 position, bool pointerDown) {
    Clay_SetPointerState(Clay_Vector2{ .x = position.x, .y = position.y }, pointerDown);
}
// Initialize Clay's internal arena and setup required data before layout can begin. Only needs to be called once.
// - arena can be created using CreateArenaWithCapacityAndMemory()
// - layoutDimensions are the initial bounding dimensions of the layout (i.e. the screen width and height for a full screen layout)
// - errorHandler is used by Clay to inform you if something has gone wrong in configuration or layout.
Context* Initialize(Arena arena, Dimensions layoutDimensions, ErrorHandler errorHandler) {
    return Clay_Initialize(
        arena,
        *reinterpret_cast<Clay_Dimensions*>(&layoutDimensions),
        *reinterpret_cast<Clay_ErrorHandler*>(&errorHandler)
    );
}
// Returns the Context that clay is currently using. Used when using multiple instances of clay simultaneously.
Context* GetCurrentContext() {
    return Clay_GetCurrentContext();
}
// Sets the context that clay will use to compute the layout.
// Used to restore a context saved from GetCurrentContext when using multiple instances of clay simultaneously.
void SetCurrentContext(Context* context) {
    return Clay_SetCurrentContext(context);
}
// Updates the state of Clay's internal scroll data, updating scroll content positions if scrollDelta is non zero, and progressing momentum scrolling.
// - enableDragScrolling when set to true will enable mobile device like "touch drag" scroll of scroll containers, including momentum scrolling after the touch has ended.
// - scrollDelta is the amount to scroll this frame on each axis in pixels.
// - deltaTime is the time in seconds since the last "frame" (scroll update)
void UpdateScrollContainers(bool enableDragScrolling, Vector2 scrollDelta, float deltaTime) {
    Clay_UpdateScrollContainers(enableDragScrolling, Clay_Vector2{ scrollDelta.x, scrollDelta.y }, deltaTime);
}
// Updates the layout dimensions in response to the window or outer container being resized.
void SetLayoutDimensions(Dimensions dimensions) {
    Clay_SetLayoutDimensions(Clay_Dimensions{.width = dimensions.width, .height = dimensions.height });
}
// Called before starting any layout declarations.
void BeginLayout(void) {
    Clay_BeginLayout();
}
// Called when all layout declarations are finished.
// Computes the layout and generates and returns the array of render commands to draw.
RenderCommandArray EndLayout(void) {
    Clay_RenderCommandArray array = Clay_EndLayout();
    return {
        .capacity = array.capacity,
        .length = array.length,
        .internalArray = reinterpret_cast<RenderCommand*>(array.internalArray)
    };
}
// Calculates a hash ID from the given idString.
// Generally only used for dynamic strings when CLAY_ID("stringLiteral") can't be used.
ElementId GetElementId(String idString) {
    Clay_ElementId elementId = Clay_GetElementId(idString);
    return *reinterpret_cast<ElementId*>(&elementId);
}
// Calculates a hash ID from the given idString and index.
// - index is used to avoid constructing dynamic ID strings in loops.
// Generally only used for dynamic strings when CLAY_IDI("stringLiteral", index) can't be used.
ElementId GetElementIdWithIndex(String idString, uint32_t index) {
    Clay_ElementId elementId = Clay_GetElementIdWithIndex(idString, index);
    return *reinterpret_cast<ElementId*>(&elementId);
}
// Returns layout data such as the final calculated bounding box for an element with a given ID.
// The returned ElementData contains a `found` bool that will be true if an element with the provided ID was found.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or GetElementId for dynamic strings.
ElementData GetElementData(ElementId id) {
    Clay_ElementData elementData = Clay_GetElementData(*reinterpret_cast<Clay_ElementId*>(&id));
    return *reinterpret_cast<ElementData*>(&elementData);
}
// Returns true if the pointer position provided by SetPointerState is within the current element's bounding box.
// Works during element declaration, e.g. CLAY({ .backgroundColor = Hovered() ? BLUE : RED });
bool Hovered(void) {
    return Clay_Hovered();
}

using OnHoverFunction = void (*)(ElementId elementId, PointerData pointerData, intptr_t userData);

// Bind a callback that will be called when the pointer position provided by SetPointerState is within the current element's bounding box.
// - onHoverFunction is a function pointer to a user defined function.
// - userData is a pointer that will be transparently passed through when the onHoverFunction is called.
void OnHover(OnHoverFunction onHoverFunction, intptr_t userData) {
    using ClayOnHoverFunction = void (*)(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);

    Clay_OnHover(
        *reinterpret_cast<ClayOnHoverFunction*>(onHoverFunction),
        userData
    );
}
// An imperative function that returns true if the pointer position provided by SetPointerState is within the element with the provided ID's bounding box.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or GetElementId for dynamic strings.
bool PointerOver(ElementId elementId) {
    return Clay_PointerOver(*reinterpret_cast<Clay_ElementId*>(&elementId));
}
// Returns data representing the state of the scrolling element with the provided ID.
// The returned ScrollContainerData contains a `found` bool that will be true if a scroll element was found with the provided ID.
// An imperative function that returns true if the pointer position provided by SetPointerState is within the element with the provided ID's bounding box.
// This ID can be calculated either with CLAY_ID() for string literal IDs, or GetElementId for dynamic strings.
ScrollContainerData GetScrollContainerData(ElementId id) {
    Clay_ScrollContainerData data = Clay_GetScrollContainerData(*reinterpret_cast<Clay_ElementId*>(&id));
    return *reinterpret_cast<ScrollContainerData*>(&data);
}

using MeasureTextFunction = Dimensions(*)(StringSlice text, TextElementConfig *config, void *userData);

// Binds a callback function that Clay will call to determine the dimensions of a given string slice.
// - measureTextFunction is a user provided function that adheres to the interface Dimensions (StringSlice text, TextElementConfig *config, void *userData);
// - userData is a pointer that will be transparently passed through when the measureTextFunction is called.
void SetMeasureTextFunction(MeasureTextFunction measureTextFunction, void *userData) {
    using ClayMeasureTextFunction = Clay_Dimensions(*)(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);

    Clay_SetMeasureTextFunction(
        *reinterpret_cast<ClayMeasureTextFunction*>(&measureTextFunction),
        userData
    );
}
// Experimental - Used in cases where Clay needs to integrate with a system that manages its own scrolling containers externally.
// Please reach out if you plan to use this function, as it may be subject to change.
void SetQueryScrollOffsetFunction(Vector2 (*queryScrollOffsetFunction)(uint32_t elementId, void *userData), void *userData);
// A bounds-checked "get" function for the RenderCommandArray returned from EndLayout().
RenderCommand* RenderCommandArray_Get(RenderCommandArray* array, int32_t index);
// Enables and disables Clay's internal debug tools.
// This state is retained and does not need to be set each frame.
void SetDebugModeEnabled(bool enabled) {
    Clay_SetDebugModeEnabled(enabled);
}
// Returns true if Clay's internal debug tools are currently enabled.
bool IsDebugModeEnabled(void) {
    return Clay_IsDebugModeEnabled();
}
// Enables and disables visibility culling. By default, Clay will not generate render commands for elements whose bounding box is entirely outside the screen.
void SetCullingEnabled(bool enabled) {
    Clay_SetCullingEnabled(enabled);
}
// Returns the maximum number of UI elements supported by Clay's current configuration.
int32_t GetMaxElementCount(void) {
    return Clay_GetMaxElementCount();
}
// Modifies the maximum number of UI elements supported by Clay's current configuration.
// This may require reallocating additional memory, and re-calling Initialize();
void SetMaxElementCount(int32_t maxElementCount) {
    Clay_SetMaxElementCount(maxElementCount);
}
// Returns the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
int32_t GetMaxMeasureTextCacheWordCount(void) {
    return Clay_GetMaxMeasureTextCacheWordCount();
}
// Modifies the maximum number of measured "words" (whitespace seperated runs of characters) that Clay can store in its internal text measurement cache.
// This may require reallocating additional memory, and re-calling Initialize();
void SetMaxMeasureTextCacheWordCount(int32_t maxMeasureTextCacheWordCount) {
    Clay_SetMaxMeasureTextCacheWordCount(maxMeasureTextCacheWordCount);
}
// Resets Clay's internal text measurement cache, useful if memory to represent strings is being re-used.
// Similar behaviour can be achieved on an individual text element level by using TextElementConfig.hashStringContents
void ResetMeasureTextCache(void) {
    Clay_ResetMeasureTextCache();
}

#endif // CLAY_HEADER

}

/*
LICENSE
zlib/libpng license

Copyright (c) 2024 Nic Barker

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/