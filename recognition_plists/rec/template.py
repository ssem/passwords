import operator

class Template():
    def __init__(self):
        self.total = 0
        self.save = {}

    def parse_word(self, word):
        pass

    def add_word(self, word):
        self.total += 1
        if word in self.save:
            self.save[word] += 1
        else:
            self.save[word] = 1

    def parse_list(self, mylist):
        for word in mylist:
            self.parse_word(word.rstrip('\r\n'))

    def return_total(self):
        return self.total

    def return_data(self):
        sorted_values = sorted(self.save.items(), key=operator.itemgetter(1))
        sorted_values.reverse()
        return sorted_values

    def print_data(self, count):
        counter = 0
        for data in self.return_data():
            counter += 1
            if counter < count:
                try:percent = round(float(data[1]) / float(self.return_total()) * 100, 2)
                except:percent = 0
                print '{:<50}{:<10}{:<.2f}%'.format(data[0], data[1], percent)

    def print_html(self, count):
        counter = 0
        for data in self.return_data():
            counter += 1
            if counter < count:
                try:percent = round(float(data[1]) / float(self.return_total()) * 100, 2)
                except:percent = 0
                print '<tr>'
                print '\t<td>%s</td>' % data[0]
                print '\t<td>%s</td>' % data[1]
                print '\t<td>%s</td>' % percent
                print '</tr>'
