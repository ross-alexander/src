#!/usr/bin/env python3.12

# ----------------------------------------------------------------------
#

# Hornbill Inventory (hb-inv)

# Collect VM details from vcenter using the REST API via pyvmomi SDK
# then attach CSV file using MIME and send via SMTP.

# 2025-02-11: Add patchingphase period

# 2025-01-21:
#  - Fix directory creation

# 2024-11-19:
#  - smtp split out into its own sub key
#  - csv_directory and file_format merged into csv_path
#  - Individual VC configuration under 'vc'

# 2024-08-29: Initial cut
#
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
#
# https://github.com/vmware/vsphere-automation-sdk-python
#
# ----------------------------------------------------------------------

from os import access, R_OK
from os.path import isfile
import sys
import requests
import urllib3
import csv
from pathlib import Path
import smtplib
import base64
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication
from email.mime.base import MIMEBase
from email import encoders
from datetime import date
from pyVim.connect import SmartConnect
import yaml
import argparse

# ----------------------------------------------------------------------
#
# mime_send_csv
#
# Use a combination of email.mime and smtplib to attach CSV file
# to a message and send it via SMTP
#
# ----------------------------------------------------------------------

def mime_send_csv(config, path, mime_type, mime_subtype):

    if not ('smtp' in config):
        print("Configuration missing smtp key", file=sys.stderr)
        exit(1)

    smtp_config = config['smtp']

    if not ('server' in smtp_config):
        print("Configuration missing smtp.server", file=sys.stderr)
        exit(1)

    if not ('sender' in smtp_config):
        print("Configuration missing smtp.sender", file=sys.stderr)
        exit(1)

    if not ('recipient' in smtp_config):
        print("Configuration missing smtp.recipient", file=sys.stderr)
        exit(1)

    # --------------------
    # Create new smtp object using smtplib module
    # --------------------

    smtp = smtplib.SMTP(smtp_config['server'], 25)

    email_sender = smtp_config['sender']
    email_receiver = smtp_config['recipient']

    # --------------------
    # Use various functions from email.mime
    # --------------------
    
    message = MIMEMultipart()
    message['From'] = email_sender
    message['To'] = email_receiver
    message['Subject'] = 'File attachment %s' % path.name

    # --------------------
    # Add some text, otherwise outlook puts attachment as a message inside the message
    # --------------------
    
    message.attach(MIMEText("File attachment %s" % path.name, "plain"))

    contents = Path(path).read_bytes()
    attachment = MIMEBase(mime_type, mime_subtype)
    attachment.set_payload(contents)
    encoders.encode_base64(attachment)
    attachment.add_header('Content-Disposition', f'attachment; filename="%s"' % path.name)
    message.attach(attachment)

    smtp.sendmail(email_sender, email_receiver, message.as_string())

# ----------------------------------------------------------------------
#
# pyvim_vminfo
#
# ----------------------------------------------------------------------

def pyvim_vminfo(vm, dst):
    """
    Print information for a particular virtual machine or recurse into a folder
    with depth protection
    """

    # if this is a group it will have children. if it does, recurse into them
    # and then return

    if hasattr(vm, 'childEntity'):
        vmlist = vm.childEntity
        for child in vmlist:
            pyvim_vminfo(child, dst)
        return

    # --------------------
    # The structure and fields are same here as in the perl VIM
    # module, which makes converting the old code much simpler.
    # --------------------
    
    summary = vm.summary
    config = vm.config
    runtime = vm.runtime

    tags = { x.key:x.value for x in vm.customValue }
    storage = int(summary.storage.committed) + int(summary.storage.uncommitted)
    
    dst.append(
        {
            'name': summary.config.name,
            'memory': int(summary.config.memorySizeMB),
            'guest': summary.guest.guestFullName,
            'ipv4': summary.guest.ipAddress,
            'ncpu': summary.config.numCpu,
            'uuid': summary.config.uuid,
            'storage': int(storage/(2**30)),
            'state': str(runtime.powerState),
            'notes': config.annotation,
            'tags': tags
        }
    )


# ----------------------------------------------------------------------
#
# pyvim_vm_list
#
# ----------------------------------------------------------------------

def pyvim_vm_list(config):
    si = SmartConnect(host=config['vc_host'], user=config['vc_user'], pwd=config['vc_pass'], disableSslCertValidation=True)
    session_id = si.content.sessionManager.currentSession.key

    content = si.RetrieveContent()

    # The function pyvim_vminfo is recursive, so we pass in an empty
    # list and have the function append any VM details as it runs.
    
    vm_list = []
    for child in content.rootFolder.childEntity:
        if hasattr(child, 'vmFolder'):
            datacenter = child
            vmfolder = datacenter.vmFolder
            for vm in vmfolder.childEntity:
                pyvim_vminfo(vm, vm_list)

    # --------------------
    # Convert tag ids to numbers
    # --------------------

    key_map = {}
    cfm = content.customFieldsManager
    for field in cfm.field:
        key_map[field.key] = field.name

    # Covert tags from number to text
    
    for vm in vm_list:
        vm['tags'] = {key_map[x]:y for (x,y) in vm['tags'].items()}
                
    return vm_list

# ----------------------------------------------------------------------
#
# pyvim_vm_list_csv
#
# ----------------------------------------------------------------------

def vm_list_csv(config, path):
    dst_vm_list = pyvim_vm_list(config)

    
    # Create CSV with header line and use csv module to do all the
    # hard work.

    cluster = config['cluster'] if 'cluster' in config else None
    
    with open(path, 'w', newline='') as csvfile:
        fieldnames = ['Name', 'Cluster', 'State', 'Guest OS', 'Patching Phase', 'IP Address', 'UUID', 'Provisioned Space', 'Host Mem', 'CPUs', 'Notes', 'environment', 'serverfunction', 'client', 'servicename']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames,dialect='excel')
        writer.writeheader()
        for vm in dst_vm_list:
            writer.writerow({'Name': vm['name'],
                             'Cluster': cluster,
                             'State': vm['state'],
                             'Guest OS': vm['guest'],
                             'IP Address': vm['ipv4'],
                             'Patching Phase': vm['tags']['patchingphase'] if 'patchingphase' in vm['tags'] else None,
                             'UUID': vm['uuid'],
                             'Provisioned Space': vm['storage'],
                             'Host Mem': vm['memory'], 'CPUs': vm['ncpu'],
#                             'Notes': vm['notes'],
                             'environment': vm['tags']['environment'] if 'environment' in vm['tags'] else None,
                             'serverfunction': vm['tags']['serverfunction'] if 'serverfunction' in vm['tags'] else None,
                             'client': vm['tags']['client'] if 'client' in vm['tags'] else None,
                             'servicename': vm['tags']['servicename'] if 'servicename' in vm['tags'] else None,
                             })


# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='vCenter to CSV')
parser.add_argument("-c", "--config", action='store', type=str, required=True, help="YAML configuration file")
parser.add_argument("-v", "--vc", action='store', type=str, required=True, help="VC configuration key")
args = parser.parse_args()

# --------------------
# Get configuration details from YAML file to avoid
# embedding credentials in this scripts
# --------------------

if (not isfile(args.config)) or (not access(args.config, R_OK)):
    print("%s: configuration file %s missing or inaccessible" % (sys.argv[0], args.config),file=sys.stderr)
    exit(1)

with open(args.config, 'r') as configfile:
    config = yaml.load(configfile, Loader=yaml.CLoader)


# --------------------
# Get specific VC configuration
# --------------------

if not 'vc' in config:
    print("%s: key 'vc' missing from configuration" % (sys.argv[0]), file=sys.stderr)
    exit(1)

if not args.vc in config['vc']:
    print("%s: vc '%s' missing from vc configuration" % (sys.argv[0], args.vc), file=sys.stderr)
    exit(1)

vc_config = config['vc'][args.vc]

# --------------------
# Check CSV directory exists
# --------------------

if not 'csv_path' in vc_config:
    print("%s: csv_path is a required configuration key" % (sys.argv[0]), file=sys.stderr)
    exit(1)

today = date.today()
path = Path(today.strftime(vc_config['csv_path']))
dir = path.parent
    
if dir.exists() and not dir.is_dir():
    print("%s exists but is not a directory" % (dir), file=sys.stderr)
    exit(1)

if not dir.exists():
    if not (dir.mkdir(parents=True, exist_ok=True) is None):
        print("Failed to make directory %s" % (dir))
        exit(1)

# --------------------
# Create CSV file
# --------------------

vm_list_csv(vc_config, path)

# --------------------
# Email CSV as MIME attachment
# --------------------

if ('smtp' in config):
    mime_send_csv(config, path, "application", "csv")
