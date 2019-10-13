import json

from django.test import TestCase, Client

class End2EndTest(TestCase):
    def test_basic_end2end_test(self):
        client = Client()
        get_request = client.get('/search/')
        response = json.loads(get_request.getvalue())
