from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic

def index(request):
    request_params = request.GET
    query = Comic.objects.filter(keywords__contains=request_params['keywords'])
    comic_list = []
    try:
        comic_list.append(query.get())
    except Comic.DoesNotExist:
        pass

    return HttpResponse(json.dumps([serialize_comic(comic) for comic in comic_list]))

def serialize_comic(comic):
    if comic is None:
        return {}

    return {'origin': comic.origin, 'url': comic.url}
