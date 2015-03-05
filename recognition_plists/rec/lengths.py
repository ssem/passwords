from rec.template import Template

class Find_Lengths(Template):
    def parse_word(self, word):
        self.total += 1
        length = len(word)
        if length in self.save:
            self.save[length] += 1
        else:
            self.save[length] = 1
