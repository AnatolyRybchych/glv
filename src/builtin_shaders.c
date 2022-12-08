#include "_glv.h"

const char *draw_texture_vert = 
"#version 110\n"
"\n"
"attribute vec4 vertex_p\n;"
"attribute vec2 tex_coords;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 tex_mvp;\n"
"\n"
"varying vec2 tex_uv;\n"
"\n"
"void main(){\n"
"    tex_uv = (vec4(tex_coords, 1, 1) * tex_mvp).xy;\n"
"    gl_Position = vertex_p * mvp;\n"
"}\n";

const char *draw_texture_frag =
"#version 110\n"
"uniform sampler2D tex;\n"
"varying vec2 tex_uv;\n"
"\n"
"void main(){\n"
"    gl_FragColor = texture2D(tex, tex_uv);\n"
"}\n";
