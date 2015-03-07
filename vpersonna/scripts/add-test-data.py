#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import sys
import os
import codecs
import datetime

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "vpersonna.settings")
import django
django.setup()

from vprofile.models import Client, Device, Remote, TrafficSession

# Setup Django environment.
UTF8Writer = codecs.getwriter('utf8')
sys.stdout = UTF8Writer(sys.stdout)

def add_test_data():
    client = Client()
    client.client_id = 12345
    client.client_name = 'Victor Papusa-Atomica'
    client.phone_number = '8989989'
    email = 'victor@papusa.atomica.com'
    card_id = '1899218919291'
    address = 'Calea Prazului nr. 1, Zaibăreni, Dolj'
    contract_id = '1/32.13.2015'
    contract_type = 'de muritor'
    client.save()

    device = Device()
    device.client = client
    device.device_id = 54321
    device.device_type = 'tv'
    device.device_name = 'Alonda'
    device.save()

    remote = Remote()
    remote.ip_address = '69.69.69.69'
    remote.hostname = 'pr0n.4.victor.com'
    remote.protocol = 'TCP'
    remote.app_protocol = 'HTTP'
    remote.save()

    session = TrafficSession()
    session.device = device
    session.remote = remote
    session.start_time = datetime.datetime(2015, 03, 05, 0, 45)
    session.end_time = datetime.datetime(2015, 03, 05, 0, 49)
    session.num_packets = 7000
    session.dropped_packets = 500
    session.traffic_size = 75000
    session.meta = 'Debbie does Victor'
    session.save()

def read_data():
    clients = Client.objects.all()
    devices = Device.objects.all()
    remotes = Remote.objects.all()
    sessions = TrafficSession.objects.all()

    for c in clients:
        print c.__dict__
    for d in devices:
        print d.__dict__
    for r in remotes:
        print r.__dict__
    for s in sessions:
        print s.__dict__

def remove_all_data():
    clients = Client.objects.all()
    devices = Device.objects.all()
    remotes = Remote.objects.all()
    sessions = TrafficSession.objects.all()

    for c in clients:
        c.delete()
    for d in devices:
        d.delete()
    for r in remotes:
        r.delete()
    for s in sessions:
        s.delete()

def main():
    add_test_data()
    read_data()
    remove_all_data()

if __name__ == "__main__":
    sys.exit(main())
