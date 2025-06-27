#include "rskft.h"
#include "math.h"

static const char* TAG = "PRJ_STFT";

#define FFT_SIZE 4096
#define STFT_SIZE 1024
#define WICKET_SIZE 1024

rskft_t rskfft;
rskft_t rskstft;

esp_err_t rskft_init(rskft_t *rskft,rskft_type_t type,int size)
{
    esp_err_t err = ESP_OK;
    dl_fft_f32_t *fft_handle = rskft->fft_handle;
    if(fft_handle != NULL)
    {
        ESP_LOGI(TAG,"fft handle is initted!");
    }
    /* FFT 输入为2的幂指数 */
    if(size == 0) return ESP_FAIL;
    int point = 1;
    while(point < size) point <<= 1;

    // point = point>>1;
    rskft->size = point;
    rskft->type = type;

    printf("point = %d\n",point);
    fft_handle = dl_rfft_f32_init(point,MALLOC_CAP_SPIRAM);
     printf("fft_handle.log2n = %d\n",fft_handle->log2n);
    rskft->fft_handle = fft_handle;

    if(fft_handle == NULL)
    {
        printf("fft init failed!\n");
        err = ESP_FAIL;
        return err;
    }
   
    rskft->input_data = (float *)heap_caps_malloc(point * sizeof(float), MALLOC_CAP_SPIRAM);

    if(type == RSKFT_STFT)
    {
        rskft->wicket_data = (float *)heap_caps_malloc(point * sizeof(float), MALLOC_CAP_SPIRAM);
    }
        

    return err;
}

void rskft_wicket_data_full(float *wicket_data,float *input_data,int len)
{
    for (int i = 0; i < len; i++) {
        float win = 0.5f * (1.0f - cosf(2 * M_PI  * i / (len - 1)));        //汉宁窗算法
        wicket_data[i] = input_data[i] * win;
    }
}
esp_err_t rskft_run(rskft_t *rskft,float *input_data,int len)
{
    esp_err_t err = ESP_OK;
    dl_fft_f32_t *fft_handle = rskft->fft_handle;
    if(fft_handle == NULL)
    {
        ESP_LOGI(TAG,"plese init fft handle first!");
        err = ESP_FAIL;
        return err;
    }
    int size = len;
    if(size > fft_handle->fft_point)
        size = fft_handle->fft_point;

    if(rskft->type == RSKFT_STFT)
    {
        printf("run stft!\n");
        memset(rskft->wicket_data,0,size * sizeof(float));
        rskft_wicket_data_full(rskft->wicket_data,input_data,size);
        dl_rfft_f32_run(fft_handle,rskft->wicket_data);
        memcpy(input_data,rskft->wicket_data,size * sizeof(float));
    }
    else
    {
        // for(int i = 0;i < len;i++)
        // {
        //     printf("%f ",input_data[i]);
        // }
        dl_rfft_f32_run(fft_handle,input_data);
        printf("run fft!\n");
    }
        

    float output_data = 0;
     
    for(int i = 0;i < fft_handle->fft_point/2;i++)
    {
        // printf("i=%d:real = %f\n",i,input_data[2 * i]);
        // printf("     imag = %f\n",input_data[2 * i + 1]);
        float real = input_data[2 * i];
        float imag = input_data[2 * i + 1];
        float magnitude = sqrtf(real * real + imag * imag);
        // printf("     magnitude = %f\n",magnitude);
        if(rskft->type == RSKFT_STFT)
        {
            // output_data += sqrtf(real * real + imag * imag);
            output_data += (real * real + imag * imag);
        }
        else 
        {
            if(magnitude > output_data)
            {
                output_data = magnitude;
            }
        } 
    }
    rskft->output_data = output_data;
    printf("fft output_data = %f\n",output_data);
    return err;
}

esp_err_t rskft_deinit(rskft_t *rskft)
{ 
    esp_err_t err = ESP_FAIL;
    if(!rskft) return err;
    if(!rskft->fft_handle)  return err;

    dl_rfft_f32_deinit(rskft->fft_handle);
    if(rskft->input_data)
        heap_caps_free(rskft->input_data);
    if(rskft->wicket_data)
        heap_caps_free(rskft->wicket_data);

    err = ESP_OK;
    return err;
}


