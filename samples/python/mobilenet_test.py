import tensorflow as tf
import cv2
import numpy as np
from tensorflow import keras
from tensorflow.keras import layers
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from keras.applications.mobilenet import MobileNet, preprocess_input, decode_predictions

# 加载数据集
train_data = ImageDataGenerator().flow_from_directory('dataset/train')
valid_data = ImageDataGenerator().flow_from_directory('dataset/validation')

# 加载预训练的MobileNet模型
base_model = keras.applications.MobileNetV2(
    weights='imagenet',
    input_shape=(224, 224, 3),
    include_top=False
)

# 冻结MobileNet模型的前几层
for layer in base_model.layers[:-4]:
    layer.trainable = False

# 定义新的全连接层
inputs = keras.Input(shape=(224, 224, 3))
x = base_model(inputs, training=False)
x = layers.GlobalAveragePooling2D()(x)
x = layers.Dense(256, activation='relu')(x)
outputs = layers.Dense(train_data.num_classes, activation='softmax')(x)

model = keras.Model(inputs, outputs)

# 编译模型并进行微调训练
model.compile(
    optimizer=keras.optimizers.Adam(),
    loss=keras.losses.CategoricalCrossentropy(),
    metrics=[keras.metrics.CategoricalAccuracy()]
)

model.fit(
    train_data,
    epochs=10,
    validation_data=valid_data
)

# 读取图像
img = cv2.imread('3.jpg')

# 调整图像大小为 (224, 224)
img = cv2.resize(img, (224, 224))

# 转换 BGR 到 RGB 格式
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

# 预处理图像
img = preprocess_input(img)

# 预测类别
preds = model.predict(img[np.newaxis,...])
print("preds: ", preds)
# 解码预测结果
results = decode_predictions(preds, top=1)[0]

# 输出预测结果
print('Predicted:', results[0][1])