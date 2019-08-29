# %%
import nltk.classify.util
from nltk.classify import NaiveBayesClassifier
import pandas as pd
import csv
import re
import collections
from nltk.corpus import stopwords
from nltk.stem import LancasterStemmer, WordNetLemmatizer


# %%
def remove_punctuation(words):
    new_words = []
    for word in words:
        new_word = re.sub(r'[^\w\s]', '', word)
        if new_word != '':
            new_words.append(new_word)
    return new_words


def remove_numbers(words):
    new_words = []
    for word in words:
        if not word.isdigit():
            new_words.append(word)
    return new_words


def remove_stopwords(words):
    new_words = []
    for word in words:
        if word not in stopwords.words('english'):
            new_words.append(word)
    return new_words


def stem_words(words):
    stemmer = LancasterStemmer()
    stems = []
    for word in words:
        stem = stemmer.stem(word)
        stems.append(stem)
    return stems


def lemmatize_verbs(words):
    lemmatizer = WordNetLemmatizer()
    lemmas = []
    for word in words:
        lemma = lemmatizer.lemmatize(word, pos='v')
        lemmas.append(lemma)
    return lemmas


def normalize_words(words):
    new_words = remove_punctuation(words)
    new_words = remove_numbers(new_words)
    new_words = remove_stopwords(new_words)
    new_words = stem_words(new_words)
    new_words = lemmatize_verbs(new_words)
    return new_words


# %%
pos_feats = []
neg_feats = []
with open('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/train.csv', newline='') as csvfile:
    rows = csv.reader(csvfile)
    for row in rows:
        if (row[0] == 'sentiment'):
            continue
        words = nltk.word_tokenize(row[5].lower())
        words = normalize_words(words)
        words_feats = dict([(word, True) for word in words])
        if (row[0] == '0'):
            neg_feats.append((words_feats, 'neg'))
        else:
            pos_feats.append((words_feats, 'pos'))

# %%
test_feats = []
with open('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/test.csv', newline='') as csvfile:
    rows = csv.reader(csvfile)
    for row in rows:
        if (row[0] == 'id'):
            continue
        words = nltk.word_tokenize(row[4].lower())
        words = normalize_words(words)
        words_feats = dict([(word, True) for word in words])
        test_feats.append(words_feats)

# %% 
negcutoff = int(len(neg_feats) * 4 / 5)
poscutoff = int(len(pos_feats) * 4 / 5)

# %% varify
train_feats = neg_feats[:negcutoff] + pos_feats[:poscutoff]
test_feats = neg_feats[negcutoff:] + pos_feats[poscutoff:]
print('train on %d instances, test on %d instances' % (len(train_feats), len(test_feats)))

# %% homework
train_feats = neg_feats + pos_feats

# %%
classifier = NaiveBayesClassifier.train(train_feats)

# %%
# refsets = collections.defaultdict(set)
# testsets = collections.defaultdict(set)
# for i, (feats, label) in enumerate(test_feats):
#     refsets[label].add(i)
#     observed = classifier.classify(feats)
#     testsets[observed].add(i)
print('accuracy:', nltk.classify.util.accuracy(classifier, test_feats))
# print('pos precision:', nltk.metrics.precision(refsets['pos'], testsets['pos']))
# print('pos recall:', nltk.metrics.recall(refsets['pos'], testsets['pos']))
# print('neg precision:', nltk.metrics.precision(refsets['neg'], testsets['neg']))
# print('neg recall:', nltk.metrics.recall(refsets['neg'], testsets['neg']))
classifier.show_most_informative_features()
# 0.77
# 0.76


# %%
def senttonum(sent):
    if (sent == 'pos'):
        return 4
    else:
        return 0


answer = pd.DataFrame()
answer['ID'] = range(len(test_feats))
result = []
for i in range(len(test_feats)):
    result.append(classifier.classify(test_feats[i]))
answer['Sentiment'] = [senttonum(i) for i in result]
answer.to_csv('/home/ee904/Desktop/HuaTsai/learn_ds/hw5/answer.csv', index=False)

# %%
