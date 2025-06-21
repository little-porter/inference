#include "stft.h"
#include "dl_rfft.h"
#include "math.h"

static const char* TAG = "PRJ_STFT";

float fft_input_data[] = {0};

float stft_wicket_data[] = {0};

float stft_input_data[] = {0};

float test_fft[4096];


void stft_wicket_data_full(float *wicket_data,float *input_data,int len)
{
    for (int i = 0; i < len; i++) {
        float win = 0.5f * (1.0f - cosf(2 * M_PI  * i / (len - 1)));        //汉宁窗算法
        wicket_data[i] = input_data[i] * win;
    }
}

void  stft_run(float *input_data,int len)
{
    dl_fft_f32_t *fft_handle;
    if(len = 0) return;
    int size = 1;
    while(size < len) size <<= 1;

    fft_handle = dl_rfft_f32_init(size,MALLOC_CAP_SPIRAM);
    dl_rfft_f32_run(fft_handle,input_data);
    
    for (int i = 0; i < size / 2 + 1; i++) {
        float real = fft_handle->rfft_table[2 * i];
        float imag = fft_handle->rfft_table[2 * i + 1];
        float magnitude = sqrtf(real * real + imag * imag);
        // printf("Freq bin %2d: %.2f Hz | Mag: %.4f\n", i, (float)i * 8000 / ())
    }
}






void test_fft_fun(void)
{
    dl_fft_f32_t *fft_handle;
    fft_handle = dl_rfft_f32_init(4096,MALLOC_CAP_SPIRAM);
    dl_rfft_f32_run(fft_handle,test_fft);

    for (int i = 0; i < (4096) / 2 + 1; i++) {
    float real = fft_handle->rfft_table[2 * i];
    float imag = fft_handle->rfft_table[2 * i + 1];
    float magnitude = sqrtf(real * real + imag * imag);
    // printf("Freq bin %2d: %.2f Hz | Mag: %.4f\n", i, (float)i * 8000 / (4096), magnitude);
    }

    dl_rfft_f32_deinit(fft_handle);

}



void stft_init(void)
{
    unsigned long  start_time = 0,end_time = 0;
    start_time = esp_timer_get_time();
    test_fft_fun();
    end_time = esp_timer_get_time();
    ESP_LOGI("main","fft using time is  %d ms!",(int)(end_time-start_time)/1000);
}





