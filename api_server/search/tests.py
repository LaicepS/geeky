import json

from search.models import Comic
from django.test import TestCase, Client

class End2EndTest(TestCase):
    @classmethod
    def setUpTestData(cls):
        Comic.objects.create(
            url = 'http://toto.com',
            keywords = 'foo bar joke',
            origin = 'Doiran',
        )

        Comic.objects.create(
            url = 'http://toto.com/2',
            keywords = 'bar joke',
            origin = 'Doiran',
        )

    def get_search_response(self, args):
        get_request = self.client.get('/search/', args)
        self.assertEquals(get_request.status_code, 200)
        return json.loads(get_request.getvalue())

    def test_basic_end2end(self):
        self.assertEqual([{'origin': 'Doiran', 'url': 'http://toto.com'}],
                self.get_search_response({'keywords' : 'foo'}))

    def test_missing_comic(self):
        self.assertEqual([], self.get_search_response({'keywords' : 'missing_keyword'}))

    def test_several_keywords(self):
        self.assertEqual([
            {
                'origin': 'Doiran',
                'url': 'http://toto.com'
            },
            {
                'origin': 'Doiran',
                'url': 'http://toto.com/2'
            }
        ], self.get_search_response({'keywords' : ['foo', 'bar', 'toto']}))

    def test_one_kw_match_many(self):
        self.assertEqual([
            {
                'origin': 'Doiran',
                'url': 'http://toto.com'
            },
            {
                'origin': 'Doiran',
                'url': 'http://toto.com/2'
            }], self.get_search_response({'keywords': 'bar'}))
