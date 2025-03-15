#!/usr/bin/python3

# ----------------------------------------------------------------------
#
# 2025-02-24: Script to determine if a NAS backup can be deleted
#
# LANG=en_US dsmadmc -se=BSA-SPP-M-SPM02 -id=tsmadmin -password=Royal8XL2013 -dataonly=y -displ=list "select * from filespaces where NODE_NAME like 'EMC%'" > EMC.lst
#
# ----------------------------------------------------------------------

import re
import time
import datetime
import argparse

# ----------------------------------------------------------------------
#
# split_kv_list
#
# ----------------------------------------------------------------------

def split_kv_list(kv_list):
    res = []
    kv_map = {}
    for item in kv_list:
        if (len(item)):
            items = [x.strip() for x in item.split(":", 1)]
            kv_map[items[0]] = items[1]
        else:
            if (len(kv_map.keys())):
                res.append(kv_map)
                kv_map = {}
    return res

# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Determine NDMP backups older than a cutoff')
parser.add_argument("-c", "--cutoff", action='store', type=int, required=False, help="Cutoff in months")
parser.add_argument("-p", "--path", action='store', type=str, required=True, help="Input file from DSMADMC")
args = parser.parse_args()

# Set the default cutoff to 18 months

cutoff = 18
if not (args.cutoff is None):
    cutoff = args.cutoff

# Get path from command line and open the file
    
path = args.path
with open(path, "r") as stream:
    filespaces = split_kv_list(stream.read().splitlines())

# Get today's date

today = datetime.datetime.now()

# Compile a regular expression

year_month_day_re = re.compile('y([0-9]+)_m([0-9]+)_day([0-9]+)')

# Define a size summation

size_sum = 0.0

# Loop over each filespace

for e in filespaces:
    node = e['NODE_NAME']
    name = e['FILESPACE_NAME']
    match = year_month_day_re.search(name)

    # Due to the existance of "illegal" days only using months and years
    # Calculate difference between current yera/month and backup year/month
    
    month_diff = (today.year*12 + today.month) - (int(match.group(1))*12 + int(match.group(2)))

    # Use greater than the cutoff
    
    if (month_diff > cutoff):
        # Estimate size of the backup on the storage
        size = float(e['CAPACITY']) * float(e['PCT_UTIL']) * 0.01
        size_sum += size
        print(f"{node}:\t{name}\t{size:12.2f}")

print(f"All nodes\tAll Filesystems\t\t\t{size_sum:12.2f}")
