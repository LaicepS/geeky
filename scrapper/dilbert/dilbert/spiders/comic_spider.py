import scrapy
import shutil
import tempfile
import urllib

def cleanUp(url):
    if url[0:2] == '//':
        return 'http:' + url
    else:
        return url

class ComicsSpider(scrapy.Spider):
    name = 'Comics Spider'
    start_urls = [ 'http://dilbert.com/strip/2018-11-05' ]
    counter = 0
    listener = None

    def __init__(self, listener = None):
        self.listener = listener

    def parse(self, response):
        if self.counter >= 1:
            return None

        if not self.handleResponse(response):
            return None

        previous_img_link = response.css('div.nav-left a::attr(href)').extract_first()
        if previous_img_link is not None:
            self.counter += 1
            yield response.follow(previous_img_link, self.parse)
        else:
            return None

    def handleResponse(self, response):
        if self.listener == None:
            return False

        img_url = response.css('img.img-comic::attr(src)').extract_first()
        img_url = cleanUp(img_url)
        img = urllib.request.urlopen(img_url)
        return self.listener.onImg(response.url, img.read())

    def getFileName(self):
        return 'dilbert-{}.gif'.format(self.counter)
