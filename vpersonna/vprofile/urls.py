from django.conf.urls import patterns, url

from vprofile import views

urlpatterns = patterns('',
    url(r'^$', views.index, name='index'),
)
