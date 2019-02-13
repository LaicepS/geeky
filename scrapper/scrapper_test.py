import os

import scrapy
from scrapy.crawler import CrawlerProcess

import unittest

from  dilbert.dilbert.spiders.comic_spider import ComicsSpider

class SpiderListener:
    testObject = None

    def __init__(self, testObject):
        self.testObject = testObject

    def onImg(self, img_file):
        self.testObject.assertTrue(img_file.readable())


class ComicSpiderTest(unittest.TestCase):
    def test_scrapper(self):
        process = CrawlerProcess({'USER_AGENT' : 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)',
            'LOG_ENABLED' : False})
        spiderListener = SpiderListener(self)
        process.crawl(ComicsSpider, spiderListener)
        process.start()
