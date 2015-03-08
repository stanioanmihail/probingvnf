#! /usr/bin/python

# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import sys
import os
import codecs
import datetime
import json
import random
import string
import copy

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "vpersonna.settings")
import django
django.setup()

from vprofile.models import Client, Device, Remote, TrafficSession

# Setup Django environment.
UTF8Writer = codecs.getwriter('utf8')
sys.stdout = UTF8Writer(sys.stdout)


day_priority = None
week_priority = None
month_priority = None
year_priority = None
noise_priority = None

def init_priorities():
    global day_priority, week_prioriy, month_prioriy, year_priority, noise_priority
    day_priority = 40
    week_priority = 30
    month_priority = 20
    year_priority = 10
    noise_priority = 0

def get_random_byte():
    return str(random.randint(1, 10)) + str(random.randint(0, 10))

def generate_bogus_sites(number):
    sites = []
    ips = []
    for i in range(number):
        okay = False
        while not okay:
            site = 'www.' + ''.join(random.choice(string.ascii_lowercase) for i in range(random.randint(3, 20))) + '.com'
            if site not in sites:
                sites.append(site)
                okay = True
        okay = False
        while not okay:
            ip = get_random_byte() + '.' + get_random_byte() + '.' + get_random_byte() + '.' + get_random_byte()
            if ip not in ips:
                ips.append(ip)
                okay = True
    d = [{'site': sites[i], 'ip': ips[i]} for i in range(len(ips))]
    return d

def reset_profile_settings(settings):
    settings['days'] = {}
    settings['days'] = dict([(i, []) for i in range(1, 366)])
    
def load_noise_settings(data, settings, sites):
    global noise_priority
    noise = data['noise']
    noise_variation = data['noise_variation']
    noise_session_variation = data['noise_session_variation']
    noise_packets = data['noise_packets']
    noise_packets_variation = data['noise_packets_variation']
    noise_dropped_packets = data['noise_dropped_packets']
    noise_dropped_packets_variation = data['noise_dropped_packets_variation']
    for day in settings['days']:
        actions = [action for action in settings['days'][day] if action['pri'] < noise_priority]
        for i in range(24):
            actions.append({'pri':noise_priority, 'start':i, 'end':i+1, 'sites':sites, 'bogus':True, 'number': noise, 'number_variation': noise_variation, 'session_variation':noise_session_variation, 'packets':noise_packets, 'packets_variation': noise_packets_variation, 'dropped_packets':noise_dropped_packets, 'dropped_packets_variation':noise_dropped_packets_variation})
        settings['days'][day] = actions

def load_daily_settings(data, profile, settings, sites):
    global day_priority
    session_variation = data['session_variation']
    for day in settings['days']:
        actions = [action for action in settings['days'][day] if action['pri'] < day_priority]
        for i in profile['daily']:
            start, end = i['time'].split('-')
            if len(i['unique_remotes']) == 0:
                bogus = True
                l = sites
            else:
                bogus = False
                l = i['unique_remotes']
            actions.append({'pri':day_priority, 'start':start, 'end':end, 'sites':l, 'bogus':bogus, 'number':i['remotes'], 'number_variation':i['remotes_variation'], 'session_variation':session_variation, 'packets':i['packets'], 'packets_variation': i['packets_variation'], 'dropped_packets':i['dropped_packets'], 'dropped_packets_variation':i['dropped_packets_variation']})
        settings['days'][day] = actions

def load_weekly_settings(data, profile, settings, sites):
    global week_priority
    session_variation = data['session_variation']
    for monday in range(1, 366, 7):
        for i in profile['weekly']:
            start_day, end_day = [int(s) for s in i['days'].split('-')]
            for j in range(start_day, end_day):
                day = monday + j
                if(day > 365):
                    break
                actions = [action for action in settings['days'][day] if action['pri'] < week_priority]
                start, end = [int(x) for x in i['time'].split('-')]
                if len(i['unique_remotes']) == 0:
                    bogus = True
                    l = sites
                else:
                    bogus = False
                    l = i['unique_remotes']
                actions.append({'pri':week_priority, 'start':start, 'end':end, 'sites':l, 'bogus':bogus, 'number':i['remotes'], 'number_variation':i['remotes_variation'], 'session_variation':session_variation, 'packets':i['packets'], 'packets_variation': i['packets_variation'], 'dropped_packets':i['dropped_packets'], 'dropped_packets_variation':i['dropped_packets_variation']})
                settings['days'][day] = actions

def load_yearly_settings(data, profile, settings, sites):
    global year_priority
    session_variation = data['session_variation']
    for i in profile['yearly']:
        start_day, end_day = [int(s) for s in i['days'].split('-')]
        for day in range(start_day, end_day):
            actions = [action for action in settings['days'][day] if action['pri'] < year_priority]
            start, end = i['time'].split('-')
            if len(i['unique_remotes']) == 0:
                bogus = True
                l = sites
            else:
                bogus = False
                l = i['unique_remotes']
            actions.append({'pri':year_priority, 'start':start, 'end':end, 'sites':l, 'bogus':bogus, 'number':i['remotes'], 'number_variation':i['remotes_variation'], 'session_variation':session_variation, 'packets':i['packets'], 'packets_variation': i['packets_variation'], 'dropped_packets':i['dropped_packets'], 'dropped_packets_variation':i['dropped_packets_variation']})
            settings['days'][day] = actions

def load_settings(data, profile, settings, bogus_sites):
    load_noise_settings(data, settings, bogus_sites)
    load_daily_settings(data, profile, settings, bogus_sites)
    load_weekly_settings(data, profile, settings, bogus_sites)
    load_yearly_settings(data, profile, settings, bogus_sites)

def add_client_ids(clients):
    for client in clients:
        client['client_id'] = random.randint(1,10000000)

def get_random_name():
    return random.choice(string.ascii_uppercase) + ''.join(random.choice(string.ascii_lowercase) for i in range(random.randint(2, 20)))

def get_lowercase_string():
    return ''.join(random.choice(string.ascii_lowercase) for i in range(random.randint(2, 20)))

def get_number():
    return ''.join(str(random.choice(range(10))) for i in range(random.randint(10, 14)))

def add_client_names(clients):
    for client in clients:
        client['client_name'] = get_random_name() + ' ' + get_random_name() + '-' + get_random_name()

def add_client_phone_number(clients):
    for client in clients:
        client['client_phone_number'] = get_number()

def add_email(clients):
    for client in clients:
        client['email'] = get_lowercase_string() + '@' + get_lowercase_string() + '.com'

def add_card_id(clients):
    for client in clients:
        client['card_id'] = get_number()

def add_address(clients):
    for client in clients:
        client['address'] = get_lowercase_string() + ' ' + get_lowercase_string() + ' ' + get_lowercase_string()

def add_contract_id(clients):
    for client in clients:
        client['contract_id'] = get_number()

def add_contract_type(clients):
    for client in clients:
        client['contract_type'] = get_lowercase_string()

def add_device_id(clients):
    for client in clients:
        client['device_id'] = get_number()

def add_device_type(clients):
    for client in clients:
        client['device_type'] = get_lowercase_string()

def add_device_name(clients):
    for client in clients:
        client['device_name'] = get_lowercase_string()

def add_data(clients, settings, app_protocols):
    for day in settings['days']:
        for action in settings['days'][day]:
            c = random.choice(clients)
            client = Client()
            client.client_id = c['client_id']
            client.client_name = c['client_name']
            client.phone_number = c['client_phone_number']
            email = c['email']
            card_id = c['card_id']
            address = c['address']
            contract_id = c['contract_id']
            contract_type = c['contract_type']
            client.save()

            device = Device()
            device.client = client
            device.device_id = c['device_id']
            device.device_type = c['device_type']
            device.device_name = c['device_name']
            device.save()

            page = random.choice(action['sites'])
            ip = page['ip']
            site = page['site']
            protocol = 'TCP'
            app_protocol = random.choice(app_protocols)

            remote = Remote()
            remote.ip_address = ip
            remote.hostname = site
            remote.protocol = protocol
            remote.app_protocol = app_protocol
            remote.save()

            print action['start'], day
            start_time = datetime.datetime(2015, 1, 1, int(action['start']), 0) + datetime.timedelta(day)
            end_time = datetime.datetime(2015, 1, 1, int(action['end']) - 1, 59) + datetime.timedelta(day)
            num_packets = action['packets']
            dropped_packets = action['dropped_packets']
            traffic_size = num_packets * 2
            meta = ''

            session = TrafficSession()
            session.device = device
            session.remote = remote
            session.start_time = start_time
            session.end_time = end_time
            session.num_packets = num_packets
            session.dropped_packets = dropped_packets
            session.traffic_size = traffic_size
            session.meta = meta
            session.save()
            



def add_test_data():
    client = Client()
    client.client_id = 12345
    client.client_name = 'Victor Papusa-Atomica'
    client.phone_number = '8989989'
    email = 'victor@papusa.atomica.com'
    card_id = '1899218919291'
    address = 'Calea Prazului nr. 1, Zaibareni, Dolj'
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
        
def main():
    settings_file = open('scripts/settings.json')
    data = json.load(settings_file)
    settings_file.close()
    profiles = data['profiles']
    
    init_priorities()

    bogus_sites = generate_bogus_sites(data['bogus_sites_number'])
    settings = {'days': None}

    total = 0
    for profile in profiles:
        total = total + profile['clients']

    clients = [{}] * total
    add_client_ids(clients)
    add_client_names(clients)
    add_client_phone_number(clients)
    add_email(clients)
    add_card_id(clients)
    add_address(clients)
    add_contract_id(clients)
    add_contract_type(clients)
    add_device_id(clients)
    add_device_type(clients)
    add_device_name(clients)

    for profile in profiles:
        reset_profile_settings(settings)
        load_settings(data, profile, settings, bogus_sites)
        add_data(clients, settings, ['http', 'http', 'http', 'http', 'http', 'video', 'video', 'video', 'voip', 'audio'])


if __name__ == '__main__':
    main()
