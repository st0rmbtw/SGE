import os
import platform
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from threading import Lock, Thread
from typing import Callable

COMMENT_PATTERN = re.compile(
    r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
    re.DOTALL | re.MULTILINE
)

COMBINED_SAMPLER_PATTERN = re.compile(
    r'SPIRV_Cross_Combined(?P<name>[a-zA-Z]+)Sampler'
)

SLANG_BASE_FLAGS = ("-matrix-layout-column-major", "-line-directive-mode", "none")
SLANG_DEBUG_FLAGS = ("-g3",)
SLANG_RELEASE_FLAGS = ("-O3",)
SPIRV_CROSS_FLAGS = ("--no-es", "--remove-unused-variables", "--version", "430")
FXC_DEBUG_FLAGS = ("/Zi", "/Od", "/Zss", "/Fd", "./shader-debug-info/")
FXC_RELEASE_FLAGS = ("/O2",)

COMPILED_SHADERS: set[str] = set()
COMPILED_SHADERS_MUTEX = Lock()

FXC_PATH = None

DEBUG_MODE = False

EXE_EXTENSION = ".exe" if platform.system() == "Windows" else ""
SLANG_EXECUTABLE = f"slangc{EXE_EXTENSION}"

def find_win_sdk() -> Path | None:
    kits: list[Path] = []
    for kit in Path("C:\\Program Files (x86)\\Windows Kits\\10\\bin\\").iterdir():
        # skip dirs with letters in the name
        if any(char.isalpha() for char in kit.stem):
            continue
        kits.append(kit)
    if len(kits) == 0:
        return None
    kits.sort()
    return kits[-1].joinpath('x64')

@dataclass
class ShaderStages:
    vertex: bool = False
    fragment: bool = False

    @property
    def has_any(self) -> bool:
        return self.vertex or self.fragment

class ShaderStageMap:
    d3d: dict[str, ShaderStages]
    vk: dict[str, ShaderStages]
    metal: dict[str, ShaderStages]
    gl: dict[str, ShaderStages]

    def __init__(self) -> None:
        self.d3d = {}
        self.vk = {}
        self.metal = {}
        self.gl = {}

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

SHADER_SOURCE_STRUCTURE_NAME = "EmbeddedShaderSource"
SHADER_SOURCE_STRUCTURE_CODE = f"""struct {SHADER_SOURCE_STRUCTURE_NAME} {{
    sge::ShaderSource vertex;
    sge::ShaderSource fragment;
}};
"""

def compile_vulkan_shader(basename: str, path: str, flags: tuple[str]) -> tuple[str, ShaderStages]:
    var_name = f"VULKAN_{basename.upper()}"
    flags = ("-target", "spirv") + flags

    result = ""
    stages = ShaderStages()

    with COMPILED_SHADERS_MUTEX:
        slang_stderr = subprocess.DEVNULL if path in COMPILED_SHADERS else sys.stderr
        COMPILED_SHADERS.add(path)

    with tempfile.NamedTemporaryFile("wb", delete=True, delete_on_close=False, suffix=".spv") as out_file:
        out_file.close()

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "VS") + flags + ("-o", str(out_file.name)),
            stdout=subprocess.DEVNULL,
            stderr=slang_stderr
        ).wait()

        if code == 0:
            stages.vertex = True
            with open(out_file.name, "rb") as f:
                result += write_bytes(f, f"{var_name}_VERT")

    with tempfile.NamedTemporaryFile("wb", delete=True, delete_on_close=False, suffix=".spv") as out_file:
        out_file.close()

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "PS") + flags + ("-o", str(out_file.name)),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        ).wait()

        if code == 0:
            stages.fragment = True
            with open(out_file.name, "rb") as f:
                result += write_bytes(f, f"{var_name}_FRAG")

    return result, stages

def compile_d3d_shader(basename: str, path: str, base_flags: tuple[str]) -> tuple[str, ShaderStages]:
    var_name = f"D3D11_{basename.upper()}"
    flags = ("-target", "hlsl") + base_flags

    result = ""
    stages = ShaderStages()

    fxc_base_flags = FXC_DEBUG_FLAGS if DEBUG_MODE else FXC_RELEASE_FLAGS

    with COMPILED_SHADERS_MUTEX:
        slang_stderr = subprocess.DEVNULL if path in COMPILED_SHADERS else sys.stderr
        COMPILED_SHADERS.add(path)

    with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".hlsl") as out_hlsl:
        out_hlsl.close()

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "VS") + flags  + ("-o", str(out_hlsl.name)),
            stdout=subprocess.DEVNULL,
            stderr=slang_stderr
        ).wait()

        if code == 0:
            stages.vertex = True
            with tempfile.NamedTemporaryFile('wb', delete=True, delete_on_close=False, suffix='.cso') as out_bin:
                out_bin.close()
                subprocess.Popen(
                    (str(FXC_PATH),) + fxc_base_flags + ("/E", "VS", "/T", "vs_5_0", "/Fo", out_bin.name, out_hlsl.name),
                    stdout=subprocess.DEVNULL,
                    stderr=sys.stderr
                ).wait()
                with open(out_bin.name, "rb") as f:
                    result += write_bytes(f, f"{var_name}_VERT")

    with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".hlsl") as out_hlsl:
        out_hlsl.close()

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "PS") + flags + ("-o", str(out_hlsl.name)),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        ).wait()

        if code == 0:
            stages.fragment = True
            with tempfile.NamedTemporaryFile('wb', delete=True, delete_on_close=False, suffix='.cso') as out_bin:
                out_bin.close()
                subprocess.Popen(
                    (str(FXC_PATH),) + fxc_base_flags + ("/E", "PS", "/T", "ps_5_0", "/Fo", out_bin.name, out_hlsl.name),
                    stdout=subprocess.DEVNULL,
                    stderr=sys.stderr
                ).wait()
                with open(out_bin.name, "rb") as f:
                    result += write_bytes(f, f"{var_name}_FRAG")

    return result, stages

def compile_metal_shader(basename: str, path: str, flags: tuple[str]) -> tuple[str, ShaderStages]:
    var_name = f"METAL_{basename.upper()}"
    flags = ("-target", "metal") + flags

    result = ""
    stages = ShaderStages()

    with COMPILED_SHADERS_MUTEX:
        slang_stderr = subprocess.DEVNULL if path in COMPILED_SHADERS else sys.stderr
        COMPILED_SHADERS.add(path)

    with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".metal") as out_metal:
        out_metal.close();

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "VS") + flags + ("-o", str(out_metal.name)),
            stdout=subprocess.DEVNULL,
            stderr=slang_stderr
        ).wait()

        if code == 0:
            stages.vertex = True
            with open(out_metal.name, 'r') as f:
                result += write_constant(f, f"{var_name}_VERT")

    with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".metal") as out_metal:
        out_metal.close()

        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "PS") + flags + ("-o", str(out_metal.name)),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        ).wait()

        if code == 0:
            stages.fragment = True
            with open(out_metal.name, 'r') as f:
                result += write_constant(f, f"{var_name}_FRAG")

    result = result.replace('[[vertex]]', 'vertex')
    result = result.replace('[[fragment]]', 'fragment')

    return result, stages

def compile_gl_shader(basename: str, path: str, flags: tuple[str]) -> tuple[str, ShaderStages]:
    var_name = f"GL_{basename.upper()}"
    flags = ("-target", "spirv") + flags

    result = ""
    stages = ShaderStages()

    with COMPILED_SHADERS_MUTEX:
        slang_stderr = subprocess.DEVNULL if path in COMPILED_SHADERS else sys.stderr
        COMPILED_SHADERS.add(path)

    with tempfile.NamedTemporaryFile("wb", delete=True, delete_on_close=False, suffix=".spv") as out_spv:
        out_spv.close()
        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "VS") + ("-o", str(out_spv.name)),
            stdout=subprocess.DEVNULL,
            stderr=slang_stderr
        ).wait()

        if code == 0:
            stages.vertex = True
            with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".glsl") as out_glsl:
                subprocess.Popen(
                    ("spirv-cross", str(out_spv.name), "--stage", "vert") + SPIRV_CROSS_FLAGS,
                    stdout=out_glsl,
                    stderr=sys.stderr
                ).wait()

                out_glsl.close()

                with open(out_glsl.name, "r") as f:
                    result += write_constant(f, f"{var_name}_VERT", True)

    with tempfile.NamedTemporaryFile("wb", delete=True, delete_on_close=False, suffix=".spv") as out_spv:
        out_spv.close()
        code = subprocess.Popen(
            (SLANG_EXECUTABLE, path, "-entry", "PS") + flags + ("-o", str(out_spv.name)),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        ).wait()

        if code == 0:
            stages.fragment = True
            with tempfile.NamedTemporaryFile("w", delete=True, delete_on_close=False, suffix=".glsl") as out_glsl:
                subprocess.Popen(("spirv-cross", str(out_spv.name), "--stage", "frag", "--rename-interface-variable", "out", "0", "fragColor") + SPIRV_CROSS_FLAGS,
                    stdout=out_glsl,
                    stderr=sys.stderr
                ).wait()

                out_glsl.close()

                with open(out_glsl.name, "r") as f_glsl:
                    result += write_constant(f_glsl, f"{var_name}_FRAG", True)

    return result, stages

def snake_to_pascal(snake_str):
    return "".join(word.capitalize() for word in snake_str.split("_"))

def generate_getter_function(name, stages: ShaderStageMap) -> str | None:
    upper = name.upper()

    d3d_stages = stages.d3d.get(name, ShaderStages())
    vk_stages = stages.vk.get(name, ShaderStages())
    metal_stages = stages.metal.get(name, ShaderStages())
    gl_stages = stages.gl.get(name, ShaderStages())

    if not any((
        d3d_stages.has_any,
        vk_stages.has_any,
        metal_stages.has_any,
        gl_stages.has_any
    )): return

    def get_swith_case_code(stages, backend, source_type, vertex_name, fragment_name) -> str:
        code = ' ' * 8
        code += f"case sge::RenderBackend::{backend}: return {SHADER_SOURCE_STRUCTURE_NAME}{{"
        code += ".vertex="
        if stages.vertex:
            code += f"{{sge::as_bytes({vertex_name}), LLGL::ShaderSourceType::{source_type}}}"
        else:
            code += "{}"
        code += ', '
        code += ".fragment="
        if stages.fragment:
            code += f"{{sge::as_bytes({fragment_name}), LLGL::ShaderSourceType::{source_type}}}"
        else:
            code += "{}"
        code += '};\n'
        return code

    code = ""
    code += f"static inline constexpr {SHADER_SOURCE_STRUCTURE_NAME} Get{snake_to_pascal(name)}ShaderSourceCode(const sge::RenderBackend backend) {{\n"
    code += ' ' * 4
    code += "switch (backend) {\n"

    code += get_swith_case_code(vk_stages, "Vulkan", "BinaryBuffer", f"VULKAN_{upper}_VERT", f"VULKAN_{upper}_FRAG")

    code += ' ' * 8
    code += "case sge::RenderBackend::D3D12:\n"
    code += get_swith_case_code(d3d_stages, "D3D11", "BinaryBuffer", f"D3D11_{upper}_VERT", f"D3D11_{upper}_FRAG")

    code += get_swith_case_code(metal_stages, "Metal", "CodeString", f"METAL_{upper}_VERT", f"METAL_{upper}_FRAG")
    code += get_swith_case_code(gl_stages, "OpenGL", "CodeString", f"GL_{upper}_VERT", f"GL_{upper}_FRAG")

    code += ' ' * 8
    code += "default: SGE_UNREACHABLE();\n"
    code += ' ' * 4
    code += '}\n'
    code += '}\n'
    return code

mutex = Lock()

CompileFnType = Callable[[str, str, tuple], tuple[str, ShaderStages]]

def main():
    global DEBUG_MODE

    shader_dir_path = sys.argv[1]
    output_file_path = sys.argv[2]
    guard_name = sys.argv[3]

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
        elif arg == "-debug":
            DEBUG_MODE = True

    shaders_dir = Path(shader_dir_path)
    output_file = Path(output_file_path)

    if not shaders_dir.exists():
        print(f"Directory '{shader_dir_path}' doesn't exist")
        return

    if DEBUG_MODE:
        for item in Path('./shader-debug-info/').iterdir():
            if item.is_file(): item.unlink()
        os.makedirs("./shader-debug-info/", exist_ok=True)

    if compile_d3d:
        global FXC_PATH
        sdk_path = find_win_sdk()
        if sdk_path is None:
            print("[ERROR] No Windows SDK was found!")
            return
        FXC_PATH = sdk_path.joinpath('fxc.exe')
        print(f"[INFO] Found FXC: {FXC_PATH}")

    shaders_hpp_content = (
        f"#ifndef {guard_name}\n"
        f"#define {guard_name}\n\n"
        "#include <SGE/assert.hpp>\n"
        "#include <SGE/types/backend.hpp>\n"
        "#include <SGE/types/shader_source.hpp>\n"
        "#include <SGE/utils/data.hpp>\n\n"
    )

    shader_names = set()

    if DEBUG_MODE:
        slang_flags = SLANG_BASE_FLAGS + ("-I", str(shaders_dir))
    else:
        slang_flags = SLANG_BASE_FLAGS + SLANG_RELEASE_FLAGS + ("-I", str(shaders_dir))

    stages_map: ShaderStageMap = ShaderStageMap()
    results: list[tuple[int, str]] = []

    tasks: list[Thread] = []

    i = 0

    for item in sorted(shaders_dir.iterdir()):
        if not item.is_file(): continue
        if item.name == "common.slang": continue
        if item.stem in shader_names: continue
        shader_names.add(item.stem)

        item_path = item.resolve()

        def process(compile_fn: CompileFnType, stages_map, index, item_path, slang_flags):
            nonlocal results
            result, stages = compile_fn(item_path.stem, str(item_path), slang_flags)
            with mutex:
                results.append((index, result))
                stages_map[item_path.stem] = stages

        if compile_d3d:
            print(f"Compiling {item} for D3D11 ...")
            t = Thread(target=process, args=(compile_d3d_shader, stages_map.d3d, i, item_path, slang_flags))
            t.start()
            tasks.append(t)
            i += 1

        if compile_vulkan:
            print(f"Compiling {item} for Vulkan ...")
            t = Thread(target=process, args=(compile_vulkan_shader, stages_map.vk, i+1000, item_path, slang_flags))
            t.start()
            tasks.append(t)
            i += 1

        if compile_metal:
            print(f"Compiling {item} for Metal ...")
            t = Thread(target=process, args=(compile_metal_shader, stages_map.metal, i+2000, item_path, slang_flags))
            t.start()
            tasks.append(t)
            i += 1

        if compile_gl:
            print(f"Compiling {item} for OpenGL ...")
            t = Thread(target=process, args=(compile_gl_shader, stages_map.gl, i+3000, item_path, slang_flags))
            t.start()
            tasks.append(t)
            i += 1

    for task in tasks:
        task.join()

    results.sort(key=lambda r: r[0])
    for _, result in results:
        shaders_hpp_content += result

    shaders_hpp_content += SHADER_SOURCE_STRUCTURE_CODE
    shaders_hpp_content += '\n'

    for name in sorted(shader_names):
        function_code = generate_getter_function(name, stages_map)
        if function_code:
            shaders_hpp_content += function_code

    shaders_hpp_content += f"#endif // {guard_name}"

    with open(output_file, "w") as f:
        f.write(shaders_hpp_content)

if __name__ == "__main__":
    main()
