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

void   *input_pulse(void* data);
void    getPulseDefaultSink();
