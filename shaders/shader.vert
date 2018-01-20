uniform float uf_time;
attribute vec2 av2_pos;
varying vec2 vv2_pos;

void main()
{
    vv2_pos = av2_pos;
    gl_Position = vec4(vv2_pos, 0., 1.);
}
