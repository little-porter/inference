#ifndef _RSKFT_H_
#define _RSKFT_H_ 

#include "project_system.h"
#include "dl_rfft.h"

typedef enum 
{
    RSKFT_FFT = 0,
    RSKFT_STFT = 1,
}rskft_type_t;

typedef struct
{
    dl_fft_f32_t *fft_handle;
    float *input_data;
    float *wicket_data;
    float output_data;
    int size;
    rskft_type_t type;
}rskft_t;

extern rskft_t rskfft;
extern rskft_t rskstft;

esp_err_t rskft_init(rskft_t *rskft,rskft_type_t type,int size);
esp_err_t rskft_run(rskft_t *rskft,float *input_data,int len);

#endif
