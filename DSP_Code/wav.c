#include "wav.h"

int is_wav_format(wavfile* header)
{
  if (  header->id[0] != 'R'
        || header->id[1] != 'I' 
        || header->id[2] != 'F' 
        || header->id[3] != 'F' ) 
  { 
    printf("ERROR: Not wav format: %s\n", header->id);
    return 0;
  } 
  else
  {
    return 1;
  }
}

int is_single_channel_16_bit_PCM(wavfile* header)
{
  return 0;
}

int read_header(FILE* wav, wavfile* header)
{
  //fread(header->id,4*sizeof(uint8_t),4,wav);
  //fread(&header->totallength,sizeof(int32_t),1,wav);
  //fread(header,sizeof(header),1,wav);
  //fread(header,sizeof(header),1,wav);
  //fread(header,sizeof(header),1,wav);
    
  //printf("Size of header: %u\n", sizeof(wavfile));  
  if( fread(header,sizeof(wavfile),1,wav) < 1)
  {
    printf("Can't read input header!");
    return 0;
  }
  else
  {
    printf("HEADER FOR FILE:\n");
    printf("id:               %.*s\n", 4, header->id);
    printf("total length:     %ld\n", (int32_t)header->totallength);
    printf("wavefmt:          %.*s\n", 8, header->wavefmt);
    printf("format:           %ld\n", (int32_t)header->format);
    printf("pcm:              %d\n", (int16_t)header->pcm);
    printf("channels:         %d\n", (int16_t)header->channels);
    printf("frequency:        %ld\n", (int32_t)header->frequency);
    printf("bytes_per_second: %ld\n", (int32_t)header->bytes_per_second);
    printf("bytes_by_capture: %d\n", (int16_t)header->bytes_by_capture);
    printf("bits_per_sample:  %d\n", (int16_t)header->bits_per_sample);
    printf("data:             %.*s\n", 4, header->data);
    printf("bytes_in_data:    %ld\n\n", header->bytes_in_data);
    return 1;
  }
}

int load_data(FILE* input, void* data, uint32_t size)
{
  return fread(data,sizeof(int16_t),size,input); 
  
  //return fread(data,sizeof(uint16_t),size,input); 
}

int write_data(wavfile* header, FILE* output, void* data, uint32_t size)
{
  printf("Writing %u size header\n", (uint16_t)sizeof(wavfile));
  printf("Writing %lu 16_bit values\n", size);
  uint32_t header_bytes_written = fwrite(header, sizeof(wavfile),1, output);
  uint32_t data_bytes = fwrite(data, sizeof(uint16_t), size, output); 
  printf("Wrote %lu header bytes and %lu data bytes\n", header_bytes_written, data_bytes);
  return fclose(output);
  //return 1;
}
