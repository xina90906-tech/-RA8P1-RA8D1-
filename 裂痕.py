"""
裂痕检测系统 — 瑞萨 RA8D1 Vision Board / OpenMV 固件 4.5.0

工作流程：
1. 人脸检测       — Haar Cascade 检测人脸
2. 等待5秒       — 人脸检测通过后等待5秒
3. 数字识别+锁定  — ML模型识别数字，当某数字概率 > 75% 时锁定该数字
4. 裂痕检测       — 锁定数字后持续检测裂痕
5. 串口输出       — 检测到裂痕则将锁定的数字编号发送至UART；未检测到不发送

管线：gamma校正 → 直方图均衡 → Canny-Sobel边缘提取
      → gamma对比度拉伸 → 二值化 → 膨胀加粗 → blob检测框选

全 Helium 加速管线（gamma/histeq/canny/binary/dilate）
"""

import sensor
import image
import lcd
import time
import uos
import gc
import tf
from machine import UART

# --------------------------- 配置参数 ---------------------------
start_flag = 0

# --------------------------- 功能类定义 ---------------------------
class PID_step_motor:
    def __init__(self, kp, ki, target=240):
        self.e = 0
        self.e_last = 0
        self.kp = kp
        self.ki = ki
        self.target = target
    def cal(self, value):
        self.e = self.target - value
        delta = self.kp * (self.e-self.e_last) + self.ki * self.e
        self.e_last = self.e
        return delta

rx_order1 = [0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x02, 0xF0, 0xF0]
def send_order1(number, dir_, steps):
    global rx_order1, uart
    rx_order1[2] = number
    rx_order1[3] = dir_
    rx_order1[4] = (steps // 256) % 256
    rx_order1[5] = steps % 256
    rx_order1[6] = sum(rx_order1[2:6]) % 256
    uart.write(bytes(rx_order1))

# ============================================================
# 可调参数
# ============================================================
GAMMA_PRE = 0.35        # 前段 gamma（0.3–0.6）—— 提亮暗部，越低越激进
GAMMA_POST = 2.5        # 后段 gamma（2.0–5.0）—— 对比度拉伸，越高边缘越凸显
DILATE_ITER = 0         # 膨胀迭代次数（1–3），加粗裂纹线条使其可被 blob 检测到
EDGE_THRESH = 40        # 二值化阈值（0–255），高于此值的像素为裂纹边缘

# ============================================================
# 可调参数 —— blob 判定与框选
# ============================================================
BLOB_AREA_MIN = 10         # 最小 blob 面积（过滤噪点）
BLOB_AREA_MAX = 1500      # 最大 blob 面积（过滤齿轮外轮廓大块边缘）
BLOB_DENSITY_MAX = 0.45   # 最大密度 area/(w*h)，裂纹是稀疏细线，齿轮边缘较密实
BLOB_ELONGATION_MIN = 2.5 # 最小长宽比 max(w,h)/min(w,h)，裂纹是细长条，齿轮齿廓较方正
BLOB_ROUNDNESS_MAX = 0.35 # 最大圆度 4*pi*area/perimeter^2，裂纹接近直线，齿轮弧线圆度偏高
BLOB_USE_RADIAL = True    # 径向距离过滤：排除齿轮外圈边缘，保留内部裂纹
BLOB_MERGE = True         # 合并邻近碎片
BLOB_MARGIN = 15          # 合并间距（像素）

DEBUG_BLOB = False         # 调试模式：打印每个阶段的 blob 数量



# ============================================================
# 人脸检测参数配置
# ============================================================

FACE_THRESHOLD = 0.6
FACE_SCALE = 1.3

# ============================================================
# 追踪参数配置
# ============================================================

DEADBAND = 15           #（死区 / 死带）
RETRACK_THRESHOLD =10  #（重新追踪阈值 / 唤醒阈值）
CENTERED_DURATION = 2

# ============================================================
# 可调参数 —— 数字识别
# ============================================================
DIGIT_WAIT_MS = 5000      # 人脸检测通过后等待时间（毫秒）
DIGIT_CONF_THRESH = 0.75  # 数字锁定置信度阈值
DIGIT_HOLD_MS = 2000      # 置信度达标后需持续保持的时间（毫秒），2秒后锁定

# ============================================================
# 状态常量
# ============================================================
STATE_FACE = 0          # 人脸检测状态
STATE_DIGIT = 1         # 等待5秒 + 数字识别锁定状态
STATE_CRACK = 2         # 裂痕检测状态


# ============================================================
# 人脸检测初始化
# ============================================================

def init_face_detection():
    try:
        cascade = image.HaarCascade("frontalface")
        return cascade
    except:
        try:
            cascade = image.HaarCascade("eye")
            return cascade
        except:
            return None


# ============================================================
# 人脸检测函数
# ============================================================

def detect_face(img, cascade):
    try:
        faces = img.find_features(cascade, threshold=FACE_THRESHOLD,
                                  scale_factor=FACE_SCALE)
        if faces:
            best = max(faces, key=lambda r: r[2] * r[3])
            return best
    except:
        pass
    return None


# ============================================================
# 裂痕处理管线
# ============================================================
def process_frame(img):
    # 第0步：RGB转灰度（摄像头输出为RGB565，裂痕检测需要灰度图像）
    # 使用replace确保原地修改（OpenMV中to_grayscale返回新图像）
    img.replace(img.to_grayscale())

    # 第1步：伽马校正 —— 强力提亮暗部
    # 3D打印黑色齿轮经 gamma<1 后表面被提亮为灰色，
    # 裂纹/划痕保持暗色，形成可利用的亮度差
    img.gamma(gamma=GAMMA_PRE, contrast=1.0, brightness=0.0)

    # 第2步：直方图均衡化 —— 全局对比度最大化
    img.histeq()

    # 第3步：Canny-Sobel 边缘提取 —— Helium SIMD 极致加速
    # image.EDGE_CANNY 内部使用 Sobel 3×3 卷积核计算梯度幅值与方向
    # |Gx| + |Gy| 在 Cortex-M85 上由 Helium (MVE) 向量指令集并行计算
    # 输出为二值化边缘图：边缘=白色(255)，背景=黑色(0)
    img.find_edges(image.EDGE_CANNY)

    # 第4步：gamma 对比度拉伸增强 —— 微弱边缘信号放大
    # gamma > 1 非线性变换：暗部（噪声/弱边缘）被压缩趋近于 0，
    # 亮部（真实裂纹边缘）保持高亮度，效果等价于对比度拉伸
    img.gamma(gamma=GAMMA_POST, contrast=1.0, brightness=0.0)

    # 第5步：二值化 —— 分离裂纹边缘与背景
    # binary([(lo, hi)]) 将亮度在 [lo, hi] 范围内的像素置为白色
    img.binary([(EDGE_THRESH, 255)])

    # 第6步：形态学膨胀 —— 加粗裂纹线条
    for _ in range(DILATE_ITER):
        img.dilate(1)

    return img

# ============================================================
# 裂纹 Blob 检测与判定
# ============================================================
def find_cracks(img, debug=False):
    """
    在二值化图像中查找裂纹 blob，针对长条状裂纹优化。

    判定逻辑：
    1. 面积过滤：噪点(太小) 和 齿轮外轮廓(太大) 排除
    2. 密度过滤：裂纹是稀疏细线，齿轮边缘较密实
    3. 长宽比过滤：裂纹是细长条，齿轮齿廓较方正
    4. 圆度过滤：裂纹接近直线(低圆度)，齿轮弧线圆度偏高
    5. 径向距离过滤（可选）：排除齿轮外圈边缘
    """
    debug = DEBUG_BLOB or debug
    try:
        all_blobs = img.find_blobs([(128, 255)], area_threshold=BLOB_AREA_MIN,
                                   merge=BLOB_MERGE, margin=BLOB_MARGIN)
        if debug:
            print("[BLOB] total found: %d" % len(all_blobs))

        if not all_blobs:
            return []

        w_img = img.width()
        h_img = img.height()
        cx_img = w_img // 2
        cy_img = h_img // 2

        # 逐级过滤 + 调试
        stage1 = []  # 面积
        stage2 = []  # 密度
        stage3 = []  # 长宽比
        stage4 = []  # 圆度

        for b in all_blobs:
            area = b.area()
            density = b.density()
            bw, bh = b.w(), b.h()
            elongation = max(bw, bh) / min(bw, bh) if bw >= 1 and bh >= 1 else 0
            roundness = b.roundness()

            if area < BLOB_AREA_MIN or area > BLOB_AREA_MAX:
                continue
            stage1.append(b)

            if density > BLOB_DENSITY_MAX:
                continue
            stage2.append(b)

            if elongation < BLOB_ELONGATION_MIN:
                continue
            stage3.append(b)

            if roundness > BLOB_ROUNDNESS_MAX:
                continue
            stage4.append(b)

        if debug:
            print("[BLOB] after area:      %d" % len(stage1))
            print("[BLOB] after density:   %d" % len(stage2))
            print("[BLOB] after elongation:%d" % len(stage3))
            print("[BLOB] after roundness: %d" % len(stage4))

        candidates = []
        for b in stage4:
            dist_sq = (b.cx() - cx_img) ** 2 + (b.cy() - cy_img) ** 2
            candidates.append((b, dist_sq))

        if not candidates:
            if debug:
                # 打印被淘汰 blob 的属性，辅助调参
                for i, b in enumerate(all_blobs[:5]):
                    bw, bh = b.w(), b.h()
                    elong = max(bw, bh) / min(bw, bh) if bw >= 1 and bh >= 1 else 0
                    print("[BLOB#%d] area=%d density=%.2f elong=%.2f round=%.2f w=%d h=%d" % (
                        i, b.area(), b.density(), elong, b.roundness(), bw, bh))
            return []

        # 径向距离过滤：排除齿轮外圈边缘，保留内部裂纹
        if BLOB_USE_RADIAL and len(candidates) >= 2:
            max_dist = max(d for _, d in candidates)
            boundary_threshold = max_dist * 64 // 100  # 最远距离的 80%
            cracks = [b for b, d in candidates if d < boundary_threshold]
            if debug:
                print("[BLOB] after radial:   %d  (max_dist^2=%d, thresh=%d)" % (
                    len(cracks), max_dist, boundary_threshold))
                for b, d in candidates:
                    label = "CRACK" if d < boundary_threshold else "GEAR"
                    print("  %s: (%d,%d) dist^2=%d area=%d" % (
                        label, b.cx(), b.cy(), d, b.area()))
            return cracks
        else:
            return [b for b, _ in candidates]

    except:
        pass
    return []



# ============================================================
# 串口初始化与发送
# ============================================================
global uart
# 串口初始化
uart = UART(2,115200)

# ============================================================
# 串口指令发送（PID步进电机）
# ============================================================
rx_order = [0xAA, 0xAA, 0x00, 0x00, 0x00, 0xFF, 0xFF]


def send_order(tab, crack):
    global rx_order, uart
    rx_order[2] = tab & 0xFF
    rx_order[3] = crack & 0xFF
    rx_order[4] = (tab + crack) & 0xFF
    uart.write(bytearray(rx_order))
    #print(1)

# ============================================================
# 数字识别（Edge Impulse TFLite模型）
# ============================================================
def init_digit_recognition():
    """加载Edge Impulse训练的TFLite模型和标签文件，返回(net, labels)。"""
    net = None
    labels = None

    try:
        net = tf.load("trained.tflite", load_to_fb=uos.stat('trained.tflite')[6] > (gc.mem_free() - (64*1024)))
        print("模型 trained.tflite 加载成功！")
    except Exception as e:
        print(e)
        print("WARN: Failed to load trained.tflite, skip digit recognition")
        return (None, None)

    try:
        labels = [line.rstrip('\n') for line in open("labels.txt")]
        print("标签 labels.txt 加载成功！")
    except Exception as e:
        print("WARN: Failed to load labels.txt, skip digit recognition")
        return (None, None)

    return (net, labels)


def recognize_digit(net, labels, img):
    """使用TFLite模型识别数字，返回[(label, probability), ...]。"""
    if net is None or labels is None:
        return []

    try:
        results = []
        for obj in tf.classify(net, img):
            predictions_list = list(zip(labels, obj.output()))
            results.extend(predictions_list)
        return results
    except Exception as e:
        print("ERR: classify failed:", e)
        return []


# ============================================================
# 主程序
# ============================================================
def main():

    # --- 硬件初始化 ---
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)   # RGB565格式（用于数字识别模型）
    sensor.set_framesize(sensor.QVGA)     # 320×240
    sensor.set_windowing((240, 240))      # 裁剪为240x240（匹配Edge Impulse训练尺寸）
    sensor.skip_frames(time=2000)         # 等待摄像头稳定
    sensor.set_vflip(True)
    sensor.set_hmirror(True)
    lcd.init()

    pid_x = PID_step_motor(kp=0.5, ki=0.7, target=120)
    pid_y = PID_step_motor(kp=0.5, ki=0.7, target=120)

    face_cascade = init_face_detection()
    if face_cascade is None:
        print("ERROR: Haar cascade not found!")
        return
    print("Face detection initialized")

    last_center_x = -1
    last_center_y = -1
    TARGET_CENTERED = False
    centered_start_time = 0


    # --- UART2 串口初始化（PID步进电机控制）---
    start_flag = 0  # 开启PID并发送数据标志位

    # --- 数字识别模型初始化 ---
    net, labels = init_digit_recognition()
    print("DIGIT MODEL: %s" % ("OK" if net else "NONE"))

    # --- 状态机 ---
    state = STATE_FACE
    face_rect = None
    locked_digit = None          # 锁定后的数字标签
    digit_wait_start = 0         # 进入数字等待状态的时刻 (ticks_ms)
    digit_candidate = None       # 当前概率>75%的候选数字标签
    digit_candidate_start = 0    # 候选数字首次达标的时刻 (ticks_ms)
    crack_sent = False            # 本段裂痕是否已发送过（仅第一帧发送）
    clock = time.clock()
    frame = 0

    while True:
        clock.tick()
        img = sensor.snapshot()

        # ========================================================
        # 状态0：人脸检测
        # ========================================================
        if state == STATE_FACE:
            face_rect = detect_face(img, face_cascade)

            if face_rect:
                x, y, w, h = face_rect
                center_x = x + w // 2
                center_y = y + h // 2

                print("Face center: (%d, %d)" % (center_x, center_y))

                img.draw_rectangle(face_rect, color=(0, 255, 0), thickness=2)
                img.draw_cross(center_x, center_y, color=(0, 255, 0), size=5)
                img.draw_string(10, 10, "(%d,%d)" % (center_x, center_y), color=(0, 255, 0))

                delta_from_center_x = abs(center_x - pid_x.target)
                delta_from_center_y = abs(center_y - pid_y.target)

                delta_from_last_x = abs(center_x - last_center_x) if last_center_x != -1 else 0
                delta_from_last_y = abs(center_y - last_center_y) if last_center_y != -1 else 0

                last_center_x = center_x
                last_center_y = center_y

                if delta_from_center_x > DEADBAND or delta_from_center_y > DEADBAND:
                    print("追踪目标，偏差: X=%d, Y=%d" % (delta_from_center_x, delta_from_center_y))
                    start_flag = 1
                    TARGET_CENTERED = False
                    centered_start_time = 0
                elif not TARGET_CENTERED:
                    print("目标已居中，开始计时")
                    TARGET_CENTERED = True
                    start_flag = 0
                    centered_start_time = time.time()
                elif delta_from_last_x > RETRACK_THRESHOLD or delta_from_last_y > RETRACK_THRESHOLD:
                    print("目标移动过大，重新追踪: X=%d, Y=%d" % (delta_from_last_x, delta_from_last_y))
                    start_flag = 1
                    TARGET_CENTERED = False
                    centered_start_time = 0
                elif TARGET_CENTERED and centered_start_time > 0:
                    elapsed_time = time.time() - centered_start_time
                    if elapsed_time >= CENTERED_DURATION:
                        print("目标居中达到 %d 秒，退出人脸追踪" % CENTERED_DURATION)
                        state = STATE_DIGIT
                        start_flag = 0
                        img.draw_string(10, 50, "Waiting 0xFF", color=(0, 0, 255), scale=2)

                if start_flag == 1:
                    delta_x = pid_x.cal(center_x)
                    dir_x = 0 if delta_x > 0 else 1
                    send_order1(0, dir_x, abs(int(delta_x)))

                    delta_y = pid_y.cal(center_y)
                    dir_y = 0 if delta_y < 0 else 1
                    send_order1(1, dir_y, abs(int(delta_y)))

            else:
                img.draw_string(10, 10, "No Face", color=(255, 0, 0), scale=2)
                print("No face detected")
                start_flag = 0
                TARGET_CENTERED = False
                centered_start_time = 0
                last_center_x = -1
                last_center_y = -1

        # ========================================================
        # 状态1：等待5秒 + 数字识别保持2秒后锁定
        # ========================================================
        if state == STATE_DIGIT:
            now = time.ticks_ms()
            elapsed = time.ticks_diff(now, digit_wait_start)

            if elapsed < DIGIT_WAIT_MS:
                # 等待倒计时 — 不处理图像，直接显示原始画面
                remain = (DIGIT_WAIT_MS - elapsed) // 1000 + 1
                img.draw_string(10, 10, "Wait %ds..." % remain, color=(255, 255, 0), scale=2)
                img.draw_string(10, 40, "Please identify the number.", color=(255, 255, 0), scale=2)
                lcd.display(img.copy())
                frame += 1
                continue

            # 5秒到 → 执行数字识别（使用原始画面，不做图像处理）
            digit_results = recognize_digit(net, labels, img.copy())

            if digit_results:
                # 找置信度最高的数字
                best_label, best_prob = max(digit_results, key=lambda x: x[1])

                # 显示识别结果
                img.draw_string(10, 10, "Digit: %s (%.0f%%)" % (best_label, best_prob * 100),
                                color=(255, 255, 0), scale=2)
                img.draw_string(10, 40, "Please identify the number.", color=(255, 255, 0), scale=2)

                if best_prob >= DIGIT_CONF_THRESH:
                    # 置信度达标 — 检查是否为同一候选数字
                    if best_label == digit_candidate:
                        # 同一候选，检查是否已保持足够时间
                        hold_elapsed = time.ticks_diff(now, digit_candidate_start)
                        if hold_elapsed >= DIGIT_HOLD_MS:
                            # 保持2秒以上 → 锁定数字 → 进入裂痕检测
                            locked_digit = best_label
                            digit_candidate = None
                            digit_candidate_start = 0
                            state = STATE_CRACK
                            print("DIGIT LOCKED: %s (%.1f%%) -> CRACK MODE" % (
                                locked_digit, best_prob * 100))
                            # fall through 到裂痕检测（本帧就执行）
                        else:
                            # 还在保持中，显示进度
                            hold_remain = (DIGIT_HOLD_MS - hold_elapsed) // 1000 + 1
                            img.draw_string(10, 70, "Holding %s... %ds" % (best_label, hold_remain),
                                            color=(0, 255, 0), scale=2)
                            lcd.display(img.copy())
                            frame += 1
                            continue
                    else:
                        # 新的候选数字 → 重新开始计时
                        digit_candidate = best_label
                        digit_candidate_start = now
                        img.draw_string(10, 70, "Candidate: %s" % best_label,
                                        color=(0, 255, 0), scale=2)
                        lcd.display(img.copy())
                        frame += 1
                        continue
                else:
                    # 置信度不足 → 重置候选，继续扫描
                    digit_candidate = None
                    digit_candidate_start = 0
                    img.draw_string(10, 70, "Scanning...", color=(255, 150, 0), scale=2)
                    lcd.display(img.copy())
                    frame += 1
                    continue
            else:
                # 模型未就绪 → 继续扫描
                img.draw_string(10, 10, "No model", color=(255, 0, 0), scale=2)
                img.draw_string(10, 40, "Please identify the number.", color=(255, 255, 0), scale=2)
                lcd.display(img.copy())
                frame += 1
                continue

        # ========================================================
        # 状态2：裂痕检测（数字已锁定）
        # ========================================================
        if state == STATE_CRACK:
            # 执行裂痕检测管线（会原地修改图像为灰度二值图）
            process_frame(img)

            # 查找裂纹 blob
            blobs = find_cracks(img)

            # 显示锁定数字
            img.draw_string(10, 10, "Locked: %s" % locked_digit, color=(0, 255, 0))

            if blobs:
                # 框选裂纹
                for b in blobs:
                    img.draw_rectangle(b.rect(), color=255, thickness=1)
                    img.draw_cross(b.cx(), b.cy(), color=255, size=3)

                # 仅在检测到裂痕的第一帧，并且收到串口发来的 0x0F 后发送
                if not crack_sent:

                    rx_data = uart.read()         # 读取所有串口缓存数据【
                    if 0xFF in rx_data:           # 识别到发过来的 0xFF 字节+
                        send_order(int(locked_digit), 1)
                        print("hdfdhkkjfkhdkslkjfksljflkfj")
                        crack_sent = True

                # 帧率统计（每10帧打印一次）
                if frame % 10 == 0:
                    print("FPS: %.1f  digit=%s  cracks=%d" % (
                        clock.fps(), locked_digit, len(blobs)))

            else:
                # 未检测到裂纹
                if frame % 10 == 0:
                    print("FPS: %.1f  digit=%s  no cracks" % (
                        clock.fps(), locked_digit))

        # --- 显示 ---
        lcd.display(img.copy())
        frame += 1


main()
