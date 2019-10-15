from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic

def index(request):
    request_params = request.GET
    query = Comic.objects.filter(keywords__contains=request_params['keywords'])
    try:
        comic = query.get()
    except Comic.DoesNotExist:
        comic = None

    return HttpResponse(serialize_comic(comic))

def serialize_comic(comic):
    if comic is None:
        return json.dumps({})

    return json.dumps({'origin': comic.origin, 'url': comic.url})
