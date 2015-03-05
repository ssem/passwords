import os
import rec
import enchant
from rec.template import Template

class Find_Suffixes(Template):
    def __init__(self):
        self.total = 0
        self.d = enchant.DictWithPWL(
            "en", os.path.dirname(rec.__file__) + '/spell_check.txt')
        self.save = {}

    def parse_word(self, word, baseword):
        index = word.lower().find(baseword)
        if index >= 0:
            suffix = word[index + len(baseword):]
            if len(suffix) > 0:
                self.total += 1
                if suffix in self.save:
                    self.save[suffix] += 1
                else:
                    self.save[suffix] = 1

    def parse_list(self, mylist):
        for word in mylist:
            word = word.rstrip('\r\n')
            try:basewords = self.d.suggest(word)[:2]
            except:continue
            for baseword in basewords:
                self.parse_word(word, baseword)
