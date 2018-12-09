//header file for pulse, part of cava.
struct audio_data {
    int audio_out_r[2048];
    int audio_out_l[2048];
    sem_t semaphore;
    int format;
    unsigned int rate;
    char *source; //alsa device, fifo path or pulse source
    int im; //input mode alsa, fifo or pulse
    int channels;
    int terminate; // shared variable used to terminate audio thread
    char *error_message;
};

struct config_params {
    char *color, *bcolor, *raw_target, *audio_source, *gradient_color_1,
         *gradient_color_2, *data_format;
    char bar_delim, frame_delim ;
    double monstercat, integral, gravity, ignore, sens;
    unsigned int lowcf, highcf;
    double *smooth;
    int smcount, customEQ, im, om, col, bgcol, autobars, stereo, is_bin,
        ascii_range, bit_format, gradient, fixedbars, framerate, bw, bs,
        autosens, overshoot, waves;
};

void   *input_pulse(void* data);
void    getPulseDefaultSink();
