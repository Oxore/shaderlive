uniform float uf_time;
uniform float uf_fft_l[464];
uniform float uf_fft_r[464];
uniform vec2 uv2_res;
varying vec2 vv2_pos;

float reducer = 0.0000001;
int numlines = 232; //464 lines for 20 kHz, 232 to skip every seceond line

float eq()
{
    for (int i = -numlines; i < numlines; ++i)
        if (vv2_pos.x <= float(i)/float(numlines))
            if (i < 0)
                return uf_fft_l[-i * 2] * exp(float(-i) * 0.001);
            else
                return uf_fft_r[i * 2] * exp(float(i) * 0.001);
    return 0.;
}

void main()
{
    vec2 my_offset = vv2_pos * vec2(1., uv2_res.y / uv2_res.x);
    gl_FragColor = vec4(vec3(abs(sin(eq() * reducer)) * abs(1./my_offset.y)), 0.);
}
