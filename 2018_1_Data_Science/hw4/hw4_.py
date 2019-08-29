# %%
import keras
keras.applications.inception_v3.InceptionV3(
    include_top=True,
    weights='imagenet',
    input_tensor=None,
    input_shape=None,
    pooling=None,
    classes=1000)

from tensorflow.python.keras import backend as K
from tensorflow.python.keras.models import Model
from tensorflow.python.keras.layers import Flatten, Dense, Dropout
from tensorflow.python.keras.applications.resnet50 import ResNet50
from tensorflow.python.keras.optimizers import Adam
from tensorflow.python.keras.preprocessing.image import ImageDataGenerator

import csv
import os
import pathlib
#import imageio
import numpy as np
import pandas as pd
from PIL import Image

IMG_WIDTH = 28
IMG_HEIGHT = 28


def run(infile, outdir, label=None):
    #_make_dir(outdir)
    pic_array = []
    pic_label = []
    # parse file
    training = pd.read_csv(infile)#,names=train_header)
    data = training.values.tolist()
    for pic in data:
        pic_rgb = []
        for pix in pic[1:]:
            pic_rgb.append((pix, pix, pix))
        pixels = pic_rgb
        array = np.array(pixels, dtype=np.uint8)
        array = array.reshape((IMG_WIDTH, IMG_HEIGHT, 3))
        array = array.repeat(2, axis=0).repeat(2, axis=1)
        #print(array)
        pic_label.append(pic[0])
        pic_array.append(array)

        
    return np.array(pic_array), np.array(pic_label)

_EXISTING_PATHS = set()

def _make_dir(dirpath):
    if not os.path.exists(dirpath):
        os.makedirs(dirpath);
        '''
        mkdir -p
        pathlib.Path(dirpath).mkdir(parents=True, exist_ok=True)
        _EXISTING_PATHS.add(dirpath)
        '''


def _make_img(grays):
    pass
  
infile = 'data/train.csv'
dir_pic = 'sample/train'
train_pic, train_label = run(infile, dir_pic)

infile = 'data/test.csv'
dir_pic = 'sample/test'
test_pic, test_label = run(infile, dir_pic)

#print(train_pic[0])
print(np.shape(train_pic))
print(np.shape(train_label))
#print(test_label)

# 資料路徑
DATASET_PATH  = 'sample'
# 影像大小
IMAGE_SIZE = (56, 56)
# 影像類別數
NUM_CLASSES = 10
# 若 GPU 記憶體不足，可調降 batch size 或凍結更多層網路
BATCH_SIZE = 128
# 凍結網路層數
FREEZE_LAYERS = 10
# Epoch 數
NUM_EPOCHS = 10
# 模型輸出儲存的檔案
WEIGHTS_FINAL = 'model-resnet50-final.h5'

# 透過 data augmentation 產生訓練與驗證用的影像資料
train_datagen = ImageDataGenerator(rotation_range=40,
                                   width_shift_range=0.2,
                                   height_shift_range=0.2,
                                   shear_range=0.2,
                                   zoom_range=0.2,
                                   channel_shift_range=10,
                                   horizontal_flip=True,
                                   fill_mode='nearest')
train_batches = train_datagen.flow(train_pic,train_label,batch_size=BATCH_SIZE)

#test_datagen = ImageDataGenerator()
#test_batches = test_datagen.flow_from_directory(DATASET_PATH + '/test',batch_size=BATCH_SIZE)

# 輸出各類別的索引值
#for cls, idx in train_batches.class_indices.items():
 #   print('Class #{} = {}'.format(idx, cls))

# 以訓練好的 ResNet50 為基礎來建立模型，
# 捨棄 ResNet50 頂層的 fully connected layers
net = ResNet50(include_top=False, weights='imagenet', input_tensor=None,
               input_shape=(IMAGE_SIZE[0],IMAGE_SIZE[1],3))
x = net.output
x = Flatten()(x)

# 增加 DropOut layer
x = Dropout(0.5)(x)

# 增加 Dense layer，以 softmax 產生個類別的機率值
output_layer = Dense(NUM_CLASSES, activation='softmax', name='softmax')(x)

# 設定凍結與要進行訓練的網路層
net_final = Model(inputs=net.input, outputs=output_layer)
for layer in net_final.layers[:FREEZE_LAYERS]:
    layer.trainable = False
for layer in net_final.layers[FREEZE_LAYERS:]:
    layer.trainable = True

# 使用 Adam optimizer，以較低的 learning rate 進行 fine-tuning
net_final.compile(optimizer=Adam(lr=1e-5),
                  loss='sparse_categorical_crossentropy', metrics=['accuracy'])

# 輸出整個網路結構
#print(net_final.summary())

# 訓練模型
'''
net_final.fit_generator(train_batches,
                        steps_per_epoch = np.ceil(train_pic.shape[0]) // BATCH_SIZE,
                        epochs = NUM_EPOCHS,
                        verbose = 1
                       )
'''
net_final.fit(train_pic, train_label, batch_size=BATCH_SIZE, epochs=NUM_EPOCHS, verbose=1)
# 儲存訓練好的模型
net_final.save(WEIGHTS_FINAL)

net_final.evaluate(train_pic, train_label)
test_label = net_final.predict(np.array(test_pic))

#print(test_label)
t_label = []
for label in test_label:
    m = label.argmax(axis=0)
    t_label.append(m)
print(t_label)

# Record to submission.csv
submission = pd.read_csv('data/sub.csv')
print(submission.head())
submission['label'] = t_label#.flatten().astype(int)
submission.to_csv('submission.csv', index=False)

# 辨識每一張圖
for f in files:
    img = image.load_img(f, target_size=(28, 28))
    if img is None:
        continue
    x = image.img_to_array(img)
    x = np.expand_dims(x, axis = 0)
    pred = net.predict(x)[0]
    top_inds = pred.argsort()[::-1][:5]
    print(f)
    for i in top_inds:
        print('    {:.3f}  {}'.format(pred[i], cls_list[i]))