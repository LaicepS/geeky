import unittest

def get_ids(keywords, database):
    ans = {}
    for keyword in keywords:
        for entry in database:
            if keyword in database[entry]:
                if entry not in ans.keys():
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

