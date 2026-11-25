#ifndef SGE_RENDERER_DRAW_HPP_
#define SGE_RENDERER_DRAW_HPP_

#include <SGE/renderer/batch.hpp>
#include <SGE/utils/frame_arena.hpp>

namespace sge {

namespace Draw {
    inline DrawCall<const Sprite&> Sprite(const Sprite& sprite) {
        return DrawCall<const class Sprite&>(sprite);
    }

    inline DrawCall<const TextureAtlasSprite&> AtlasSprite(const TextureAtlasSprite& sprite) {
        return DrawCall<const TextureAtlasSprite&>(sprite);
    }

    inline DrawCall<ShapeDrawData> Rect(glm::vec2 position, const ShapeRect& rect) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = rect.size,
            .color = rect.color,
            .borderColor = rect.borderColor,
            .borderThickness = rect.borderThickness,
            .borderRadius = rect.borderRadius,
            .anchor = rect.anchor,
            .type = Shape::Rect
        });
    }

    inline DrawCall<ShapeDrawData> Circle(glm::vec2 position, const ShapeCircle& circle) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = glm::vec2(circle.radius * 2.0f),
            .color = circle.color,
            .borderColor = circle.borderColor,
            .borderThickness = circle.borderThickness,
            .borderRadius = BorderRadius::Absolute(0.0),
            .anchor = circle.anchor,
            .type = Shape::Circle
        });
    }

    inline DrawCall<ShapeDrawData> Arc(glm::vec2 position, const ShapeArc& arc) {
        return DrawCall(ShapeDrawData {
            .position = position,
            .size = glm::vec2(arc.outerRadius * 2.0f),
            .color = arc.color,
            .borderColor = sge::color::TRANSPARENT,
            .borderThickness = arc.innerRadius,
            .borderRadius = BorderRadius::Absolute(arc.startAngle, arc.endAngle, 0.0f, 0.0f),
            .anchor = arc.anchor,
            .type = Shape::Arc
        });
    }

    inline DrawCall<LineDrawData> Line(glm::vec2 start, glm::vec2 end, float thickness, const sge::LinearRgba& color, BorderRadius borderRadius = BorderRadius()) {
        return DrawCall(LineDrawData {
            .start = start,
            .end = end,
            .thickness = thickness,
            .color = color,
            .borderRadius = borderRadius
        });
    }

    template<size_t Size>
    inline DrawCall<SdfTextDrawData> TextSdf(sge::RichText<Size> text, glm::vec2 position, sge::TextAlignment alignment, const sge::Font& font) {
        auto copied = sge::FrameArena::Copy(text);
        return DrawCall(SdfTextDrawData {
            .sections = std::span(copied->data(), copied->size()),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }

    inline DrawCall<SdfTextDrawData> TextSdf(std::string_view text, float size, sge::LinearRgba color, glm::vec2 position, sge::TextAlignment alignment, const sge::Font& font) {
        auto section = sge::FrameArena::Allocate<RichTextSection>();
        section->text = text;
        section->color = color;
        section->size = size;

        return DrawCall(SdfTextDrawData {
            .sections = std::span(section, 1),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }

    template<size_t Size>
    inline DrawCall<VectorTextDrawData> TextVector(sge::RichText<Size> text, glm::vec2 position, sge::TextAlignment alignment, const sge::FontVector& font) {
        return DrawCall(VectorTextDrawData {
            .sections = std::span(text.data(), text.size()),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }

    inline DrawCall<VectorTextDrawData> TextVector(std::string_view text, float size, sge::LinearRgba color, glm::vec2 position, sge::TextAlignment alignment, const sge::FontVector& font) {
        auto section = sge::FrameArena::Allocate<RichTextSection>();
        section->text = text;
        section->color = color;
        section->size = size;

        return DrawCall(VectorTextDrawData {
            .sections = std::span(section, 1),
            .font = font,
            .position = position,
            .alignment = alignment
        });
    }
};

}

#endif // SGE_RENDERER_DRAW_HPP_