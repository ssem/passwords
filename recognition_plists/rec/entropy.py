import math
from rec.template import Template

class Find_Entropy(Template):
    def __init__(self):
        self.total = 0
        self.save = {}
        self.total_save = 0

    def parse_word(self, word):
        self.total += 1
        try:
            prob = [ float(word.count(c)) / len(word) for c in dict.fromkeys(list(word)) ]
            entropy = - sum([ p * math.log(p) / math.log(2.0) for p in prob ])
            entropy = abs(entropy / len(word))
            self.total_save += entropy
        except:return
        if entropy in self.save:
            self.save[entropy] += 1
        else:
            self.save[entropy] = 1
