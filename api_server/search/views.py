from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic

def index(request):
    request_params = request.GET
    query = Comic.objects.filter(keywords__contains=request_params['keywords'])
    comic = get_object_or_404(query)
    return HttpResponse(json.dumps({'origin': comic.origin, 'url': comic.url}))
