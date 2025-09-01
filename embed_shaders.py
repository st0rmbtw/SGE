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

SLANG_FLAGS = ("-matrix-layout-column-major", "-O3", "-line-directive-mode", "none", "-g0")
SPIRV_CROSS_FLAGS = ("--no-es", "--remove-unused-variables", "--no-420pack-extension", "--version", "410")

def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " "
        else:
            return s
    return re.sub(COMMENT_PATTERN, replacer, text)

def write_constant(f, name):
    content = ""
    for line in f.readlines():
        l = comment_remover(line).strip(" \t")
        l = l.replace('SPIRV_Cross_Combined', '').replace('SLANG_ParameterGroup_', '')
        if l == '\n': continue
        content += l
    # content = comment_remover(f.read())
    size = len(content) + 1
    return f"static const char {name}[{size}] = R\"({content})\";\n\n"

def write_bytes(f, name):
    l = list(f.read())
    content = ', '.join(str(x) for x in l)
    size = len(l)
    return f"static const unsigned char {name}[{size}] = {{{content}}};\n\n"

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

def compile_vulkan_shader(executable: str, item_path: Path):
    basename = item_path.stem.upper()
    var_name = f"VULKAN_{basename}"
    flags = ("-target", "spirv") + SLANG_FLAGS
    
    result = ""
    
    fd, path = tempfile.mkstemp(suffix=".spv")

    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "VS") + flags + ("-o", str(path)),
    )
    ps.wait()
    
    with os.fdopen(fd, "rb") as f:
        result += write_bytes(f, f"{var_name}_VERT")
    
    fd, path = tempfile.mkstemp(suffix=".spv")
    
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "PS") + flags + ("-o", str(path)),
    )
    ps.wait()

    with os.fdopen(fd, "rb") as f:
        result += write_bytes(f, f"{var_name}_FRAG")
        
    return result

def compile_d3d_shader(executable: str, item_path: Path):
    basename = item_path.stem.upper()
    var_name = f"D3D11_{basename}"
    flags = ("-target", "hlsl") + SLANG_FLAGS
    
    result = ""

    fd, path = tempfile.mkstemp(suffix=".hlsl")
    
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "VS") + flags + ("-o", str(path)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_VERT")
    
    fd, path = tempfile.mkstemp(suffix=".hlsl")
    
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "PS") + flags + ("-o", str(path)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()

    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_FRAG")
        
    return result

def compile_metal_shader(executable: str, item_path: Path):
    basename = item_path.stem.upper()
    var_name = f"METAL_{basename}"
    flags = ("-target", "metal") + SLANG_FLAGS
    
    result = ""
    
    fd, path = tempfile.mkstemp(suffix=".metal")

    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "VS") + flags + ("-o", str(path)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_VERT")
    
    fd, path = tempfile.mkstemp(suffix=".metal")
    
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "PS") + flags + ("-o", str(path)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()

    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_FRAG")
        
    return result

def compile_opengl_shader(executable: str, item_path: Path):
    basename = item_path.stem.upper()
    var_name = f"GL_{basename}"
    flags = ("-target", "spirv") + SLANG_FLAGS
    
    result = ""
    
    _, path_spv = tempfile.mkstemp(suffix=".spv")
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "VS") + flags + ("-o", str(path_spv)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    fd, path_glsl = tempfile.mkstemp(suffix=".glsl")
    ps = subprocess.Popen(
        ("spirv-cross", str(path_spv), "--stage", "vert") + SPIRV_CROSS_FLAGS + ("--output", str(path_glsl)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_VERT")
    
    _, path_spv = tempfile.mkstemp(suffix=".spv")
    ps = subprocess.Popen(
        (executable, str(item_path), "-entry", "PS") + flags + ("-o", str(path_spv)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    fd, path_glsl = tempfile.mkstemp(suffix=".glsl")
    ps = subprocess.Popen(
        ("spirv-cross", str(path_spv), "--stage", "frag") + SPIRV_CROSS_FLAGS + ("--output", str(path_glsl)),
        stdout=sys.stdout,
        stderr=sys.stderr
    )
    ps.wait()
    
    with os.fdopen(fd, "r") as f:
        result += write_constant(f, f"{var_name}_FRAG")
        
    return result

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
    code += f"case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_{upper}_VERT, sizeof(D3D11_{upper}_VERT), D3D11_{upper}_FRAG, sizeof(D3D11_{upper}_FRAG));\n"
    code += ' ' * 8
    code += f"case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_{upper}_VERT, sizeof(METAL_{upper}_VERT), METAL_{upper}_FRAG, sizeof(METAL_{upper}_FRAG));\n"
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

    shaders_hpp_content = (
        "#ifndef _SGE_RENDERER_SHADERS_HPP_\n"
        "#define _SGE_RENDERER_SHADERS_HPP_\n\n"
        "#include <cstdlib>\n"
        "#include <SGE/types/backend.hpp>\n"
        "#include <SGE/assert.hpp>\n\n"
    )

    shader_names = set()

    for item in sorted(shaders_dir.iterdir()):
        if not item.is_file(): continue
        if item.stem in shader_names: continue
        shader_names.add(item.stem)
        
        item_path = item.resolve()
        
        executable = f"slangc{ext}"
        
        shaders_hpp_content += compile_d3d_shader(executable, item_path)
        shaders_hpp_content += compile_vulkan_shader(executable, item_path)
        shaders_hpp_content += compile_metal_shader(executable, item_path)
        shaders_hpp_content += compile_opengl_shader(executable, item_path)
        
    shaders_hpp_content += SHADER_SOURCE_STRUCTURE_CODE
    shaders_hpp_content += '\n'

    for name in sorted(shader_names):
        shaders_hpp_content += generate_getter_function(name)

    shaders_hpp_content += "#endif"

    with open(shaders_hpp_file, "w") as f:
        f.write(shaders_hpp_content)

if __name__ == "__main__":
    main()
