#!/usr/bin/env python
import re
from rec.template import Template

class Find_Years(Template):
    def parse_word(self, word):
        for year in re.findall("\d{4}", word):
            self.total += 1
            if year in self.save:
                self.save[year] += 1
            else:
                self.save[year] = 1
