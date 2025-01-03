#pragma once

#include "NvInfer.h"
#include <opencv2/opencv.hpp>
#include "mainwindow.h"
using namespace nvinfer1;
using namespace std;
using namespace cv;

struct Detection
{
    float conf;
    int class_id;
    Rect bbox;
};

class YOLOv11
{

public:

    YOLOv11(string model_path, nvinfer1::ILogger& logger);
    ~YOLOv11();

    void preprocess(Mat& image);
    void infer();
    void postprocess(vector<Detection>& output);
    void draw(Mat& image, const vector<Detection>& output);
    void change_conf(float conf);;
    void setConf(float conf);
private:
    void init(std::string engine_path, nvinfer1::ILogger& logger);

    float* gpu_buffers[2];
    float* cpu_output_buffer;

    cudaStream_t stream;
    IRuntime* runtime;
    ICudaEngine* engine;
    IExecutionContext* context;


    int input_w;
    int input_h;
    int num_detections;
    int detection_attribute_size;
    int num_classes = 80;
    const int MAX_IMAGE_SIZE = 4096 * 4096;
    float conf_threshold = 0.3f;
    float nms_threshold = 0.4f;

    vector<Scalar> colors;

    void build(std::string onnxPath, nvinfer1::ILogger& logger);
    bool saveEngine(const std::string& filename);
};