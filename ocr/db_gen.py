import os.path
import unittest
import shutil
import tempfile


def gen_dictionary(rows):
    res = {}
    for row in rows:
        for word in row[2].split():
            if word not in res:
                res[word] = [row[0]]
            else:
                res[word].append(row[0])

    return res


class testDbGen(unittest.TestCase):
    def testSmallDb(self):
        self.assertEqual(
            {"funny": [123], "dilbert": [123], "joke": [123]},
            gen_dictionary([(123, "dilbert", "funny dilbert joke")]),
        )

    def testTwoRowsDB(self):
        self.assertEqual(
            {
                "funny": [123, 234],
                "dilbert": [123],
                "joke": [123, 234],
                "another": [234],
            },
            gen_dictionary(
                [
                    (123, "dilbert", "funny dilbert joke"),
                    (234, "dilbert", "another funny joke"),
                ]
            ),
        )
