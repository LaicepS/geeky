import os.path
import unittest
import shutil
import tempfile

# test easy file return easy text
# test multiple file should return the coresponding dic

def gen_dictionary(dir):
    return {}

class testDbGen(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.TemporaryDirectory()

    def testEmptyDir(self):
        self.assertEqual({}, gen_dictionary(self.test_dir))

    def testEmptyFiles(self):
        for file_name in ['file1', 'file2']:
            file_path = os.path.join(self.test_dir.name, file_name)
            f = open(file_path, "wb")
            f.write(''.encode())
            f.close()

        self.assertEqual({}, gen_dictionary(self.test_dir))

    def testBasicImg(self):
        shutil.copy('test.gif', self.test_dir.name)
        res = gen_dictionary(self.test_dir)
        self.assertTrue('test.gif' in res)
        self.assertEquals('foo', res['test.gif'])

    def tearDown(self):
        self.test_dir.cleanup()

