# %%
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


# %%
def normalize(df, testdata=False):
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
            continue
        text_dict = dict([(word, True) for word in words])
        if (testdata):
            ret.append(text_dict)
        elif (df.sentiment[i] == 4):
            ret.append((text_dict, 'pos'))
        else:
            ret.append((text_dict, 'neg'))
        if ((i + 1) % 50000 == 0):
            print('Tweets %d of %d has been processed' % (i + 1, df.shape[0]))
    return ret


# %%
df = pd.read_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/train.csv')
# df = pd.read_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/ttt.csv')
data = normalize(df)
train, test = train_test_split(data, test_size=0.1)

# %%
len(data)

# %%


def plot_wordcloud(data, color='black'):
    words = ' '.join(data)
    wordcloud = WordCloud(width=1600, height=800, background_color=color,
                          max_font_size=200).generate(words)
    plt.figure(figsize=(12, 10))
    plt.imshow(wordcloud, interpolation="bilinear")
    plt.axis("off")
    plt.show()


# %%
pos_df = df[df.sentiment == 4]['text']
plot_wordcloud(pos_df, 'white')

# %%
neg_df = df[df.sentiment == 0]['text']
plot_wordcloud(neg_df)






# %%
df_clean = pd.read_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/clean_train.csv')
X = df_clean.text
y = df_clean.sentiment
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=1)

# %%
%time
vect = TfidfVectorizer(analyzer="word", ngram_range=(1, 3))
X_dtm = vect.fit_transform(X)

# %%
from sklearn.naive_bayes import MultinomialNB
from sklearn.model_selection import cross_val_score
nb = MultinomialNB(alpha=10)
nb.fit(X_dtm, y)
mse = cross_val_score(estimator=nb, X=X_dtm, y=y, cv=10, scoring='neg_mean_squared_error')
mse.mean()

# %%
from sklearn.linear_model import LogisticRegression
lr = LogisticRegression(C=1.1)
lr.fit(X_dtm, y)
mse = cross_val_score(estimator=lr, X=X_dtm, y=y, cv=10, scoring='neg_mean_squared_error')
mse.mean()

# %%
from sklearn.svm import LinearSVC
svm = LinearSVC(random_state=0)
svm.fit(X_dtm, y)
mse = cross_val_score(estimator=svm, X=X_dtm, y=y, cv=10, scoring='neg_mean_squared_error')
mse.mean()
















# %%
# training_set = nltk.classify.apply_features(extract_features, tweets)
# classifier = nltk.NaiveBayesClassifier.train(training_set)
classifier = nltk.NaiveBayesClassifier.train(train)
# classifier = nltk.NaiveBayesClassifier.train(data)

# %%
result = []
senttt = []
for text_dict, sent in test:
    result.append(classifier.classify(text_dict))
    senttt.append(sent)
A = np.array([0 if i == 'neg' else 4 for i in result])
B = np.array([0 if i == 'neg' else 4 for i in senttt])
mse = (np.square(A - B)).mean()
print('mse:', mse)
print('accuracy:', nltk.classify.util.accuracy(classifier, test))
# classifier.show_most_informative_features()


# %%
df_final = pd.read_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/test.csv')
final = normalize(df_final, testdata=True)
answer = pd.DataFrame()
answer['ID'] = range(len(final))
result = []
for text_dict in final:
    result.append(classifier.classify(text_dict))
answer['Sentiment'] = [0 if i == 'neg' else 4 for i in result]
answer.to_csv(
    '/home/ee904/Desktop/HuaTsai/learn_ds/hw5/answer.csv', index=False)

# %%
df_data = pd.DataFrame()
tttt = []
ssss = []
for i in range(len(data)):
    tttt.append(' '.join(data[i][0]).strip())
    if (data[i][1] == 'pos'):
        ssss.append(4)
    else:
        ssss.append(0)
    # ssss.append(data[i][1])
df_data['text'] = tttt
df_data['sentiment'] = ssss
df_data.to_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/clean_train.csv', index=False)


#%%
%time


#%%
