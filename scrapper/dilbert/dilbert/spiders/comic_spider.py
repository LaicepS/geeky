import scrapy
import shutil
import tempfile
import urllib.response

class ComicsSpider(scrapy.Spider):
    name = 'Comics Spider'
    start_urls = [ 'http://dilbert.com/strip/2018-11-05' ]
    counter = 0

    def parse(self, response):
        if self.counter >= 10:
            return None

        self.copyImageOnDisk(response)

        previous_img_link = response.css('div.nav-left a::attr(href)').extract_first()
        if previous_img_link is not None:
            self.counter += 1
            yield response.follow(previous_img_link, self.parse)
        else:
            return None

    def copyImageOnDisk(self, response):
        img_url = response.css('img.img-comic::attr(src)').extract_first()
        img = urllib.request.urlopen(img_url)
        dst_file = open(self.getFileName(), 'wb')
        shutil.copyfileobj(img, dst_file)


    def getFileName(self):
        return 'dilbert-{}.gif'.format(self.counter)
