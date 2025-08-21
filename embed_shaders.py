from pathlib import Path
import os
import sys
import tempfile
import subprocess
import platform
import re

COMMENT_PATTERN = re.compile(
    r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
    re.DOTALL | re.MULTILINE
)

def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " "
        else:
            return s
    return re.sub(COMMENT_PATTERN, replacer, text)

def write_constant(name, path):
    content = ""
    with open(path, "r") as f:
        for line in f.readlines():
            l = comment_remover(line).strip(" \t")
            if l == '\n': continue
            content += l
        # content = comment_remover(f.read())
        size = len(content) + 1
        return f"static const char {name}[{size}] = R\"({content})\";\n\n"

def signed_byte(b):
    return b - 256 if b >= 128 else b

SHADER_SOURCE_STRUCTURE_CODE = """struct ShaderSourceCode {
    ShaderSourceCode(const void* v, size_t vs, const void* f, size_t fs) :
        vs_source(v),
        vs_size(vs),
        fs_source(f),
        fs_size(fs) {}

    const void* vs_source;
    size_t vs_size;

    const void* fs_source;
    size_t fs_size;
};
"""

def generate_getter_function(name):
    upper = name.upper()

    code = ""
    code += f"static inline ShaderSourceCode Get{name.capitalize()}ShaderSourceCode(const sge::RenderBackend backend) {{\n"
    code += ' ' * 4
    code += "switch (backend) {\n"
    code += ' ' * 8
    code += f"case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_{upper}_VERT, sizeof(VULKAN_{upper}_VERT), VULKAN_{upper}_FRAG, sizeof(VULKAN_{upper}_FRAG));\n"
    code += ' ' * 8
    code += "case sge::RenderBackend::D3D11:\n"
    code += ' ' * 8
    code += f"case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_{upper}, sizeof(D3D11_{upper}), D3D11_{upper}, sizeof(D3D11_{upper}));\n"
    code += ' ' * 8
    code += f"case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_{upper}, sizeof(METAL_{upper}), METAL_{upper}, sizeof(METAL_{upper}));\n"
    code += ' ' * 8
    code += f"case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_{upper}_VERT, sizeof(GL_{upper}_VERT), GL_{upper}_FRAG, sizeof(GL_{upper}_FRAG));\n"
    code += ' ' * 8
    code += "default: SGE_UNREACHABLE();\n"
    code += ' ' * 4
    code += '}\n'
    code += '}\n'
    return code

def main():
    cwd = sys.argv[1]
    ext = ""

    if platform.system() == "Windows":
        ext = ".exe"

    renderer_dir = Path(cwd, "src/engine/renderer/")

    if not renderer_dir.exists(): return

    shaders_hpp_file = Path(renderer_dir, "shaders.hpp")

    shaders_dir = Path(renderer_dir, "shaders")
    d3d11_dir = Path(shaders_dir, "d3d11")
    metal_dir = Path(shaders_dir, "metal")
    opengl_dir = Path(shaders_dir, "opengl")
    vulkan_dir = Path(shaders_dir, "vulkan")

    shaders_hpp_content = (
        "#ifndef _SGE_RENDERER_SHADERS_HPP_\n"
        "#define _SGE_RENDERER_SHADERS_HPP_\n\n"
        "#include <cstdlib>\n"
        "#include <SGE/types/backend.hpp>\n"
        "#include <SGE/assert.hpp>\n\n"
    )

    shader_names = set()

    if d3d11_dir.exists():
        for item in sorted(d3d11_dir.iterdir()):
            if not item.is_file(): continue

            shader_names.add(item.stem)

            basename = item.stem.upper()
            var_name = f"D3D11_{basename}"
            shaders_hpp_content += write_constant(var_name, item)

    if metal_dir.exists():
        for item in sorted(metal_dir.iterdir()):
            if not item.is_file(): continue

            shader_names.add(item.stem)

            basename = item.stem.upper()
            var_name = f"METAL_{basename}"
            shaders_hpp_content += write_constant(var_name, item)

    if opengl_dir.exists():
        for item in sorted(opengl_dir.iterdir()):
            if not item.is_file(): continue

            shader_names.add(item.stem)

            basename = item.stem.upper()
            var_name = f"GL_{basename}"

            if item.suffix == ".vert":
                var_name += "_VERT"
            elif item.suffix == ".frag":
                var_name += "_FRAG"
            elif item.suffix == ".comp":
                var_name += "_COMP"

            shaders_hpp_content += write_constant(var_name, item)

    if vulkan_dir.exists():
        for item in sorted(vulkan_dir.iterdir()):
            if not item.is_file(): continue

            shader_names.add(item.stem)

            basename = item.stem.upper()
            var_name = f"VULKAN_{basename}"

            if item.suffix == ".vert":
                var_name += "_VERT"
            elif item.suffix == ".frag":
                var_name += "_FRAG"
            elif item.suffix == ".comp":
                var_name += "_COMP"

            fd, path = tempfile.mkstemp(suffix=".spv")

            ps = subprocess.Popen(
                (f"glslang{ext}", "-V", "--enhanced-msgs", "-o", path, str(item)),
                shell=True
            )
            ps.wait()

            with os.fdopen(fd, "rb") as f:
                l = list(f.read())
                content = ', '.join(str(x) for x in l)
                size = len(l)
                shaders_hpp_content += f"static const unsigned char {var_name}[{size}] = {{{content}}};\n\n"

    shaders_hpp_content += SHADER_SOURCE_STRUCTURE_CODE
    shaders_hpp_content += '\n'

    for name in sorted(shader_names):
        shaders_hpp_content += generate_getter_function(name)

    shaders_hpp_content += "#endif"

    with open(shaders_hpp_file, "w") as f:
        f.write(shaders_hpp_content)

if __name__ == "__main__":
    main()
