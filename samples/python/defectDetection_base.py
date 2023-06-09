import cv2
import numpy as np

# 加载图像并转换为灰度图像
img = cv2.imread('kerf_image.jpg')
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# 计算灰度直方图
hist = cv2.calcHist([gray], [0], None, [256], [0, 256])

# 找到灰度直方图的峰值
max_val = np.max(hist)
min_val = np.min(hist)

# 对图像进行二值化
#_, binary = cv2.threshold(gray, 50, 255, cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)
blur = cv2.GaussianBlur(gray, (3, 3), 0)
_, binary = cv2.threshold(blur, min_val, max_val, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

# 开运算去除小噪点
kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
dilate = cv2.dilate(binary, kernel, iterations=1)
erode = cv2.erode(dilate, kernel, iterations=1)

# 查找轮廓
contours, _ = cv2.findContours(erode, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# 应用面积阈值，忽略面积非常小的轮廓
area_threshold = 100
contours = [contour for contour in contours if cv2.contourArea(contour) > area_threshold]

# 在原始图像上绘制轮廓
cv2.drawContours(img, contours, -1, (0, 255, 0), 2)
# 显示结果
cv2.imshow('result', img)
cv2.waitKey(0)
