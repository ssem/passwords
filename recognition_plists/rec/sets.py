import string
from rec.template import Template

class Find_Sets(Template):
    def __init__(self):
        self.total = 0
        self.save = {'lower_ascii':0, 'digit':0, 'upper_ascii':0, 'punctuation':0, 'other':0}

    def parse_word(self, word):
        for letter in word:
            self.total += 1
            if letter in string.ascii_lowercase:
                self.save['lower_ascii'] += 1
            elif letter in string.digits:
                self.save['digit'] += 1
            elif letter in string.ascii_uppercase:
                self.save['upper_ascii'] += 1
            elif letter in string.punctuation + ' ':
                self.save['punctuation'] += 1
            else:
                self.save['other'] += 1
