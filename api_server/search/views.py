from django.shortcuts import render

from django.http import HttpResponse

import json

def index(request):
    d = { "toto" : "tata" }
    return HttpResponse(json.dumps(d))
