from django.db import models
from django.core.validators import RegexValidator


class Client(models.Model):
    client_id = models.IntegerField('Client ID', blank=False)
    client_name = models.CharField('Client Name', max_length=200)
    phone_regex = RegexValidator(regex='^\\+?1?\\d{9,15}$', message="Phone number must be entered in the format: '+999999999'. Up to 15 digits allowed.")
    phone_number = models.CharField('Phone Number', validators=[phone_regex], blank=True, max_length=20)
    email = models.EmailField('Email', max_length=100)
    card_id_regex = RegexValidator(regex='^\\d{13}$', message='Card ID (CNP) is 13 digits left.')
    card_id = models.CharField('Card ID', validators=[card_id_regex], blank=True, max_length=20)
    address = models.CharField('Address', max_length=300)
    contract_id = models.CharField('Contract', max_length=100)
    contract_type = models.CharField('Contract Type', max_length=100)


class Device(models.Model):
    client = models.ForeignKey(Client)
    device_id = models.IntegerField('Device ID', blank=False)
    decive_type = models.CharField('Device Type', blank=False, max_length=40)
    decive_name = models.CharField('Device Name', max_length=100)


class Remote(models.Model):
    ip_address = models.GenericIPAddressField('IP Address', blank=False)
    hostname = models.CharField('Hostname', blank=True, max_length=100)
    protocol = models.CharField('L4 Protocol', max_length=100)
    app_protocol = models.CharField('L7 Protocol', max_length=100)


class TrafficSession(models.Model):
    device = models.ForeignKey(Device)
    remote = models.ForeignKey(Remote)
    start_time = models.DateTimeField('Start Time', blank=False)
    end_time = models.DateTimeField('End Time', blank=False)
    num_packets = models.PositiveIntegerField('Number of Packets')
    dropped_packets = models.PositiveIntegerField('Dropped Packets')
    traffic_size = models.PositiveIntegerField('Traffic Size (in KB)')
    meta = models.TextField('Meta')
