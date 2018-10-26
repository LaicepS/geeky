#!/usr/bin/env python3

import cgi
import cgitb; cgitb.enable()
import os
import unittest

import controller

def get_html():
    form = cgi.FieldStorage()
    keywords = form["search"].value.split()
    res = '<ul>'
    for comic in get_comic_list(keywords):
        res += "<li>" + get_comic_html(comic) + "</li>"
    res += "</ul>"
    return res

def get_comic_html(comic):
    return "<img src='/img/" + comic + ".png' alt='" + comic + "'>"

def get_comic_list(keywords):
    db_file = open('/home/dorian/geeky/data/db.txt')
    db = controller.get_all_comics(db_file.read().split(os.linesep))
    db_file.close()
    return controller.get_ids(keywords, db)

class TestGetComicList(unittest.TestCase):
    def test_crash(self):
        self.assertEqual({}, get_comic_list('foo'))
        self.assertTrue('smbc-001' in get_comic_list(['alligator']))

def main():
    print("Content-type: text/html")
    print("")

    print("""
    <html>
    <body>
    """)

    print(get_html())


    print("""
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()

# vim: set ft=python

