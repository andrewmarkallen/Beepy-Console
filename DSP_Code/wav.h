#ifndef _WAV_H_
#define _WAV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcmp
#include <stdint.h> // for int16_t and int32_t

typedef struct wavfile_struct
{
    char    id[4];          // should always contain "RIFF"
    int32_t totallength;    // total file length minus 8
    uint8_t    wavefmt[8];     // should be "WAVEfmt "
    int32_t format;         // 16 for PCM format
    int16_t pcm;            // 1 for PCM format
    int16_t channels;       // channels
    int32_t frequency;      // sampling frequency
    int32_t bytes_per_second;
    int16_t bytes_by_capture;
    int16_t bits_per_sample;
    char    data[4];        // should always contain "data"
    int32_t bytes_in_data;
} wavfile;
//__attribute__ ((__packed__))
//Check id is RIFF

int is_wav_format(wavfile* header);

int is_single_channel_16_bit_PCM(wavfile* header);

int read_header(FILE* wav, wavfile* header);

int load_data(FILE* input, void* data, uint32_t size);

int write_data(wavfile* header, FILE* output, void* data, uint32_t size);

#endif