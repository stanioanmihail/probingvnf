# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('vprofile', '0001_initial'),
    ]

    operations = [
        migrations.RenameField(
            model_name='device',
            old_name='decive_name',
            new_name='device_name',
        ),
        migrations.RenameField(
            model_name='device',
            old_name='decive_type',
            new_name='device_type',
        ),
    ]
