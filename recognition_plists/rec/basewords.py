import os
import rec
import enchant
from rec.template import Template

class Find_Basewords(Template):
    def __init__(self):
        self.total = 0
        self.d = enchant.DictWithPWL(
            "en", os.path.dirname(rec.__file__) + '/spell_check.txt')
        self.save = {}

    def parse_word(self, word):
        try:suggestions = self.d.suggest(word)[:2]
        except:return
        for s in suggestions:
            self.total += 1
            if s in self.save:
                self.save[s] += 1
            else:
                self.save[s] = 1
