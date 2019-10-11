from django.db import models

class Comic(models.Model):
    url = models.URLField(max_length=400)
    keywords = models.CharField(max_length=400)
