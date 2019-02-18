import shutil
import io

import scrapy
from scrapy.crawler import CrawlerProcess

from PIL import Image

import pytesseract

from  dilbert.dilbert.spiders.comic_spider import ComicsSpider

import MySQLdb as mdb

def extract_text(img_path):
    return pytesseract.image_to_string(Image.open(img_path))
    

class SpiderListener:
    def onImg(self, img_data):
        print(extract_text(io.BytesIO(img_data)))
        return True


def main():
    try:
        conn = mdb.connect(host = 'localhost', user = 'dorian', 
                password = 'moijemon', database='geeky')
        cursor = conn.cursor()

        process = CrawlerProcess({'USER_AGENT' : 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)',
            'LOG_LEVEL' : 'WARNING'})
        spiderListener = SpiderListener()
        process.crawl(ComicsSpider, spiderListener)
        process.start()

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
