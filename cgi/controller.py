import unittest

def get_ids(keywords, database):
    ans = {}
    for keyword in keywords:
        for entry in database:
            if keyword in database[entry]:
                if entry not in ans:
                    ans[entry] = [keyword]
                else:
                    ans[entry].append(keyword)

    return ans

class TestGetIds(unittest.TestCase):

    def test_empty(self):
        self.assertEqual({}, get_ids([''], {}))

    def test_keyword_doesnt_match(self):
        self.assertEqual({}, get_ids(['keywordNotFound'], {}))

    def test_basic_match(self):
        self.assertEqual({'comic-id' : ['my-keyword']}, 
                get_ids(['my-keyword'], {'comic-id' : ['my-keyword']}))

    def test_several_matches(self):
        self.assertEqual(
                {'comic-id1' : ['keyword2'], 'comic-id2' : ['keyword1']},
                get_ids(['keyword1', 'keyword2'], 
                    { 'comic-id1' : 'keyword2', 'comic-id2' : 'keyword1' }))

    def test_one_keyword_matches(self):
        self.assertEqual(
                {'comic-id1' : ['keyword2']},
                get_ids(['keyword1', 'keyword2'],
                    { 'comic-id1' : 'keyword2' }))

def parse_comics_db_line(line):
    if line == '':
        return None

    first_split = line.split(':')

    if len(first_split) != 2:
        return None

    id_ = first_split[0]
    keywords = first_split[1].split(',')
    return id_,keywords


class TestParseDbLine(unittest.TestCase):
    def test_parse_empty(self):
        self.assertEqual(None, parse_comics_db_line(""))

    def test_parse_gibberish(self):
        self.assertEqual(None, parse_comics_db_line("not a valid syntax"))

    def test_parse_minimal(self):
        self.assertEqual(('comic_id', ['keyword']), 
                parse_comics_db_line('comic_id:keyword'))

    def test_parse_multiple_keywords(self):
        self.assertEqual(('comic_id', ['keyword1', 'keyword2']), 
                parse_comics_db_line('comic_id:keyword1,keyword2'))



def get_all_comics(lines):
    ans = {}
    for line in lines:
        pair = parse_comics_db_line(line)
        if pair == None:
            continue

        if pair[0] not in ans:
            ans[pair[0]] = pair[1]
        else:
            for kw in pair[1]:
                if kw not in ans[pair[0]]:
                    ans[pair[0]].append(kw)

    return ans

class TestInsertDbLines(unittest.TestCase):
    def test_double_insertion(self):
        self.assertEqual({'same_comic_id' : ['same_keyword']},
                get_all_comics(['same_comic_id:same_keyword',
                'same_comic_id:same_keyword'
                ]))

    def test_multiple_occurence(self):
        self.assertEqual({'comic_id' : ['keyword1', 'keyword2']},
                get_all_comics(['comic_id:keyword1', 'comic_id:keyword2']))
