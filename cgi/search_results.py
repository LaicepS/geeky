#!/usr/bin/env python3

import cgi
import cgitb

cgitb.enable()
import os
import unittest
import tempfile

import controller

DEFAULT_DB = "/home/dorian/geeky/data/db.txt"


def get_html():
    form = cgi.FieldStorage()
    keywords = form["search"].value.split()
    res = "<ul>"
    for comic in get_comic_list(keywords, DEFAULT_DB):
        res += "<li>" + get_comic_html(comic) + "</li>"
    res += "</ul>"
    return res


def get_comic_html(comic):
    return "<img src='/img/" + comic + ".png' alt='" + comic + "'>"


def get_comic_list(keywords, database_path):
    with open(database_path) as db_file:
        db = controller.get_all_comics(db_file.read().splitlines())
        return controller.get_ids(keywords, db)


class TestGetComicList(unittest.TestCase):
    def setUp(self):
        tmp_file = tempfile.NamedTemporaryFile(mode="w+t", delete=False)
        print("smbc-001:alligator", file=tmp_file)
        tmp_file.close()
        self.test_db = tmp_file.name

    def test_keyword_not_found(self):
        self.assertEqual({}, get_comic_list("foo", self.test_db))

    def test_simple_keyword(self):
        self.assertIn("smbc-001", get_comic_list(["alligator"], self.test_db))

    def tearDown(self):
        os.remove(self.test_db)


def main():
    print("Content-type: text/html")
    print("")

    print(
        """
    <html>
    <body>
    """
    )

    print(get_html())

    print(
        """
    </body>
    </html>
    """
    )


if __name__ == "__main__":
    main()

# vim: set ft=python
