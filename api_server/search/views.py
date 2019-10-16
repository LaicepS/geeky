from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic


def index(request):
    request_params = request.GET

    comic_list = set()
    for kw in request_params.getlist("keywords"):
        query = Comic.objects.filter(keywords__contains=kw)
        try:
            [comic_list.add(comic) for comic in query]
        except Comic.DoesNotExist:
            pass

    return HttpResponse(json.dumps([serialize_comic(comic) for comic in comic_list]))


def serialize_comic(comic):
    if comic is None:
        return {}

    return {"origin": comic.origin, "url": comic.url}
