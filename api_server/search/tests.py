import json

from search.models import Comic
from django.test import TestCase, Client


class End2EndTest(TestCase):
    db_comics = [
        {"url": "http://toto.com/", "keywords": "foo bar joke", "origin": "Doiran"},
        {"url": "http://toto.com/2", "keywords": "bar joke", "origin": "Doiran"},
    ]

    @classmethod
    def setUpTestData(cls):
        for c in cls.db_comics:
            Comic.objects.create(
                url=c["url"], keywords=c["keywords"], origin=c["origin"]
            )

    def get_search_response(self, args):
        get_request = self.client.get("/search/", args)
        self.assertEquals(get_request.status_code, 200)
        return json.loads(get_request.getvalue())

    def test_basic_end2end(self):
        self.assertEqual(
            [remove_keywords(self.db_comics[0])],
            self.get_search_response({"keywords": "foo"}),
        )

    def test_missing_comic(self):
        self.assertEqual([], self.get_search_response({"keywords": "missing_keyword"}))

    def test_one_kw_match_many(self):
        self.assertEqual(
            [
                remove_keywords(self.db_comics[0]),
                remove_keywords(self.db_comics[1]),
            ],
            self.get_search_response({"keywords": "bar"}),
        )

    def test_more_matches_first(self):
        self.assertEqual(
            [
                remove_keywords(self.db_comics[0]),
                remove_keywords(self.db_comics[1]),
            ],
            self.get_search_response({"keywords": ["foo", "bar"]}),
        )

def remove_keywords(dictionnary):
    return { k:v for k, v in dictionnary.items() if k != "keywords" }
