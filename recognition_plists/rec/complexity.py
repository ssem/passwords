import string
from rec.template import Template

class Find_Complexity(Template):
    def parse_word(self, word):
        self.total +=1
        key = {}
        for letter in word:
            if letter in string.ascii_lowercase:
                key['lower_ascii'] = 1
            elif letter in string.digits:
                key['digit'] = 1
            elif letter in string.ascii_uppercase:
                key['upper_ascii'] = 1
            elif letter in string.punctuation + ' ':
                key['punctuation'] = 1
            else:
                key['other'] = 1
        key = ', '.join(key)
        if key in self.save:
            self.save[key] += 1
        else:
            self.save[key] = 1
