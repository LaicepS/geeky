import os.path
import unittest
import shutil
import tempfile

def extract_words(img_file):
    res = []
    cmd = 'tesseract {} {}'.format(img_file, '/tmp/toto')
    os.system(cmd)
    f = open('/tmp/toto.txt')
    for l in f.readlines():
        res += l.strip().split()
    f.close()
    return res

# test simple file should return simple text
class ExtractWordsTests(unittest.TestCase):
    # test empty file should return nothing
    def testEmptyFile(self):
        with tempfile.NamedTemporaryFile() as empty_file:
            self.assertEqual([], extract_words(empty_file.name))

    def testSimpleFile(self):
        self.assertEqual(['FOO'], extract_words('test.gif'))


def gen_dictionary(dir):
    res = {}
    for dirpath, dirnames, filenames in os.walk(dir.name, followlinks=False):
        gif_filenames = filter(lambda f: f.lower().endswith('.gif'), filenames)
        for gif in gif_filenames:
            words = extract_words(gif)
            res[gif] = words

    return res

class testDbGen(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.TemporaryDirectory()

    def testEmptyDir(self):
        self.assertEqual({}, gen_dictionary(self.test_dir))

    def testEmptyFiles(self):
        for file_name in ['file1.gif', 'file2.gif']:
            file_path = os.path.join(self.test_dir.name, file_name)
            f = open(file_path, "wb")
            f.write(''.encode())
            f.close()

        result = gen_dictionary(self.test_dir)
        self.assertIn('file1.gif', result)
        self.assertEquals([], result['file1.gif'])
        self.assertIn('file2.gif', result)
        self.assertEquals([], result['file2.gif'])

    def testBasicImg(self):
        shutil.copy('test.gif', self.test_dir.name)
        res = gen_dictionary(self.test_dir)
        self.assertTrue('test.gif' in res)
        self.assertEqual(['FOO'], res['test.gif'])

    def tearDown(self):
        self.test_dir.cleanup()

