华清远见培训机构平台的的桃子分拣课设作业。
代码并不完全是本人编写，是在一个开源项目 https://github.com/spacewalk01/yolov11-tensorrt 的基础上更改而来。

1.安装vs2022 professional
https://visualstudio.microsoft.com/zh-hans/thank-you-downloading-visual-studio/?sku=Professional&channel=Release&version=VS2022&source=VSLandingPage&cid=2030&passive=false
选择“使用c++的桌面开发”的套件。

2.安装cuda12.4.0
https://developer.download.nvidia.com/compute/cuda/12.4.0/local_installers/cuda_12.4.0_551.61_windows.exe

3.安装Tensorrt8.6.1.6
https://developer.nvidia.com/downloads/compute/machine-learning/tensorrt/secure/8.6.1/zip/TensorRT-8.6.1.6.Windows10.x86_64.cuda-11.8.zip
将D:\TensorRT-8.6.1.6\lib放置环境PATH中。

4.安装cudnn8.9.7
https://developer.nvidia.com/downloads/compute/cudnn/secure/8.9.7/local_installers/12.x/cudnn-windows-x86_64-8.9.7.29_cuda12-archive.zip/
下载完毕后将内容拷贝到C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.4中，打开bin，将cublas64_12.dll，cublasLt64_12.dll，cudart64_12.dll分别复制一份，将复制品分别更名为cublas64_11.dll，cublasLt64_11.dll，cudart64_110.dll.

5.安装Qt6.7.3
具体下载方法看阿里云qt镜像网站https://developer.aliyun.com/mirror/qt?spm=a2c6h.13651102.0.0.44851b11iEqrtY
选择6.7.3中的msvc2022和source源文件，source源文件用来配置QMQTT,配置方法具体请看https://blog.csdn.net/kangzhaofang/article/details/138530094
安装完毕后，将D:\Qt\6.7.3\msvc2022_64\bin放置环境PATH中。

6.安装opencv4.10.0
https://github.com/opencv/opencv/releases/download/4.10.0/opencv-4.10.0-windows.exe
将D:\OpenCV4.10.0\build\x64\vc16\bin和D:\OpenCV4.10.0\build\x64\vc16\lib放到环境PATH中。

7.安装python3.11.4
https://www.python.org/ftp/python/3.11.4/python-3.11.4-amd64.exe
然后命令行执行pip install ultralytics

8.将项目使用visual stdio 2022打开，修改cmakelists.txt的第41行为你的实际tensorrt路径

9.release构建，将ptToOnnx.py和"D:\TensorRT-8.6.1.6\bin\trtexec.exe"放到release出来的，与exe文件同级的目录下。

QQ1976682491
