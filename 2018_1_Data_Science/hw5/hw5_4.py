# %%
import os
os.environ["CUDA_DEVICE_ORDER"]="PCI_BUS_ID"
os.environ["CUDA_VISIBLE_DEVICES"]="1" 
from keras.callbacks import ModelCheckpoint
from keras.models import Model
from keras.layers import Input, Dense, concatenate, Activation
from keras.preprocessing.sequence import pad_sequences
from keras.preprocessing.text import Tokenizer
from gensim.models import KeyedVectors
from sklearn import utils
import multiprocessing
from gensim.models.doc2vec import TaggedDocument
from gensim.models.word2vec import Word2Vec
import gensim
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
import nltk
from nltk.corpus import stopwords
from nltk.classify import SklearnClassifier
from wordcloud import WordCloud
import matplotlib.pyplot as plt
from bs4 import BeautifulSoup
import re
from sklearn.feature_extraction.text import TfidfVectorizer
from tqdm import tqdm
tqdm.pandas(desc="progress-bar")

# %%
df_clean = pd.read_csv(
    '/home/ee904/Desktop/HuaTsai/learn_ds/hw5/clean_train.csv')
X = df_clean.text
y = df_clean.sentiment
X_train, X_val, y_train, y_val = train_test_split(X, y, test_size=0.1)

# %%
df_clean_test = pd.read_csv(
    '/home/ee904/Desktop/HuaTsai/learn_ds/hw5/clean_test.csv')
X_test = df_clean_test.text


# %%
def labelize_tweets(tweets, prefix):
    result = []
    for i, t in enumerate(tweets):
        result.append(TaggedDocument(t.split(), [prefix + '_%s' % i]))
    return result


# %%
model_cbow = KeyedVectors.load('w2v_model_cbow.word2vec')
model_sg = KeyedVectors.load('w2v_model_sg.word2vec')

# %%
embeddings_index = {}
for w in model_cbow.wv.vocab.keys():
    embeddings_index[w] = np.append(model_cbow.wv[w], model_sg.wv[w])
print('Found %s word vectors.' % len(embeddings_index))

# %%
tokenizer = Tokenizer(num_words=100000)
tokenizer.fit_on_texts(X_train)
x_train_seq = tokenizer.texts_to_sequences(X_train)
x_train_seq = pad_sequences(x_train_seq, maxlen=35)
x_val_seq = tokenizer.texts_to_sequences(X_val)
x_val_seq = pad_sequences(x_val_seq, maxlen=35)
x_test_seq = tokenizer.texts_to_sequences(X_test)
x_test_seq = pad_sequences(x_test_seq, maxlen=35)

# %%
num_words = 100000
embedding_matrix = np.zeros((num_words, 200))
for word, i in tokenizer.word_index.items():
    if i >= num_words:
        continue
    embedding_vector = embeddings_index.get(word)
    if embedding_vector is not None:
        embedding_matrix[i] = embedding_vector

# %%
y_train[y_train == 4] = 1
y_val[y_val == 4] = 1


# %%
def MSE(y_predict, y_val):
    result = y_predict * 4
    A = result.flatten()
    B = y_val.values
    mse = (np.square(A - B)).mean()
    print(mse)


# %%
from keras.layers import Conv1D, GlobalMaxPooling1D
from keras.models import Sequential
from keras.layers import Dense, Dropout
from keras.layers import Flatten
from keras.layers.embeddings import Embedding
tweet_input = Input(shape=(45,), dtype='int32')

tweet_encoder = Embedding(100000, 200, weights=[
                          embedding_matrix], input_length=45, trainable=True)(tweet_input)
bigram_branch = Conv1D(filters=100, kernel_size=2, padding='valid',
                       activation='relu', strides=1)(tweet_encoder)
bigram_branch = GlobalMaxPooling1D()(bigram_branch)
trigram_branch = Conv1D(filters=100, kernel_size=3, padding='valid',
                        activation='relu', strides=1)(tweet_encoder)
trigram_branch = GlobalMaxPooling1D()(trigram_branch)
fourgram_branch = Conv1D(filters=100, kernel_size=4, padding='valid',
                         activation='relu', strides=1)(tweet_encoder)
fourgram_branch = GlobalMaxPooling1D()(fourgram_branch)
merged = concatenate([bigram_branch, trigram_branch, fourgram_branch], axis=1)

merged = Dense(256, activation='relu')(merged)
merged = Dropout(0.2)(merged)
merged = Dense(1)(merged)
output = Activation('sigmoid')(merged)
model = Model(inputs=[tweet_input], outputs=[output])
model.compile(loss='binary_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])
print(model.summary())

# %%
filepath = "CNN_best_weights.{epoch:02d}-{val_acc:.4f}.hdf5"
checkpoint = ModelCheckpoint(
    filepath, monitor='val_acc', verbose=1, save_best_only=True, mode='max')
model.fit(x_train_seq, y_train, batch_size=32, epochs=5,
          validation_data=(x_val_seq, y_val), callbacks=[checkpoint])

# %%
