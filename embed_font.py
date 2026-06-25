import sys
import struct
from pathlib import Path
from dataclasses import dataclass
from typing import Any
import freetype

FONT_SIZE = 68
PADDING = 4
TEXTURE_WIDTH = 1512

@dataclass
class GlyphInfo:
    character: Any
    buffer: bytearray
    bitmap_width: int
    bitmap_rows: int
    bitmap_left: int
    bitmap_top: int
    advance_x: int
    col: int
    row: int
    
def bytes_as_str(b: bytearray):
    return ','.join(str(x) for x in b)

def main():
    if len(sys.argv) < 4:
        print(f"Usage: {sys.argv[0]} <font_file> <output_file> <guard_name>")
        return
    
    font_file_path_str = sys.argv[1]
    output_file_path_str = sys.argv[2]
    guard_name = sys.argv[3]
    
    output_file_path = Path(output_file_path_str)
    
    font_file_path = Path(font_file_path_str)
    if not font_file_path.exists():
        print(f"[ERROR] File '{font_file_path}' doesn't exist.")
        return
    
    face = freetype.Face(font_file_path_str)
    face.set_pixel_sizes(0, FONT_SIZE)
    
    col = PADDING
    row = PADDING
    
    max_height = 0
    max_ascent = float("-inf")
    max_descent = float("-inf")
    
    (character, index) = face.get_first_char()
    
    glyphs: list[GlyphInfo] = []
    
    while True:
        face.load_char(character, freetype.FT_LOAD_DEFAULT)
        face.glyph.render(freetype.FT_RENDER_MODE_SDF)
        
        if len(face.glyph.bitmap.buffer) > 0:
            if (col + face.glyph.bitmap.width + PADDING >= TEXTURE_WIDTH):
                col = PADDING
                row += max_height + PADDING
                max_height = 0
                
            max_height = max(max_height, face.glyph.bitmap.rows)
            
            buffer = bytearray(face.glyph.bitmap.buffer)
            
            info = GlyphInfo(character, buffer, face.glyph.bitmap.width, face.glyph.bitmap.rows, face.glyph.bitmap_left, face.glyph.bitmap_top, face.glyph.advance.x, col, row)
            glyphs.append(info)
            
            max_ascent = max(max_ascent, info.bitmap_top)
            max_descent = max(max_descent, info.bitmap_rows - info.bitmap_top)
            
            col += face.glyph.bitmap.width
        
        (character, index) = face.get_next_char(character, index)
        if not index:
            break
    
    texture_height = row + max_height
    texture_data = bytearray(TEXTURE_WIDTH * texture_height)
    
    for info in glyphs:
        for y in range(info.bitmap_rows):
            for x in range(info.bitmap_width):
                texture_data[(info.row + y) * TEXTURE_WIDTH + info.col + x] = info.buffer[y * info.bitmap_width + x]
        
    # from PIL import Image
    # image = Image.frombytes("L", (TEXTURE_WIDTH, texture_height), texture_data)
    # image.show()
    
    with open(output_file_path, "w") as f:
        f.write(f"#ifndef {guard_name}\n")
        f.write(f"#define {guard_name}\n")
        f.write("#include <glm/vec2.hpp>\n")
        f.write("struct EmbeddedFont { float font_size; float max_ascent; float max_descent; int16_t ascender; uint32_t texture_width; uint32_t texture_height; };\n")
        f.write("struct EmbeddedFontGlyph { glm::ivec2 size; glm::vec2 tex_size; glm::ivec2 bearing; int64_t advance; glm::vec2 texture_coords; uint32_t character; };\n")
        f.write("static EmbeddedFont FONT_META_DATA = {\n")
        f.write(f"    .font_size = {FONT_SIZE},\n")
        f.write(f"    .max_ascent = {max_ascent},\n")
        f.write(f"    .max_descent = {max_descent},\n")
        f.write(f"    .ascender = {face.ascender},\n")
        f.write(f"    .texture_width = {TEXTURE_WIDTH},\n")
        f.write(f"    .texture_height = {texture_height}\n")
        f.write("};\n")
        f.write("static EmbeddedFontGlyph FONT_GLYPHS[] = {\n")
        for info in glyphs:
            tex_width = info.bitmap_width / TEXTURE_WIDTH
            tex_height = info.bitmap_rows / texture_height
            texture_coord_x = info.col / TEXTURE_WIDTH
            texture_coord_y = info.row / texture_height
            f.write("    EmbeddedFontGlyph { ")
            f.write(f".size = glm::ivec2({info.bitmap_width}, {info.bitmap_rows}), ")
            f.write(f".tex_size = glm::vec2({tex_width}, {tex_height}), ")
            f.write(f".bearing = glm::ivec2({info.bitmap_left}, {info.bitmap_top}), ")
            f.write(f".advance = {info.advance_x}, ")
            f.write(f".texture_coords = glm::vec2({texture_coord_x}, {texture_coord_y}), ")
            f.write(f".character = {info.character} ")
            f.write("},\n")
        f.write('\n')
        f.write("};\n")
        
        f.write(f"static uint8_t FONT_TEXTURE_DATA[] = {{ {bytes_as_str(texture_data)} }};\n")
            
        f.write(f"#endif // {guard_name}")

if __name__ == "__main__":
    main()