#include "_glv.h"

#include "builtin_shaders.h"

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


const char *draw_glyph_vert = 
"#version 110\n"
"\n"
"attribute vec4 vertex_p\n;"
"attribute vec2 tex_coords;\n"
"attribute vec2 glyph_coords;\n"
"\n"
"varying vec2 tex_uv;\n"
"varying vec2 glyph_uv;\n"
"\n"
"void main(){\n"
"    tex_uv = tex_coords;\n"
"    glyph_uv = glyph_coords;\n"
"    gl_Position = vertex_p;\n"
"}\n";

const char *draw_glyph_frag = 
"#version 110\n"
"uniform sampler2D tex;\n"
"uniform sampler2D glyph_tex;\n"
"varying vec2 tex_uv;\n"
"varying vec2 glyph_uv;\n"
"\n"
"void main(){\n"
"    float glyph_mask = texture2D(glyph_tex, glyph_uv).a;\n"
"    vec4 color = texture2D(tex, tex_uv);\n"
"    gl_FragColor = vec4(color.rgb, glyph_mask * color.a);\n"
"}\n";

const char *draw_circle_vert = 
"#version 110\n"
"attribute vec4 vbo;\n"
"uniform vec2 scale;\n"
"uniform vec2 offset;\n"
"varying vec2 frag_coord;\n"
"\n"
"void main(){\n"
"    gl_Position = vec4(vbo.xy * scale + offset, vbo.zw);\n"
"    frag_coord = vbo.xy;\n"
"}\n";

const char *draw_circle_frag = 
"#version 110\n"
"varying vec2 frag_coord;\n"
"uniform vec4 color;\n"
"uniform float px_radius;\n"
"\n"
"float circle_mask = 1.0 - distance(frag_coord, vec2(0.0, 0.0));\n"
"float circle_clear_mask = circle_mask * px_radius;\n"
"\n"
"void main(){\n"
"    gl_FragColor = vec4(color.rgb, circle_clear_mask * color.a);\n"
"}\n";