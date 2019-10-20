import datetime
import json
import pytz

from search.models import Comic
from django.test import TestCase, Client
import django.utils


class End2EndTest(TestCase):
    crocodile_date_1 = datetime.datetime(2001, 1, 23, tzinfo=pytz.utc)
    crocodile_date_2 = crocodile_date_1 + datetime.timedelta(days=1)

    db_comics = [
        {"url": "http://toto.com/", "keywords": "foo bar joke", "origin": "Doiran"},
        {"url": "http://toto.com/2", "keywords": "bar joke", "origin": "Doiran"},
        {
            "url": "http://toto.com/3",
            "keywords": "crocodile joke",
            "origin": "Doiran",
            "date": crocodile_date_1,
        },
        {
            "url": "http://toto.com/4",
            "keywords": "another crocodile joke",
            "origin": "Doiran",
            "date": crocodile_date_2,
        },
    ]

    @classmethod
    def setUpTestData(cls):
        for c in cls.db_comics:
            comic_date = (
                django.utils.timezone.now()
                if c.get("date", None) is None
                else c["date"]
            )
            Comic.objects.create(
                url=c["url"],
                keywords=c["keywords"],
                origin=c["origin"],
                date=comic_date,
            )

    def get_search_response(self, args):
        get_request = self.client.get("/search/", args)
        self.assertEquals(get_request.status_code, 200)
        return json.loads(get_request.getvalue())

    def test_missing_comic(self):
        self.assertEqual([], self.get_search_response({"keywords": "missing_keyword"}))

    def test_one_kw_match_many(self):
        self.assertEqual(
            [remove_keywords(self.db_comics[0]), remove_keywords(self.db_comics[1])],
            self.get_search_response({"keywords": "bar"}),
        )

    def test_more_matches_first(self):
        self.assertEqual(
            [remove_keywords(self.db_comics[0]), remove_keywords(self.db_comics[1])],
            self.get_search_response({"keywords": ["foo", "bar"]}),
        )

    def test_more_recent_first(self):
        self.assertEqual(
            [remove_keywords(self.db_comics[3]), remove_keywords(self.db_comics[2])],
            self.get_search_response({"keywords": ["crocodile"]}),
        )


def remove_keywords(dictionnary):
    return {k: v for k, v in dictionnary.items() if k != "keywords" and k != "date"}
