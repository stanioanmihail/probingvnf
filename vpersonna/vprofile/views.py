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
    profile_dict = { 'a': 10, 'b': 20, 'c': 30, 'd': 15 }
    context = RequestContext(request, {
        'client': client,
        'profile_dict': profile_dict,
        })
    return HttpResponse(template.render(context))

def profile_on_date(request, client_id):
    client = Client.objects.get(id=client_id)
    template = loader.get_template('profile/date.html')
    tag_list = [ 'a', 'b', 'c', 'd' ]
    traffic_per_timeslot = {
            '08-10': {
                'a': 10, 'b': 20, 'c': 30, 'd': 15
                },
            '10-12': {
                'a': 10, 'b': 20, 'c': 30, 'd': 15
                },
            '12-14': {
                'a': 5, 'b': 2, 'c': 20, 'd': 5
                },
            '14-16': {
                'a': 1, 'b': 0, 'c': 0, 'd': 1
                },
            '16-18': {
                'a': 3, 'b': 4, 'c': 4, 'd': 3
                },
            '18-20': {
                'a': 5, 'b': 2, 'c': 17, 'd': 20
                },
            '20-22': {
                'a': 5, 'b': 10, 'c': 30, 'd': 30
                },
            '22-24': {
                'a': 10, 'b': 5, 'c': 10, 'd': 15
                },
            }
    context = RequestContext(request, {
        'date': request.POST['datepicker'],
        'client': client,
        'traffic_per_timeslot': OrderedDict(sorted(traffic_per_timeslot.items(), key=lambda t: t[0])),
        'tag_list': tag_list,
        })
    return HttpResponse(template.render(context))
