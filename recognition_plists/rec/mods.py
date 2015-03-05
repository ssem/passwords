import os
import rec
import operator
import enchant
from rec.template import Template

class Find_Mods(Template):
    def __init__(self):
        self.total = 0
        self.d = enchant.DictWithPWL(
            "en", os.path.dirname(rec.__file__) + '/spell_check.txt')
        self.save = {}

    def parse_word(self, word, baseword):
        if word != baseword:
            self.total += 1
            if baseword in self.save:
                self.save[baseword].append(word)
            else:
                self.save[baseword] = [word]

    def parse_list(self, mylist):
        for word in mylist:
            word = word.rstrip('\r\n')
            try:basewords = self.d.suggest(word)[:2]
            except:continue
            for baseword in basewords:
                self.parse_word(word, baseword)

    def return_data(self):
        sorted_values = {}
        for key in self.save:
            sorted_values[', '.join([key] + self.save[key][:2])] = len(self.save[key])
        sorted_values = sorted(sorted_values.items(), key=operator.itemgetter(1))
        sorted_values.reverse()
        return sorted_values
