from django.shortcuts import render
from django.http import HttpResponse
from django.template import RequestContext, loader

from vprofile.models import Client
from collections import OrderedDict

# Create your views here.
def index(request):
    client_list = Client.objects.all()
    template = loader.get_template('profile/index.html')
    context = RequestContext(request, {
        'client_list': client_list,
        })
    return HttpResponse(template.render(context))

def client_profile(request, client_id):
    client = Client.objects.get(id=client_id)
    template = loader.get_template('profile/profile.html')
    profile_dict = { 'VoIP': 10, 'HTTP (non-video)': 20, 'BitTorrent': 30, 'Video': 15 }
    context = RequestContext(request, {
        'client': client,
        'profile_dict': profile_dict,
        })
    return HttpResponse(template.render(context))

def profile_on_date(request, client_id):
    client = Client.objects.get(id=client_id)
    template = loader.get_template('profile/date.html')
    tag_list = [ 'VoIP', 'HTTP (non-video)', 'BitTorent', 'Video' ]
    traffic_per_timeslot = {
            '08-10': {
                'VoIP': 10, 'HTTP (non-video)': 20, 'BitTorrent': 30, 'Video': 15
                },
            '10-12': {
                'VoIP': 10, 'HTTP (non-video)': 20, 'BitTorrent': 30, 'Video': 15
                },
            '12-14': {
                'VoIP': 5, 'HTTP (non-video)': 2, 'BitTorrent': 20, 'Video': 5
                },
            '14-16': {
                'VoIP': 1, 'HTTP (non-video)': 0, 'BitTorrent': 0, 'Video': 1
                },
            '16-18': {
                'VoIP': 3, 'HTTP (non-video)': 4, 'BitTorrent': 4, 'Video': 3
                },
            '18-20': {
                'VoIP': 5, 'HTTP (non-video)': 2, 'BitTorrent': 17, 'Video': 20
                },
            '20-22': {
                'VoIP': 5, 'HTTP (non-video)': 10, 'BitTorrent': 30, 'Video': 30
                },
            '22-24': {
                'VoIP': 10, 'HTTP (non-video)': 5, 'BitTorrent': 10, 'Video': 15
                },
            }
    context = RequestContext(request, {
        'date': request.POST['datepicker'],
        'client': client,
        'traffic_per_timeslot': OrderedDict(sorted(traffic_per_timeslot.items(), key=lambda t: t[0])),
        'tag_list': tag_list,
        })
    return HttpResponse(template.render(context))
