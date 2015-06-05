from rec.template import Template

class Find_Chars(Template):
    def parse_word(self, word):
        for letter in word:
            self.total += 1
            if letter in self.save:
                self.save[letter] += 1
            else:
                self.save[letter] = 1

    def print_data(self, count):
        counter = 0
        response = ''
        returned_data = self.return_data()
        for data in returned_data:
            counter += 1
            if counter < count:
                response += data[0]
        print 'all: %s\n' % response
        counter = 0
        for data in returned_data:
            counter += 1
            if counter < count:
                try:percent = round(float(data[1]) / float(self.return_total()) * 100, 2)
                except:percent = 0
                print '{:<50}{:<10}{:<10}'.format(data[0], data[1], percent)
