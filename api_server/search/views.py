import operator

from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic


def index(request):
    request_params = request.GET

    comic_list = dict()
    for kw in request_params.getlist("keywords"):
        query = Comic.objects.filter(keywords__contains=kw)
        for comic in query:
            if comic in comic_list:
                comic_list[comic] += 1
            else:
                comic_list[comic] = 1

    comic_list = sorted(comic_list.items(), key=operator.itemgetter(1), reverse=True)
    comic_list = (comic for comic, counter in comic_list)
    return HttpResponse(json.dumps([serialize_comic(comic) for comic in comic_list]))


def serialize_comic(comic):
    if comic is None:
        return {}

    return {"origin": comic.origin, "url": comic.url}
