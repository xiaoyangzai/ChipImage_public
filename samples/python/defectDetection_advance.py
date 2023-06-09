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

max_area = 0
max_contour = None
for contour in contours:
    area = cv2.contourArea(contour)
    if area > max_area:
        max_area = area
        max_contour = contour

# 从上半部分寻找第一条线
half_height = img.shape[0] //2
max_y_point = None 
# 找到contour中Y坐标值大于0小于图像高度一半范围内的Y值最小的点
points = []
for contour in contours:
    for point in contour:
        y = point[0][1]
        if y > 0 and y < half_height:
            if point[0][0] < 10 or point[0][0] >= img.shape[1] - 10:
                continue
            points.append(point[0])

if len(points) > 0:
    max_y_point = max(points, key=lambda p: p[1])
else:
    print('没有符合条件的点')

top_y = None
for y in range(max_y_point[1], half_height):
    if np.sum(max_contour[y]) > 0:
        top_y = y
        break

# 从下半部分寻找第二条线
bottom_y = None
for y in range(half_height, img.shape[0]):
    if np.sum(max_contour[y]) > 0:
        bottom_y = y
    else:
        break

# 在图像上绘制两条水平线
if top_y is not None and bottom_y is not None:
    cv2.line(img, (0, top_y), (img.shape[1], top_y), (0, 0, 255), 2)
    cv2.line(img, (0, bottom_y), (img.shape[1], bottom_y), (0, 0, 255), 2)

    # 计算宽度并返回结果
    width = bottom_y - top_y
    result = {'top_line_y': top_y, 'bottom_line_y': bottom_y, 'width': width}
    print(result)
else:
    print('没找到')

# 在原始图像上绘制轮廓
cv2.drawContours(img, contours, -1, (0, 255, 255), 2)
# 显示结果
cv2.imshow('result', img)
cv2.waitKey(0)
