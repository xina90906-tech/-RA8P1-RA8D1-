# Edge Impulse - OpenMV Image Classification Example
# Modified for Renesas Vision Board (tf module compatible)

import time
import uos
import gc
import sensor
import tf  # 确保导入的是 tf 模块

print("基础模块加载成功")

# 1. 初始化摄像头
sensor.reset()                         # 复位并初始化摄像头
sensor.set_pixformat(sensor.RGB565)    # 设置图像格式为 RGB565 (彩色)
sensor.set_framesize(sensor.QVGA)      # 设置分辨率为 QVGA (320x240)
sensor.set_windowing((240, 240))       # 裁剪为 240x240 正方形（匹配 Edge Impulse 训练尺寸）
sensor.skip_frames(time=2000)          # 等待摄像头稳定

net = None
labels = None

# 2. 加载 TFLite 模型
try:
    # 自动根据剩余内存大小决定是否将模型加载到 Framebuffer (tf.load)
    net = tf.load("trained.tflite", load_to_fb=uos.stat('trained.tflite')[6] > (gc.mem_free() - (64*1024)))
    print("模型 trained.tflite 加载成功！")
except Exception as e:
    print(e)
    raise Exception('Failed to load "trained.tflite", did you copy the .tflite and labels.txt file onto the mass-storage device? (' + str(e) + ')')

# 3. 加载标签文件
try:
    labels = [line.rstrip('\n') for line in open("labels.txt")]
    print("标签 labels.txt 加载成功！")
except Exception as e:
    raise Exception('Failed to load "labels.txt", did you copy the .tflite and labels.txt file onto the mass-storage device? (' + str(e) + ')')

# 4. 核心主循环
clock = time.clock()
while(True):
    clock.tick()

    # 捕获一张图片
    img = sensor.snapshot()

    # 使用老版 tf.classify 替代新版 ml.predict
    # tf.classify 会返回一个包含识别结果的对象列表
    for obj in tf.classify(net, img):
        print("" * 10)
        print("Predictions:")

        # obj.output() 会直接返回一个包含所有类别概率的列表
        predictions_list = list(zip(labels, obj.output()))

        # 循环打印每个类别的置信度
        for i in range(len(predictions_list)):
            print("%s = %f" % (predictions_list[i][0], predictions_list[i][1]))

    # 打印当前帧率
    print(clock.fps(), "fps")
