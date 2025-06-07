#include "tflm.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "all_ops_resolver.h"

#define TFLM_AREA_SIZE  (3 * 1024 * 1024)           // 3M   张量数据存储空间（存储模型输入、输出、中间计算结果）


// 2. 创建操作符解析器(模型使用方法)
static  tflite::MicroMutableOpResolver<128>  micro_op_resolver;
// static tflite::AllOpsResolver micro_op_resolver;

alignas(8) const unsigned char model_data1[10] = {0};

static uint8_t *tflm_area = NULL;

typedef struct _tflm_module
{
    /* data */
    tflite::MicroInterpreter *interpreter;      //解释器
    const unsigned char *model_data;            //模型数据
    uint8_t *tf_area;                           //张量区域
}tflm_module_t;

tflm_module_t tflm_soc;
tflm_module_t tflm_soh;
tflm_module_t tflm_rul;
tflm_module_t tflm_rsk;

static void AllOpsResolver(tflite::MicroMutableOpResolver<128> *resolver) 
{
    // Please keep this list of Builtin Operators in alphabetical order.
    resolver->AddAbs();
    resolver->AddAdd();
    resolver->AddAddN();
    resolver->AddArgMax();
    resolver->AddArgMin();
    resolver->AddAssignVariable();
    resolver->AddAveragePool2D();
    resolver->AddBatchToSpaceNd();
    resolver->AddBroadcastArgs();
    resolver->AddBroadcastTo();
    resolver->AddCallOnce();
    resolver->AddCast();
    resolver->AddCeil();
    resolver->AddCircularBuffer();
    resolver->AddConcatenation();
    resolver->AddConv2D();
    resolver->AddCos();
    resolver->AddCumSum();
    resolver->AddDepthToSpace();
    resolver->AddDepthwiseConv2D();
    resolver->AddDequantize();
    resolver->AddDetectionPostprocess();
    resolver->AddDiv();
    resolver->AddElu();
    resolver->AddEqual();
    resolver->AddEthosU();
    resolver->AddExp();
    resolver->AddExpandDims();
    resolver->AddFill();
    resolver->AddFloor();
    resolver->AddFloorDiv();
    resolver->AddFloorMod();
    resolver->AddFullyConnected();
    resolver->AddGather();
    resolver->AddGatherNd();
    resolver->AddGreater();
    resolver->AddGreaterEqual();
    resolver->AddHardSwish();
    resolver->AddIf();
    resolver->AddL2Normalization();
    resolver->AddL2Pool2D();
    resolver->AddLeakyRelu();
    resolver->AddLess();
    resolver->AddLessEqual();
    resolver->AddLog();
    resolver->AddLogicalAnd();
    resolver->AddLogicalNot();
    resolver->AddLogicalOr();
    resolver->AddLogistic();
    resolver->AddLogSoftmax();
    resolver->AddMaxPool2D();
    resolver->AddMaximum();
    resolver->AddMean();
    resolver->AddMinimum();
    resolver->AddMirrorPad();
    resolver->AddMul();
    resolver->AddNeg();
    resolver->AddNotEqual();
    resolver->AddPack();
    resolver->AddPad();
    resolver->AddPadV2();
    resolver->AddPrelu();
    resolver->AddQuantize();
    resolver->AddReadVariable();
    resolver->AddReduceMax();
    resolver->AddRelu();
    resolver->AddRelu6();
    resolver->AddReshape();
    resolver->AddResizeBilinear();
    resolver->AddResizeNearestNeighbor();
    resolver->AddRound();
    resolver->AddRsqrt();
    resolver->AddSelectV2();
    resolver->AddShape();
    resolver->AddSin();
    resolver->AddSlice();
    resolver->AddSoftmax();
    resolver->AddSpaceToBatchNd();
    resolver->AddSpaceToDepth();
    resolver->AddSplit();
    resolver->AddSplitV();
    resolver->AddSqrt();
    resolver->AddSquare();
    resolver->AddSquaredDifference();
    resolver->AddSqueeze();
    resolver->AddStridedSlice();
    resolver->AddSub();
    resolver->AddSum();
    resolver->AddSvdf();
    resolver->AddTanh();
    resolver->AddTranspose();
    resolver->AddTransposeConv();
    resolver->AddUnidirectionalSequenceLSTM();
    resolver->AddUnpack();
    resolver->AddVarHandle();
    resolver->AddWhile();
    resolver->AddZerosLike();
}

/*初始化解释器*/
static void tflm_interpreter_init(tflite::MicroInterpreter *interpreter,const unsigned char *model_data,uint8_t *tf_area)
{
    //数据空间判断
    if (tf_area == NULL) 
    {
        printf("tf_area don't allocate memory!");
        return;
    }
     //加载模型
    const tflite::Model *tl_model = tflite::GetModel(model_data);
    if (tl_model->version() != TFLITE_SCHEMA_VERSION) 
    {
        MicroPrintf("Model provided is schema version %d not equal to supported "
    		"version %d.", tl_model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
   
    /* 创建解释器 */
    interpreter = new tflite::MicroInterpreter(tl_model, micro_op_resolver, tf_area, TFLM_AREA_SIZE);

    /*分配张量内存*/
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    MicroPrintf("interpreter is regester !");
    if (allocate_status != kTfLiteOk) {
        MicroPrintf("AllocateTensors() failed");
        return;
    }

    // ESP_LOGI(TAG, "Initializing tflm is over!");
}

extern "C" void tflm_init(void)
{
    /*初始化张量内存*/
    tflm_area = (uint8_t *)heap_caps_malloc(TFLM_AREA_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    /*初始化模型使用方法*/
    AllOpsResolver(&micro_op_resolver);

    /*初始化SOC模型*/
    tflm_soc.model_data = model_data1;
    tflm_soc.tf_area = tflm_area;
    tflm_interpreter_init(tflm_soc.interpreter, tflm_soc.model_data, tflm_soc.tf_area);
    /*初始化SOH模型*/
    tflm_soh.model_data = model_data1;
    tflm_soh.tf_area = tflm_area;
    tflm_interpreter_init(tflm_soh.interpreter, tflm_soh.model_data, tflm_soh.tf_area);
}


extern "C" void tflm_run(uint8_t model_type,float *input_data,uint32_t input_num,float *output_data)
{
    TfLiteTensor *input  = nullptr;
    TfLiteTensor *output = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;

    switch (model_type)
    {
    case 0:
        /* code */
        input  = tflm_soc.interpreter->input(0);
        output = tflm_soc.interpreter->output(0);
        interpreter = tflm_soc.interpreter;
        // tflm_soc.interpreter->Invoke();
        break;
    
    default:
        break;
    }

    /*设置输入张量数据*/
    for(int i = 0; i < input_num; i++)
    {
        input->data.f[i] = input_data[i];
    }

    /*进行推理*/
    TfLiteStatus invoke_status = interpreter->Invoke();
    if(invoke_status != kTfLiteOk) 
    {
        MicroPrintf("Invoke failed\n");
        return;
    }

    /*获取输出张量数据*/
    for (int i = 0; i < output->dims->size; i++) 
    {
        if(i>=2) return ;
        output_data[i] = output->data.f[i];
    }
}



