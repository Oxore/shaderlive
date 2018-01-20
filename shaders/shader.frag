uniform float uf_time;
uniform vec2 uv2_res;
varying vec2 vv2_pos;

float world(vec3 p)
{
    return min(length(p) - 1. + 0.1*sin(uf_time + 7.*(p.x+p.y-p.z)), p.y + 1.);
}

float trace(vec3 O, vec3 D, float L)
{
    for (int i = 0; i < 32; i++) {
        vec3 p = O + D * L;
        float d = world(p);
        L += d;
        if (d < .001) break;
    }
    return L;
}

void main()
{
    vec2 p = vv2_pos * vec2(1., uv2_res.y / uv2_res.x);
    vec3 o = vec3(0., 0., 3.);
    vec3 d = normalize(vec3(p, -1.));
    float l = trace(o, d, 0.);
    gl_FragColor = vec4(l/10.);
}
