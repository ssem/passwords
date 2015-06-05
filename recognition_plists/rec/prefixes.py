import os
import rec
import enchant
from rec.template import Template

class Find_Prefixes(Template):
    def __init__(self):
        self.total = 0
        try:
            self.d = enchant.DictWithPWL(
                "en", os.path.dirname(rec.__file__) + '/spell_check.txt')
        except:
            exit('\n\tsudo is required to read site-packages/recognition/rec/spell_check.txt\n')
        self.save = {}

    def parse_word(self, word, baseword):
        index = word.lower().find(baseword)
        if index > 0:
            self.total += 1
            prefix = word[:index]
            if prefix in self.save:
                self.save[prefix] += 1
            else:
                self.save[prefix] = 1

    def parse_list(self, mylist):
        for word in mylist:
            word = word.rstrip('\r\n')
            try:basewords = self.d.suggest(word)[:2]
            except:continue
            for baseword in basewords:
                self.parse_word(word, baseword)
