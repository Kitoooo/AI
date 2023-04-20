from sklearn.base import BaseEstimator, ClassifierMixin
import numpy as np
import pandas as pd


class myNBC(BaseEstimator, ClassifierMixin):
    def __init__(self):
        self.classes = None
        self.freqs = []

        self.classes_proba = None
        self.freqs_proba = []

        self.probabilities = []

    def fit(self, X, y, user_laplace=False):
        self.classes = pd.DataFrame(y, columns=["classes"]).groupby("classes").size()
        self.classes_proba = self.classes.div(len(y))
        for column in X.T:
            df = pd.DataFrame(np.vstack((column, y)).T, columns=["attrib", "class"])
            freqs = df.groupby(["attrib", "class"]).size().unstack(fill_value=0)
            if user_laplace:
                freqs += 1
                freqs_proba = freqs.div(self.classes + len(np.unique(column)), axis=1)
            else:
                freqs_proba = freqs.div(self.classes, axis=1)
            self.freqs.append(freqs)
            self.freqs_proba.append(freqs_proba)

    def predict(self, X):
        self.predict_proba(X)
        return self.classes.index[np.argmax(self.probabilities)]

    def predict_proba(self, X):
        self.probabilities = []
        for class_ in self.classes.index:
            proba = self.classes_proba.loc[class_]
            for j, attrib in enumerate(X):
                test = self.freqs_proba[j].loc[attrib, class_]
                proba *= test
            self.probabilities.append(proba)
        return [x / sum(self.probabilities) for x in self.probabilities]
