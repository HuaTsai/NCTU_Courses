import os
os.environ["CUDA_DEVICE_ORDER"]="PCI_BUS_ID"
os.environ["CUDA_VISIBLE_DEVICES"]="1"
from keras.applications.inception_v3 import InceptionV3
from keras.preprocessing import image
from keras.models import Model
from keras.layers import Dense, GlobalAveragePooling2D
from keras import backend as K
from keras.layers import Input
from keras.optimizers import SGD
import numpy as np
import pandas as pd
from skimage.transform import resize
from sklearn.preprocessing import OneHotEncoder
import threading

lock=threading.Lock()
X = None
def preprocessing(X_ori, idx_list):
    global X
    for i in idx_list:
        if i == idx_list[0]:
            tmp = (resize(X_ori[i, :] / 255, (75, 75)) * 255).astype(np.uint8)
            result = np.array([tmp, tmp, tmp]).reshape(1, 3, 75, 75)
        else:
            tmp = (resize(X_ori[i, :] / 255, (75, 75)) * 255).astype(np.uint8)
            tmp = np.array([tmp, tmp, tmp]).reshape(1, 3, 75, 75)
            result = np.r_[result, tmp]
            print(result.shape)
    lock.acquire()
    if X is None:
        X = result
    else:
        X = np.r_[X, result]
    lock.release()

df = pd.read_csv('./train.csv')
df_test = pd.read_csv('./test.csv')
y = df.values[:, 0].reshape(-1, 1)
y = OneHotEncoder().fit_transform(y).toarray()

X_ori = np.delete(df.values, 0, 1).reshape((-1, 28, 28))
X_test_ori = np.delete(df_test.values, 0, 1).reshape((-1, 28, 28))

cpu_num = os.cpu_count()
X_ori_list = np.array_split(np.arange(X_ori.shape[0]), cpu_num)

threads = []
for i in range(cpu_num):
    threads.append(threading.Thread(target=preprocessing, args=(X_ori, X_ori_list[i])))
    threads[i].start()

for i in range(cpu_num):
    threads[i].join()

np.array_split(np.arange(X_test_ori.shape[0]), cpu_num)

print('x shape')
print(X.shape)

