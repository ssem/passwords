import string
import operator
from rec.patterns import Find_Patterns

class Find_Time_And_Patterns():
    def __init__(self, min=4, max=100):
        self.min = min
        self.max = max
        self.pattern = Find_Patterns()
        self.patterns = {}

    def parse_word(self, word):
        self.pattern.parse_word(word)

    def parse_list(self, mylist):
        for word in mylist:
            self.pattern.parse_word(word.rstrip('\r\n'))

    def return_total(self):
        return None

    def return_data(self):
        ranked_patterns = {}
        for pattern in self.pattern.return_data():
            if len(pattern[0]) >= self.min and len(pattern[0]) <= self.max:
                c = 26 ** pattern[0].count('?l') * \
                10 ** pattern[0].count('?d') * \
                26 ** pattern[0].count('?u') * \
                33 ** pattern[0].count('?s')
                ranked_patterns[float(pattern[1]) / float(c)] = [pattern[0], pattern[1], c]
        sorted_values = sorted(ranked_patterns.items(), key=operator.itemgetter(0))
        sorted_values.reverse()
        return sorted_values

    def print_data(self, count):
        counter = 0
        for data in self.return_data():
            counter += 1
            if counter < count:
                print '{:<50}{:<10}{:<10}'.format(data[1][0], data[1][1], data[1][2])
