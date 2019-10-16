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

    def get_search(self, args):
        return self.client.get('/search/', args)

    def test_basic_end2end(self):
        get_request = self.get_search({'keywords' : 'foo'})
        assert get_request.status_code == 200
        response = json.loads(get_request.getvalue())
        assert [{'origin': 'Doiran', 'url': 'http://toto.com'}] == response

    def test_missing_comic(self):
        get_request = self.get_search({'keywords' : 'missing_keyword'})
        assert get_request.status_code == 200
        response = json.loads(get_request.getvalue())
        assert [] == response

    def test_several_keywords(self):
        get_request = self.get_search({'keywords' : ['foo', 'bar', 'toto']})
        assert get_request.status_code == 200
        response = json.loads(get_request.getvalue())
        self.assertEqual([
            {
                'origin': 'Doiran',
                'url': 'http://toto.com'
            },
            {
                'origin': 'Doiran',
                'url': 'http://toto.com/2'
            }
        ], response)

