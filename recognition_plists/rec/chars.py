from rec.template import Template

class Find_Chars(Template):
    def parse_word(self, word):
        for letter in word:
            self.total += 1
            if letter in self.save:
                self.save[letter] += 1
            else:
                self.save[letter] = 1
