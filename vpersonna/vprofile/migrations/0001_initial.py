# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import django.core.validators


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Client',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('client_id', models.IntegerField(verbose_name=b'Client ID')),
                ('client_name', models.CharField(max_length=200, verbose_name=b'Client Name')),
                ('phone_number', models.CharField(blank=True, max_length=20, verbose_name=b'Phone Number', validators=[django.core.validators.RegexValidator(regex=b'^\\+?1?\\d{9,15}$', message=b"Phone number must be entered in the format: '+999999999'. Up to 15 digits allowed.")])),
                ('email', models.EmailField(max_length=100, verbose_name=b'Email')),
                ('card_id', models.CharField(blank=True, max_length=20, verbose_name=b'Card ID', validators=[django.core.validators.RegexValidator(regex=b'^\\d{13}$', message=b'Card ID (CNP) is 13 digits left.')])),
                ('address', models.CharField(max_length=300, verbose_name=b'Address')),
                ('contract_id', models.CharField(max_length=100, verbose_name=b'Contract')),
                ('contract_type', models.CharField(max_length=100, verbose_name=b'Contract Type')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Device',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('device_id', models.IntegerField(verbose_name=b'Device ID')),
                ('decive_type', models.CharField(max_length=40, verbose_name=b'Device Type')),
                ('decive_name', models.CharField(max_length=100, verbose_name=b'Device Name')),
                ('client', models.ForeignKey(to='vprofile.Client')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Remote',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('ip_address', models.GenericIPAddressField(verbose_name=b'IP Address')),
                ('hostname', models.CharField(max_length=100, verbose_name=b'Hostname', blank=True)),
                ('protocol', models.CharField(max_length=100, verbose_name=b'L4 Protocol')),
                ('app_protocol', models.CharField(max_length=100, verbose_name=b'L7 Protocol')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='TrafficSession',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('start_time', models.DateTimeField(verbose_name=b'Start Time')),
                ('end_time', models.DateTimeField(verbose_name=b'End Time')),
                ('num_packets', models.PositiveIntegerField(verbose_name=b'Number of Packets')),
                ('dropped_packets', models.PositiveIntegerField(verbose_name=b'Dropped Packets')),
                ('traffic_size', models.PositiveIntegerField(verbose_name=b'Traffic Size (in KB)')),
                ('meta', models.TextField(verbose_name=b'Meta')),
                ('device', models.ForeignKey(to='vprofile.Device')),
                ('remote', models.ForeignKey(to='vprofile.Remote')),
            ],
            options={
            },
            bases=(models.Model,),
        ),
    ]
