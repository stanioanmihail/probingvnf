#!/usr/bin/python

import sys
sys.path.insert(1,'.')

# from the DB, also retrieve hours
# minimal
class UserProfile:
    hostnames = {}
    #average_access # dictionary [hostname, average_access]
    def __init__(self, id, start, end):
        self.id = id
        self.start = start
        self.end = end
    def access(self, host_name, access_rate_min):
        self.hostnames[host_name] = access_rate_min
    def display(self):
        print(' --- Profile --- ')
        for k in self.hostnames.keys():
            print k
            print self.hostnames[k]

suggestions = ['Improve bandwidth for the next period at the cost of 2$/hour', 
                'Improve network transmission quality for netflix with 1$/hour',
                'Visit your online profile to configure performance parameters']

# for each hour, count accesses and take the best 10

# compute category to fit one of the profiles

# map client cateogry

# check relevant events in this period for client profiles

# generate suggestions

def compute_rate(dict2):
    coef = {'access_coef': 0.66, 'drop_coef': 0.33}; #configurable
    return dict2['access_rate'] * coef['access_coef'] + \
            dict2['drop_rate'] * coef['drop_coef'];

def apply_suggestion(dict1): # on each interval
    a = compute_rate(dict1)
    if dict1['access_rate'] >  0.4 and a > 0.3:
            print '\n' + suggestions[0] + '\n'
    else:
            print '\n' + suggestions[2] + '\n'

dictionary = {'access_rate': 0.22, 'drop_rate': 0.5} # computed with neural networ 
 
apply_suggestion(dictionary)
user_profile1 = UserProfile(22312, 9, 22)

user_profile1.access('netflix', 0.3)
user_profile1.access('imdb', 0.2)
user_profile1.display()


