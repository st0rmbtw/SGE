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
    )
    
    if d3d11_dir.exists():
        for item in sorted(d3d11_dir.iterdir()):
            if not item.is_file(): continue
            
            basename = item.stem.upper()
            var_name = f"D3D11_{basename}"
            shaders_hpp_content += write_constant(var_name, item)
    
    if metal_dir.exists():
        for item in sorted(metal_dir.iterdir()):
            if not item.is_file(): continue
            
            basename = item.stem.upper()
            var_name = f"METAL_{basename}"
            shaders_hpp_content += write_constant(var_name, item)
    
    if opengl_dir.exists():
        for item in sorted(opengl_dir.iterdir()):
            if not item.is_file(): continue
            
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
            )
            ps.wait()
            
            with os.fdopen(fd, "rb") as f:
                l = list(f.read())
                content = ', '.join(str(x) for x in l)
                size = len(l)
                shaders_hpp_content += f"static const unsigned char {var_name}[{size}] = {{{content}}};\n\n"
            
    shaders_hpp_content += "#endif"
    
    with open(shaders_hpp_file, "w") as f:
        f.write(shaders_hpp_content)

if __name__ == "__main__":
    main()
