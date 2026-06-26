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

COMBINED_SAMPLER_PATTERN = re.compile(
    r'SPIRV_Cross_Combined(?P<name>[a-zA-Z]+)Sampler'
)

SLANG_FLAGS = ("-matrix-layout-column-major", "-O3", "-line-directive-mode", "none", "-g0")
SPIRV_CROSS_FLAGS = ("--no-es", "--remove-unused-variables", "--no-420pack-extension", "--version", "410")

class CompileResults:
    d3d: dict[str, tuple[bool, bool]] = {}
    vk: dict[str, tuple[bool, bool]] = {}
    metal: dict[str, tuple[bool, bool]] = {}
    gl: dict[str, tuple[bool, bool]] = {}

def comment_remover(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " "
        else:
            return s
    return re.sub(COMMENT_PATTERN, replacer, text)

def write_constant(f, name, unmangle=False):
    content = ""
    for line in f.readlines():
        l = comment_remover(line).strip(" \t")
        if l == '\n': continue
        if l.startswith('#'):
            l = l.replace('\n', '\\n')
        else:
            l = l.replace('\n', '')
        l = l.replace('#', '\\n#')
        l = l.replace('"', '\\"')
        l = re.sub(COMBINED_SAMPLER_PATTERN, r"\g<name>", l)
        if unmangle:
            l = l.replace('SLANG_ParameterGroup_', '')
        content += l
    # content = comment_remover(f.read())
    return f"static const char {name}[] = \"{content}\";\n\n"

def write_bytes(f, name):
    l = list(f.read())
    content = ', '.join(str(x) for x in l)
    return f"static const unsigned char {name}[] = {{{content}}};\n\n"

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

def compile_vulkan_shader(executable: str, item_path: Path, flags: tuple[str]) -> tuple[str, bool, bool]:
    basename = item_path.stem.upper()
    var_name = f"VULKAN_{basename}"
    flags = ("-target", "spirv") + flags
    
    result = ""
    has_vertex = False
    has_fragment = False
    
    fd, path = tempfile.mkstemp(suffix=".spv")
    try:
        with os.fdopen(fd, "wb") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "VS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()
        
        if code == 0:
            has_vertex = True
            with open(path, "rb") as f:
                result += write_bytes(f, f"{var_name}_VERT")
    finally:
        os.remove(path)

    fd, path = tempfile.mkstemp(suffix=".spv")
    try:
        with os.fdopen(fd, "wb") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "PS") + flags + ("-o", str(path)),
                stdout=sys.stdout,
                stderr=sys.stderr
            ).wait()
            
        if code == 0:
            has_fragment = True
            with open(path, "rb") as f:
                result += write_bytes(f, f"{var_name}_FRAG")
    finally:
        os.remove(path)
        
    return result, has_vertex, has_fragment

def compile_d3d_shader(executable: str, item_path: Path, flags: tuple[str]) -> tuple[str, bool, bool]:
    basename = item_path.stem.upper()
    var_name = f"D3D11_{basename}"
    flags = ("-target", "hlsl") + flags
    
    result = ""
    has_vertex = False
    has_fragment = False

    fd, path = tempfile.mkstemp(suffix=".hlsl", text=True)
    try:
        with os.fdopen(fd, "w") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "VS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()

        if code == 0:
            has_vertex = True
            with open(path, "r") as f:
                result += write_constant(f, f"{var_name}_VERT")
    finally:
        os.remove(path)
    
    fd, path = tempfile.mkstemp(suffix=".hlsl", text=True)
    try:
        with os.fdopen(fd, "w") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "PS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()
            
        if code == 0:
            has_fragment = True
            with open(path, "r") as f:
                result += write_constant(f, f"{var_name}_FRAG")
    finally:
        os.remove(path)
        
    return result, has_vertex, has_fragment

def compile_metal_shader(executable: str, item_path: Path, flags: tuple[str]) -> tuple[str, bool, bool]:
    basename = item_path.stem.upper()
    var_name = f"METAL_{basename}"
    flags = ("-target", "metal") + flags
    
    result = ""
    has_vertex = False
    has_fragment = False
    
    fd, path = tempfile.mkstemp(suffix=".metal", text=True)
    try:
        with os.fdopen(fd, "w") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "VS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()
            
        if code == 0:
            has_vertex = True
            with open(path, "r") as f:
                result += write_constant(f, f"{var_name}_VERT")
    finally:
        os.remove(path)
    
    fd, path = tempfile.mkstemp(suffix=".metal", text=True)
    try:
        with os.fdopen(fd, "w") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "PS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()
            
        if code == 0:
            has_fragment = True
            with open(path, "r") as f:
                result += write_constant(f, f"{var_name}_FRAG")
    finally:
        os.remove(path)
        
    result = result.replace('[[vertex]]', 'vertex')
    result = result.replace('[[fragment]]', 'fragment')
        
    return result, has_vertex, has_fragment

def compile_opengl_shader(executable: str, item_path: Path, flags: tuple[str]) -> tuple[str, bool, bool]:
    basename = item_path.stem.upper()
    var_name = f"GL_{basename}"
    flags = ("-target", "spirv") + flags
    
    result = ""
    has_vertex = False
    has_fragment = False
    
    fd1, path_spv = tempfile.mkstemp(suffix=".spv")
    fd2, path_glsl = tempfile.mkstemp(suffix=".glsl", text=True)
    try:
        with os.fdopen(fd1, "wb") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "VS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()
            
        if code == 0:
            has_vertex = True
            with os.fdopen(fd2, "w") as f:
                subprocess.Popen(
                    ("spirv-cross", str(path_spv), "--stage", "vert") + SPIRV_CROSS_FLAGS,
                    stdout=f,
                    stderr=sys.stderr
                ).wait()
            
            with open(path_glsl, "r") as f:
                result += write_constant(f, f"{var_name}_VERT", True)
    finally:
        try:
            os.close(fd1)
        except:
            pass
        try:
            os.close(fd2)
        except:
            pass
        
        os.remove(path_glsl)
        os.remove(path_spv)
    
    fd1, path_spv = tempfile.mkstemp(suffix=".spv")
    fd2, path_glsl = tempfile.mkstemp(suffix=".glsl", text=True)
    try:
        with os.fdopen(fd1, "wb") as f:
            code = subprocess.Popen(
                (executable, str(item_path), "-entry", "PS") + flags,
                stdout=f,
                stderr=sys.stderr
            ).wait()

        if code == 0:
            has_fragment = True
            with os.fdopen(fd2, "w") as f:
                subprocess.Popen(("spirv-cross", str(path_spv), "--stage", "frag", "--rename-interface-variable", "out", "0", "fragColor") + SPIRV_CROSS_FLAGS,
                    stdout=f,
                    stderr=sys.stderr
                ).wait()
            
            with open(path_glsl, "r") as f:
                result += write_constant(f, f"{var_name}_FRAG", True)
    finally:
        try:
            os.close(fd1)
        except:
            pass
        try:
            os.close(fd2)
        except:
            pass
        
        os.remove(path_spv)
        os.remove(path_glsl)
        
    return result, has_vertex, has_fragment

def snake_to_pascal(snake_str):
    return "".join(word.capitalize() for word in snake_str.split("_"))

def generate_getter_function(name, results: CompileResults):
    upper = name.upper()
    
    d3d_has_vertex, d3d_has_fragment = results.d3d[name]
    vk_has_vertex, vk_has_fragment = results.vk[name]
    metal_has_vertex, metal_has_fragment = results.metal[name]
    gl_has_vertex, gl_has_fragment = results.gl[name]

    code = ""
    code += f"static inline ShaderSourceCode Get{snake_to_pascal(name)}ShaderSourceCode(const sge::RenderBackend backend) {{\n"
    code += ' ' * 4
    code += "switch (backend) {\n"
    code += ' ' * 8
    
    code += "case sge::RenderBackend::Vulkan: return ShaderSourceCode("
    if vk_has_vertex:
        code += f"VULKAN_{upper}_VERT, sizeof(VULKAN_{upper}_VERT)"
    else:
        code += "nullptr, 0"
    code += ', '
    if vk_has_fragment:
        code += f"VULKAN_{upper}_FRAG, sizeof(VULKAN_{upper}_FRAG)"
    else:
        code += "nullptr, 0"
    code += ');\n'
    
    code += ' ' * 8
    code += "case sge::RenderBackend::D3D11:\n"
    code += ' ' * 8
    code += "case sge::RenderBackend::D3D12: return ShaderSourceCode("
    
    if d3d_has_vertex:
        code += f"D3D11_{upper}_VERT, sizeof(D3D11_{upper}_VERT)"
    else:
        code += "nullptr, 0"
    code += ', '
    if d3d_has_fragment:
        code += f"D3D11_{upper}_FRAG, sizeof(D3D11_{upper}_FRAG)"
    else:
        code += "nullptr, 0"
    code += ');\n'
    
    code += ' ' * 8
    code += "case sge::RenderBackend::Metal: return ShaderSourceCode("

    if metal_has_vertex:
        code += f"METAL_{upper}_VERT, sizeof(METAL_{upper}_VERT)"
    else:
        code += "nullptr, 0"
    code += ', '
    if metal_has_fragment:
        code += f"METAL_{upper}_FRAG, sizeof(METAL_{upper}_FRAG)"
    else:
        code += "nullptr, 0"
    code += ');\n'
    
    code += ' ' * 8
    code += "case sge::RenderBackend::OpenGL: return ShaderSourceCode("
    
    if gl_has_vertex:
        code += f"GL_{upper}_VERT, sizeof(GL_{upper}_VERT)"
    else:
        code += "nullptr, 0"
    code += ', '
    if gl_has_fragment:
        code += f"GL_{upper}_FRAG, sizeof(GL_{upper}_FRAG)"
    else:
        code += "nullptr, 0"
    code += ');\n'
    
    code += ' ' * 8
    code += "default: SGE_UNREACHABLE();\n"
    code += ' ' * 4
    code += '}\n'
    code += '}\n'
    return code

def main():
    shader_dir_path = sys.argv[1]
    output_file_path = sys.argv[2]
    guard_name = sys.argv[3]
    ext = ""

    compile_d3d = True
    compile_vulkan = True
    compile_metal = True
    compile_gl = True
    
    for arg in sys.argv[2:]:
        if arg == "-d3d" or arg == "-vulkan" or arg == "-metal" or arg == "-gl":
            compile_d3d = False
            compile_vulkan = False
            compile_metal = False
            compile_gl = False
            break
        
    for arg in sys.argv[2:]:
        if arg == "-d3d":
            compile_d3d = True
        elif arg == "-vulkan":
            compile_vulkan = True
        elif arg == "-metal":
            compile_metal = True
        elif arg == "-gl":
            compile_gl = True

    if platform.system() == "Windows":
        ext = ".exe"

    shaders_dir = Path(shader_dir_path)
    output_file = Path(output_file_path)

    if not shaders_dir.exists():
        print(f"Directory '{shader_dir_path}' doesn't exist")
        return

    shaders_hpp_content = (
        f"#ifndef {guard_name}\n"
        f"#define {guard_name}\n\n"
        "#include <cstdlib>\n\n"
        "#include <SGE/assert.hpp>\n"
        "#include <SGE/types/backend.hpp>\n\n"
    )

    shader_names = set()

    slang_executable = f"slangc{ext}"
    slang_flags = SLANG_FLAGS + ("-I", str(shaders_dir))
    
    results: CompileResults = CompileResults()
    
    for item in sorted(shaders_dir.iterdir()):
        if not item.is_file(): continue
        if item.name == "common.slang": continue
        if item.stem in shader_names: continue
        shader_names.add(item.stem)
        
        item_path = item.resolve()
        
        if compile_d3d:
            print(f"Compiling {item} for D3D11 ...")
            result, has_vertex, has_fragment = compile_d3d_shader(slang_executable, item_path, slang_flags)
            shaders_hpp_content += result
            results.d3d[item.stem] = (has_vertex, has_fragment)
            
        if compile_vulkan:
            print(f"Compiling {item} for Vulkan ...")
            result, has_vertex, has_fragment = compile_vulkan_shader(slang_executable, item_path, slang_flags)
            shaders_hpp_content += result
            results.vk[item.stem] = (has_vertex, has_fragment)
            
        if compile_metal:
            print(f"Compiling {item} for Metal ...")
            result, has_vertex, has_fragment = compile_metal_shader(slang_executable, item_path, slang_flags)
            shaders_hpp_content += result
            results.metal[item.stem] = (has_vertex, has_fragment)
            
        if compile_gl:
            print(f"Compiling {item} for OpenGL ...")
            result, has_vertex, has_fragment = compile_opengl_shader(slang_executable, item_path, slang_flags)
            shaders_hpp_content += result
            results.gl[item.stem] = (has_vertex, has_fragment)
        
    shaders_hpp_content += SHADER_SOURCE_STRUCTURE_CODE
    shaders_hpp_content += '\n'

    for name in sorted(shader_names):
        shaders_hpp_content += generate_getter_function(name, results)

    shaders_hpp_content += f"#endif // {guard_name}"

    with open(output_file, "w") as f:
        f.write(shaders_hpp_content)

if __name__ == "__main__":
    main()
