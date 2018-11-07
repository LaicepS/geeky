import os.path
import unittest
import shutil
import tempfile

def extract_words(img_file):
    res = []
    with tempfile.NamedTemporaryFile(delete=False) as tmp_file:
        tmp_file.close()
        cmd = '2> /dev/null tesseract {} stdout > {}'.format(img_file, tmp_file.name)
        os.system(cmd)
        f = open(tmp_file.name)
        for l in f.readlines():
            res += l.strip().split()
        f.close()
    return res

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
            res[gif] = extract_words(os.path.join(dirpath, gif))

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
        self.assertEqual([], result['file1.gif'])
        self.assertIn('file2.gif', result)
        self.assertEqual([], result['file2.gif'])

    def testBasicImg(self):
        shutil.copy('test.gif', self.test_dir.name)
        res = gen_dictionary(self.test_dir)
        self.assertIn('test.gif', res)
        self.assertEqual(['FOO'], res['test.gif'])

    def tearDown(self):
        self.test_dir.cleanup()

