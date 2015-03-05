import string
from rec.template import Template

class Find_Patterns(Template):
    def parse_word(self, word):
        self.total += 1
        pattern = ''
        for letter in word:
            if letter in string.ascii_lowercase:
                pattern += '?l'
            elif letter in string.digits:
                pattern += '?d'
            elif letter in string.ascii_uppercase:
                pattern += '?u'
            elif letter in string.punctuation:
                pattern += '?s'
            else:
                pattern += letter
        if pattern in self.save:
            self.save[pattern] += 1
        else:
            self.save[pattern] = 1
