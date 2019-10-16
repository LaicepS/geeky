from django.db import models


class Comic(models.Model):
    url = models.URLField(max_length=400)
    keywords = models.CharField(max_length=400)
    origin = models.CharField(max_length=200, default=None)

    def __str__(self):
        return "{}: {}".format(self.origin, self.keywords)
