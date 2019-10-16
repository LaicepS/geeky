import json

from search.models import Comic
from django.test import TestCase, Client


class End2EndTest(TestCase):
    @classmethod
    def setUpTestData(cls):
        db_comics = [
            {"url": "http://toto.com", "keywords": "foo bar joke", "origin": "Doiran"},
            {"url": "http://toto.com/2", "keywords": "bar joke", "origin": "Doiran"},
        ]

        for c in db_comics:
            Comic.objects.create(
                url=c["url"], keywords=c["keywords"], origin=c["origin"]
            )

    def get_search_response(self, args):
        get_request = self.client.get("/search/", args)
        self.assertEquals(get_request.status_code, 200)
        return json.loads(get_request.getvalue())

    def test_basic_end2end(self):
        self.assertEqual(
            [{"origin": "Doiran", "url": "http://toto.com"}],
            self.get_search_response({"keywords": "foo"}),
        )

    def test_missing_comic(self):
        self.assertEqual([], self.get_search_response({"keywords": "missing_keyword"}))

    def test_several_keywords(self):
        self.assertEqual(
            [
                {"origin": "Doiran", "url": "http://toto.com"},
                {"origin": "Doiran", "url": "http://toto.com/2"},
            ],
            self.get_search_response({"keywords": ["foo", "bar", "toto"]}),
        )

    def test_one_kw_match_many(self):
        self.assertEqual(
            [
                {"origin": "Doiran", "url": "http://toto.com"},
                {"origin": "Doiran", "url": "http://toto.com/2"},
            ],
            self.get_search_response({"keywords": "bar"}),
        )

    def test_more_matches_first(self):
        self.assertEqual(
            [
                {"origin": "Doiran", "url": "http://toto.com"},
                {"origin": "Doiran", "url": "http://toto.com/2"},
            ],
            self.get_search_response({"keywords": ["foo", "bar"]}),
        )
