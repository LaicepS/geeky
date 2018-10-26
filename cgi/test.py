#!/usr/bin/env python3

import unittest
import cgi
import cgitb; 
cgitb.enable()

import controller

def get_html():
    form = cgi.FieldStorage()
    keywords = form["search"].value.split()

def get_comic_list(keywords):
    db_file = open('/home/dorian/geeky/data/db.txt')
    db = controller.get_all_comics(db_file)
    db_file.close()
    return controller.get_ids(keywords, db)

class TestGetComicList(unittest.TestCase):
    def test_crash(self):
        get_comic_list('foo')

def main():
    print("Content-type: text/html")
    print("")

    print("""
    <html>
    <body>
        Lore ipsum
    """)

    print(get_html())


    print("""
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()

# vim: set ft=python

