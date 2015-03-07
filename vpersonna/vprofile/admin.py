from django.contrib import admin
from vprofile.models import Client, Device, Remote, TrafficSession

# Register your models here.
admin.site.register(Client)
admin.site.register(Device)
admin.site.register(Remote)
admin.site.register(TrafficSession)
