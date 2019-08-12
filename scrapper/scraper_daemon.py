import sys
import unittest
import shutil
import io

import scrapy
from scrapy.crawler import CrawlerProcess

from PIL import Image

import pytesseract

from  dilbert.dilbert.spiders.comic_spider import ComicsSpider

import pymysql
pymysql.install_as_MySQLdb()
import MySQLdb as mdb

# return is in the format 1YYMMDD (1 stands for dilbert)
def id_from_url(url):
    splits = url.split('strip/')
    if len(splits) != 2:
        raise Exception("Unknown URL format")

    date = splits[1]

    periods = date.split('-')
    if len(periods) != 3:
        raise Exception("Unknown URL format")

    return 1*1000000 \
            + (int(periods[0])%100)*10000 \
            + int(periods[1])*100 \
            + int(periods[2])

class IdFromUrlTest(unittest.TestCase):
    def testIdFromUrl(self):
        self.assertEqual(1000124,
        id_from_url('https://dilbert.com/strip/2000-01-24'))

    def testInvalidUrl(self):
        with self.assertRaises(Exception):
            id_from_url('invalid_url')
        with self.assertRaises(Exception):
            id_from_url('dilbert.com/strip/invalide_date')


def to_ascii(s):
    return ''.join([c if ord(c) < 128 else ' ' for c in s])

def trim_whitespaces(s):
    return ' '.join(s.split())


class TestTrimWhiteSpaces(unittest.TestCase):
    def testOneWord(self):
        self.assertEqual("foo", trim_whitespaces("foo"))

    def testOneWordWithSpaces(self):
        self.assertEqual("foo", trim_whitespaces(" \nfoo  \n"))

    def testEmptyString(self):
        self.assertEqual("", trim_whitespaces(""))

    def testSeveralWords(self):
        self.assertEqual("several words to test",
                trim_whitespaces("\n  several  \twords \n\n to test"))

def extract_text(img_path):
    return trim_whitespaces(
            to_ascii(pytesseract.image_to_string(Image.open(img_path))))
    

class SpiderListener:
    def __init__(self, dbcursor):
        self.dbcursor = dbcursor

    def onImg(self, url, img_data):
        try:
            img_id = id_from_url(url)
            img_text = extract_text(io.BytesIO(img_data))
            self.dbcursor.execute(
                    "insert into comics(id, origin, keywords) values(%s, 'dilbert', %s)",
                    (img_id, img_text))
            return True

        except mdb.Error or mdb.DatabaseError or mdb.DataError as e:
            print("Error: {}".format(e), file=sys.stderr)
            return False



def main():
    try:
        conn = mdb.connect(host = 'localhost', user = 'dorian', 
                password = 'moijemon', database='geeky')
        cursor = conn.cursor()

        process = CrawlerProcess({'USER_AGENT' : 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)',
            'LOG_LEVEL' : 'WARNING'})
        spiderListener = SpiderListener(cursor)
        process.crawl(ComicsSpider, spiderListener)
        process.start()
        conn.commit()

    except mdb.Error or mdb.DatabaseError or mdb.DataError as e:
        print("Error: {}".format(e), file=sys.stderr)
        return 1
    except Exception as e:
        print(e)
        return 2
    finally:
        conn.close()



if __name__ == '__main__':
    main()
