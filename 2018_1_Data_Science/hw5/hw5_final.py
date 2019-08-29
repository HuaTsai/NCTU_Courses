# %%
'''
reference:
https://github.com/tthustla/twitter_sentiment_analysis_part11/blob/master/Capstone_part11.ipynb
'''
import multiprocessing
import os
import re
import sys

import nltk
import numpy as np
import pandas as pd
from bs4 import BeautifulSoup
from gensim.models import KeyedVectors
from gensim.models.doc2vec import TaggedDocument
from gensim.models.word2vec import Word2Vec
from keras.callbacks import ModelCheckpoint
from keras.layers import Activation
from keras.layers import Conv1D
from keras.layers import Dense
from keras.layers import Dropout
from keras.layers import GlobalMaxPooling1D
from keras.layers import Input
from keras.layers import concatenate
from keras.layers.embeddings import Embedding
from keras.models import Model, load_model
from keras.preprocessing.sequence import pad_sequences
from keras.preprocessing.text import Tokenizer
from nltk.corpus import stopwords
from sklearn import utils
from sklearn.model_selection import train_test_split
from tqdm import tqdm

tqdm.pandas(desc="progress-bar")


# %%
def normalize(df):
    stopwords_set = set(stopwords.words("english"))
    ret = []
    for i in range(df.shape[0]):
        text = df.text[i]
        text = BeautifulSoup(text, 'html.parser').get_text()
        text = re.sub(r'@[A-Za-z0-9]+', '', text)
        text = re.sub(r'http\S+', '', text)
        text = re.sub(r'[^a-zA-Z]', ' ', text)
        text = text.lower()
        words = nltk.word_tokenize(text.lower())
        words = [word for word in words if word not in stopwords_set]
        if (not words):
            words = ['good']
        ret.append(' '.join(words).strip())
        if ((i + 1) % 50000 == 0 or (i + 1) == df.shape[0]):
            print('Tweets %d of %d has been processed' % (i + 1, df.shape[0]))
    return ret


# %%
print('normalize train.csv')
df = pd.read_csv(sys.argv[1])
df_clean = pd.DataFrame()
df_clean['text'] = normalize(df)
df_clean['sentiment'] = df.sentiment
df_clean.to_csv('clean_train.csv', index=False)
print('save clean_train.csv')

# %%
print('normalize test.csv')
df_test = pd.read_csv(sys.argv[2])
df_clean_test = pd.DataFrame(normalize(df_test), columns=['text'])
df_clean_test.to_csv('clean_test.csv', index=False)
print('save test.csv')

# %%
df_clean = pd.read_csv('clean_train.csv')
X = df_clean.text
y = df_clean.sentiment
X_train, X_val, y_train, y_val = train_test_split(X, y, test_size=0.1)
df_clean_test = pd.read_csv('clean_test.csv')
X_test = df_clean_test.text


# %%
def labelize_tweets(tweets, prefix):
    result = []
    for i, t in enumerate(tweets):
        result.append(TaggedDocument(t.split(), [prefix + '_%s' % i]))
    return result


# %%
X_all = pd.concat([X_train, X_val])
X_all_w2v = labelize_tweets(X_all, 'all')
cores = multiprocessing.cpu_count()

# %%
print('training cbow model')
model_cbow = Word2Vec(sg=0, size=100, negative=5, window=2,
                      min_count=2, workers=cores, alpha=0.065, min_alpha=0.065)
model_cbow.build_vocab([x.words for x in tqdm(X_all_w2v)])
for epoch in range(30):
    model_cbow.train(utils.shuffle([x.words for x in tqdm(
        X_all_w2v)]), total_examples=len(X_all_w2v), epochs=1)
    model_cbow.alpha -= 0.002
    model_cbow.min_alpha = model_cbow.alpha

# %%
print('training skip-gram model')
model_sg = Word2Vec(sg=1, size=100, negative=5, window=2,
                    min_count=2, workers=cores, alpha=0.065, min_alpha=0.065)
model_sg.build_vocab([x.words for x in tqdm(X_all_w2v)])
for epoch in range(30):
    model_sg.train(utils.shuffle([x.words for x in tqdm(
        X_all_w2v)]), total_examples=len(X_all_w2v), epochs=1)
    model_sg.alpha -= 0.002
    model_sg.min_alpha = model_sg.alpha

# %%
model_cbow.save('w2v_model_cbow.word2vec')
model_sg.save('w2v_model_sg.word2vec')

# %%
model_cbow = KeyedVectors.load('w2v_model_cbow.word2vec')
model_sg = KeyedVectors.load('w2v_model_sg.word2vec')

# %%
embeddings_index = {}
for w in model_cbow.wv.vocab.keys():
    embeddings_index[w] = np.append(model_cbow.wv[w], model_sg.wv[w])
print('%s word vectors' % len(embeddings_index))

# %%
print('tokenize X_train, X_val and X_test')
tokenizer = Tokenizer(num_words=100000)
tokenizer.fit_on_texts(X_train)
x_train_seq = tokenizer.texts_to_sequences(X_train)
x_train_seq = pad_sequences(x_train_seq, maxlen=35)
x_val_seq = tokenizer.texts_to_sequences(X_val)
x_val_seq = pad_sequences(x_val_seq, maxlen=35)
x_test_seq = tokenizer.texts_to_sequences(X_test)
x_test_seq = pad_sequences(x_test_seq, maxlen=35)

# %%
print('generating embedding matrix')
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
    A = (y_predict * 4).flatten()
    B = y_val.values * 4
    mse = (np.square(A - B)).mean()
    return mse


# %%
tweet_input = Input(shape=(35,), dtype='int32')
tweet_encoder = Embedding(100000, 200, weights=[
                          embedding_matrix], input_length=35, trainable=False)(tweet_input)
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
print('Model summary')
model.summary()

# %%
filepath = "CNN_best_weights.{epoch:02d}-{val_acc:.4f}.hdf5"
checkpoint = ModelCheckpoint(filepath, monitor='val_acc', verbose=1,
                             save_best_only=True, mode='max')
model.fit(x_train_seq, y_train, batch_size=32, epochs=5,
          validation_data=(x_val_seq, y_val), callbacks=[checkpoint])

# %%
best_model = sorted([f for f in os.listdir('.') if 'hdf5' in f])[-1]
model = load_model(best_model)
y_predict = model.predict(x_val_seq)
print('validation mse: %f' % MSE(y_predict, y_val))

# %%
y_predict = model.predict(x_test_seq)
result = y_predict * 4
df_final = pd.DataFrame()
df_final['ID'] = range(len(result))
df_final['sentiment'] = result.flatten()
df_final.to_csv('answer.csv', index=False)
