import sys
from ultralytics import YOLO
import os

# 检查是否提供了命令行参数
if len(sys.argv) != 2:
    print("Usage: python ptToOnnx.py <path_to_pt_model>")
    sys.exit(1)

# 获取命令行传入的 PyTorch 模型路径
pt_model_path = sys.argv[1]

# 检查模型文件是否存在
if not os.path.exists(pt_model_path):
    print(f"Error: The specified file '{pt_model_path}' does not exist.")
    sys.exit(1)

# 加载 YOLO 模型
model = YOLO(pt_model_path)

# 获取输入模型所在的文件夹路径
output_dir = os.path.dirname(pt_model_path)

# 导出模型为 ONNX 格式到相同目录
export_path = model.export(format="onnx", imgsz=640, save_dir=output_dir)

print(f"Model exported to {export_path}")
