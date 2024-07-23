#!/usr/bin/env python3

import os
import subprocess

SHADER_DIR = 'src/rendering/pipeline/shaders'


class Shader:
    name: str
    path: str
    vertex_code: list[str]
    fragment_code: list[str]
    compute_code: list[str]
    is_compute: bool = False


def find_files(dir: str) -> list[str]:
    files: list[str] = []

    for (dirpath, _, filenames) in os.walk(dir):
        for filename in filenames:
            files.append(dirpath + "/" + filename)

    return files


def compile_shader(path: str) -> list[str]:
    out_file = f'{path}.u32'
    result = subprocess.run(["glslc", "-c", "-mfmt=num", path, "-o", out_file])

    if result.returncode != 0:
        return []

    file = open(out_file, 'r')

    txt: str = ''
    line = file.readline()
    while line != '':
        txt += line.replace('\n', '').replace('\t', '')
        line = file.readline()

    file.close()
    os.remove(out_file)

    return txt.split(',')


def generate_header(shader: Shader):
    define: str = shader.name.upper() + '_SHADER_GEN_H'
    class_name: str = shader.name.title() + 'Shader'
    class_name = class_name.replace("_", "")

    body_parts: list[str] = []

    if shader.is_compute:
        compute_code: str = ','.join(shader.compute_code)
        body_parts = [
            f"const uint32_t computeCode[] = {{{compute_code}}};",
            "_setupCompute(computeCode, sizeof(computeCode));",
        ]

    else:
        vertex_code: str = ','.join(shader.vertex_code)
        fragment_code: str = ','.join(shader.fragment_code)
        body_parts = [
            f"const uint32_t vertexCode[] = {{{vertex_code}}};",
            f"const uint32_t fragmentCode[] = {{{fragment_code}}};",
            "_setupGraphics(vertexCode, sizeof(vertexCode), fragmentCode, sizeof(fragmentCode));",
        ]

    body: str = '\n\t\t'.join(body_parts)

    header = f"""// THIS FILE IS GENERATED; DO NOT EDIT!

#ifndef {define}
#define {define}

#include "../shader.h"

class {class_name} : public Shader {{
public:
    {class_name}() {{
        {body}
    }}
}};

#endif // !{define}
"""

    path, _ = os.path.splitext(shader.path)

    file = open(f'{path}.gen.h', 'w')
    file.write(header)
    file.close()


def generate_shaders():
    names: list[str] = []
    data: list[Shader] = []

    for file in find_files(SHADER_DIR):
        filename = file.split('/')[-1]
        name, ext = os.path.splitext(filename)

        if ext != ".vert" and ext != ".frag" and ext != ".comp":
            continue

        if name not in names:
            names.append(name)

            shader = Shader()
            shader.name = name
            shader.path = file
            data.append(shader)

        idx: int = names.index(name)

        spirv = compile_shader(f'{file}')

        if ext == ".vert":
            data[idx].vertex_code = spirv
        elif ext == ".frag":
            data[idx].fragment_code = spirv
        else:
            data[idx].is_compute = True
            data[idx].compute_code = spirv

    for shader in data:
        generate_header(shader)


generate_shaders()
