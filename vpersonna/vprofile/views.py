from django.shortcuts import render
from django.http import HttpResponse

from vprofile.models import Client

# Create your views here.
def index(request):
    return HttpResponse("This if the vProfile start page.")

def client_profile(request, client_id):
    client = Client.objects.get(id=client_id)
    return HttpResponse("Profile page for client %s (id: %d)." %(client.client_name, client.client_id))
