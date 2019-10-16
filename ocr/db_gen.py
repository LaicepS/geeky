import os.path
import unittest
import shutil
import tempfile


def deduplicate(word_list):
    return set(word_list)


class TestDeduplicate(unittest.TestCase):
    def test_empty_deduplicatoin(self):
        self.assertEqual(set(), deduplicate([]))

    def test_basic_deduplicate(self):
        self.assertEqual({"foo", "bar"}, deduplicate({"foo", "bar", "foo"}))


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
