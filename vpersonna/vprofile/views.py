from django.shortcuts import render
from django.http import HttpResponse
from django.template import RequestContext, loader

from vprofile.models import Client

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
    return HttpResponse("Profile page for client %s (id: %d)." %(client.client_name, client.client_id))
