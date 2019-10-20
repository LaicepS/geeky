from functools import total_ordering
import operator

from django.shortcuts import get_object_or_404
from django.http import HttpResponse

import json

from search.models import Comic


def index(request):
    request_params = request.GET

    comic_freq_dict = build_freq_dict(request_params.getlist("keywords"))
    comic_list = sorted(
        comic_freq_dict.items(),
        key=lambda freq_entry: (freq_entry[0].date),
        reverse=True,
    )
    comic_list.sort(key=operator.itemgetter(1), reverse=True)
    comic_list = (comic for comic, counter in comic_list)
    return HttpResponse(json.dumps([serialize_comic(comic) for comic in comic_list]))


def build_freq_dict(keywords):
    comic_freq_dict = dict()
    for kw in keywords:
        query = Comic.objects.filter(keywords__contains=kw)
        for comic in query:
            if comic in comic_freq_dict:
                comic_freq_dict[comic] += 1
            else:
                comic_freq_dict[comic] = 1

    return comic_freq_dict


def serialize_comic(comic):
    if comic is None:
        return {}

    return {"origin": comic.origin, "url": comic.url}
