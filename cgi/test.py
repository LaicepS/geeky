#!/usr/bin/env python3

import unittest
import cgi
import cgitb; 
cgitb.enable()

import controller

print("Content-type: text/html")
print("")

print("""
<html>
<body>
    Lore ipsum
""")

form = cgi.FieldStorage()
keywords = form["search"].value.split()

comic_ids = get_ids(keywords)

print("""
</body>
</html>
""")


if __name__ == "__main__":
    unittest.main()

# vim: set ft=python

