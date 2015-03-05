#!/usr/bin/env python
from rec.template import Template

class Find_Top_Passwords(Template):
    def parse_word(self, word):
        self.total += 1
        if word in self.save:
            self.save[word] += 1
        else:
            self.save[word] = 1
