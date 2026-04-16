#!/usr/bin/env python3.12


# ----------------------------------------------------------------------
#
# 2025-09-08: Added remaining fields to match vmw-inventory.pl for
#   virtual machines.  Does not do datastores or portgroups but
#   they are rarely reported on now.

# 2025-09-03: Modify to be more compatable with inv-format
#
# 2024-11-19: Add JSON output
#
# ----------------------------------------------------------------------

import sys
from pyVmomi import vim
from pyVim.connect import SmartConnect
import yaml
import argparse
from os import access, R_OK
from os.path import isfile
import csv
import json
from pathlib import Path
from datetime import date
from datetime import datetime
import itertools

# ----------------------------------------------------------------------
#
# flatten
#
# ----------------------------------------------------------------------

def flatten(obj):
    if 'folder' in obj:
        flat = []
        for i in obj['folder']:
            f = flatten(i)
            flat.append(f) if isinstance(f, (dict)) else flat.extend(f)
        return flat
    else:
        return obj



# ----------------------------------------------------------------------
#
# pyvim_hostinfo
#
# ----------------------------------------------------------------------

def pyvim_hostinfo(obj, path):


    print(obj, path, type(obj))
    
    # This section should be able to be merged
    
    if hasattr(obj, 'childEntity'):
        res_list = []
        for child in obj.childEntity:
            newpath = path.copy()
            newpath.append(obj.name)
            res_list.append(pyvim_hostinfo(child, newpath))
        res = {
            'ref': obj._GetMoId(),
            'name': obj.name,
            'path': "/".join(path),
            'type': 'folder',
            'folder': res_list,
            }
        return res

    if isinstance(obj, vim.ClusterComputeResource) or isinstance(obj, vim.ComputeResource):
        res_list = []
        for child in obj.host:
            newpath = path.copy()
            newpath.append(obj.name)
            res_list.append(pyvim_hostinfo(child, newpath))
        res = {
            'ref': obj._GetMoId(),
            'name': obj.name,
            'path': "/".join(path),
            'type': 'cluster',
            'folder': res_list,
            }
        return res

    # Details all copied vmw-inventory

    if isinstance(obj, vim.HostSystem):
        host = {
            'ref': obj._GetMoId(),
            'name': obj.name,
            'path': "/".join(path),
            'type': 'host',
        }

        if hasattr(obj, 'hardware'):
            hardware = obj.hardware
            host['memory'] = hardware.memorySize
            host['cores'] = hardware.cpuInfo.numCpuCores
            host['packages'] = hardware.cpuInfo.numCpuPackages
            host['threads'] = hardware.cpuInfo.numCpuThreads

            host['cpu'] = []
            for cpu in hardware.cpuPkg:
                host['cpu'].append({
                    'description': cpu.description,
                    'clockspeed': cpu.hz,
                    'index': cpu.index,
                    'vendor': cpu.vendor
                    })
            host['model'] = hardware.systemInfo.model
            host['uuid'] = hardware.systemInfo.uuid
            host['vendor'] = hardware.systemInfo.vendor


        if hasattr(obj, 'summary'):
            summary = obj.summary

            if hasattr(summary, 'quickStats'):
                quickstats = summary.quickStats
                host['cpu_usage'] = quickstats.overallCpuUsage
                host['mem_usage'] = quickstats.overallMemoryUsage
                host['uptime'] = quickstats.uptime
            if hasattr(summary, 'hardware'):
                hardware = summary.hardware
                for id in hardware.otherIdentifyingInfo:
                    key = id.identifierType.key
                    if key == "ServiceTag":
                        host['servicetag'] = id.identifierValue
                    if key == "SerialNumberTag":
                        host['serialnumbertag'] = id.identifierValue

    if hasattr(obj, 'runtime'):
        runtime = obj.runtime
        host['maintenance'] = True if runtime.inMaintenanceMode else False

    if hasattr(obj, 'config'):
        config = obj.config
        if hasattr(config, 'product'):
            product = config.product
            host['product'] = {
                'api' : product.apiType,
                'build' : product.build,
                'full_name' : product.fullName,
                'name' : product.name,
                'ostype' : product.osType,
                'line' : product.productLineId,
                'vendor' :  product.vendor,
                'version' : product.version
                }
    
    return host
        
    
# ----------------------------------------------------------------------
#
# pyvim_vminfo
#
# ----------------------------------------------------------------------


def pyvim_vminfo(obj, path):
    """
    Print information for a particular virtual machine or recurse into a folder
    with depth protection
    """

    # if this is a group it will have children. if it does, recurse into them
    # and then return

    print(obj, path, type(obj))
          
#    print(obj, obj.name, " *" if hasattr(obj, 'childEntity') else "")
    
    if hasattr(obj, 'childEntity'):
        res_list = []
        for child in obj.childEntity:
            newpath = path.copy()
            newpath.append(obj.name)
            res_list.append(pyvim_vminfo(child, newpath))
        res = {
            'ref': obj._GetMoId(),
            'name': obj.name,
            'path': "/".join(path),
            'type': 'folder',
            'folder': res_list,
            }
        return res

    # --------------------
    # The structure and fields are same here as in the perl VIM
    # module, which makes converting the old code much simpler.
    # --------------------
    
    summary = obj.summary
    config = obj.config
    runtime = obj.runtime
    hardware = config.hardware
    storage = obj.storage
    layout = obj.layoutEx
    guest = obj.guest
    host = runtime.host

    tags = { x.key:x.value for x in obj.customValue }

    storage_sum = int(summary.storage.committed) + int(summary.storage.uncommitted)

    res = {
        'type': 'virtualmachine',
        'name': summary.config.name,
        'memory': int(summary.config.memorySizeMB),
        'hostname': summary.guest.hostName,
        'ipv4': summary.guest.ipAddress,
        'ncpu': summary.config.numCpu,
        'cores': hardware.numCoresPerSocket,
        'cpu': hardware.numCPU,
        'uuid': summary.config.uuid,
        'iuuid': summary.config.instanceUuid,
        'storage_sum': int(storage_sum/(2**30)),
        'boottime': str(runtime.bootTime),
        'state': str(runtime.powerState),
        'path': "/".join(path),
        'notes': config.annotation,
        'tags': tags,
        'host': host._GetMoId(),
        'devices': {}
    }

    # --------------------
    # quickstats (https://developer.broadcom.com/xapis/vsphere-web-services-api/latest/vim.vm.Summary.QuickStats.html)
    # --------------------

    if summary.quickStats:
        quickstats = summary.quickStats
        res['quickstats'] = {
            'guestmemory': quickstats.guestMemoryUsage,
            'hostmemory': quickstats.hostMemoryUsage,
        };
    
    # --------------------
    # storage
    # --------------------
    
    res['storage'] = [ {'id': s.datastore._GetMoId(), 'ds': s.datastore.name, 'committed': int(s.committed), 'uncommitted':int(s.uncommitted)} for s in storage.perDatastoreUsage ]

    # --------------------
    # SCSI controllers for virtual disks
    # --------------------

    controller_map = {}
    
    for device in hardware.device:
        if isinstance(device, vim.VirtualSCSIController):
#            print("Virtual controller [%s - %s] with bus number %d and scsiCltrUnitNumber %d" % (device.deviceInfo.label, device.deviceInfo.summary, device.busNumber, device.scsiCtlrUnitNumber))
            controller_map[device.key] = device

    disks = []

    # --------------------
    # Virtual disks
    # --------------------
    
    for device in hardware.device:
        if isinstance(device, vim.VirtualDisk):
            disk = {
                'label': device.deviceInfo.label,
                'summary': device.deviceInfo.summary,
                'kb': device.capacityInKB
            }

            if device.controllerKey in controller_map:
                disk['bus'] = int(controller_map[device.controllerKey].busNumber)
                disk['target'] = int(device.unitNumber);
#                print("Disk at %d:%d" % (disk['bus'], disk['target']))

                
                backing = device.backing

                if isinstance(backing, vim.VirtualDeviceFileBackingInfo):
                    disk['file'] = backing.fileName
                    if isinstance(backing, vim.VirtualDiskRawDiskMappingVer1BackingInfo):
                        disk['rdm'] = {
                            'name': backing.deviceName,
                            'uuid': backing.lunUuid
                        }
                disks.append(disk)
    res['devices']['disks'] = disks

    # --------------------
    # Network cards
    # --------------------

    network_cards = []
    
    for device in hardware.device:
        if isinstance(device, vim.VirtualEthernetCard):
            nic = {
                'address' : device.macAddress,
                'connected' : True if device.connectable.connected else False,
                'start' : True if device.connectable.startConnected else False,
            }
            if device.backing:
                backing = device.backing
                if isinstance(backing, vim.VirtualEthernetCardDistributedVirtualPortBackingInfo):
                    port = device.backing.port;
                    nic['portgroup'] = port.portgroupKey
                    if isinstance(backing, vim.VirtualEthernetCardNetworkBackingInfo):
                        network_ref = backing.network
                        nic['network'] = network_ref._GetMoId()
                    if isinstance(device, vim.VirtualVmxnet3):
                        nic['model'] = "vmxnet3"
                    if isinstance(device, vim.VirtualE1000):
                        nic['model'] = "e1000"
                    if isinstance(device, vim.VirtualE1000e):
                        nic['model'] = "e1000e"
            network_cards.append(nic)
    res['devices']['nics'] = network_cards

    # --------------------
    # Guest
    # --------------------

    if guest:
        res['guest'] = {
            'os': guest.guestFullName,
            'state': guest.guestState,
            'family': guest.guestFamily,
            'tools_version': guest.toolsVersion,
            'tools_status': guest.toolsVersionStatus2,
        }

    # --------------------
    # Guest networking
    # --------------------

    if guest and guest.net:
        res['guest']['net'] = []
        for net in guest.net:
            if net.connected:
                t = {
                    'mac': net.macAddress,
                    'id': net.deviceConfigId,
                    'network': net.network,
                    'connected': True
                }
                ip = net.ipConfig
                if ip:
                    t['ip'] = []
                    for addr in ip.ipAddress:
                        t['ip'].append(
                            {
                                'ip': addr.ipAddress,
                                'prefix': addr.prefixLength,
                                'state': addr.state,
                            })
            else:
                t = {
                    'connected': False
                }
            res['guest']['net'].append(t)

    # --------------------
    # Guest disks
    # --------------------

    if guest and guest.disk:
        res['guest']['disks'] = []
        for disk in guest.disk:
            res['guest']['disks'].append({
                'size':  disk.capacity,
                'path' : disk.diskPath,
                'free' :disk.freeSpace,
                })
            
    return res


# ----------------------------------------------------------------------
#
# pyvim_vm_list
#
# ----------------------------------------------------------------------

def pyvim_inv_list(config, res):
    si = SmartConnect(host=config['vc_host'], user=config['vc_user'], pwd=config['vc_pass'], disableSslCertValidation=True)
    session_id = si.content.sessionManager.currentSession.key

    content = si.RetrieveContent()

    # The function pyvim_vminfo is recursive, so we pass in an empty
    # list and have the function append any VM details as it runs.

    # --------------------
    # Convert tag ids to numbers
    # --------------------

    key_map = {}
    cfm = content.customFieldsManager
    for field in cfm.field:
        key_map[field.key] = field.name

    dc_list = []
    
    for datacenter in content.rootFolder.childEntity:
        dc_map = {
            'name': datacenter.name,
            'ref': datacenter._GetMoId()
            }
        dc_list.append(dc_map)
        if hasattr(datacenter, 'vmFolder'):
            vmfolder = datacenter.vmFolder
            dc_map['virtualmachines'] = vm_tree = pyvim_vminfo(vmfolder, [datacenter.name])
        if hasattr(datacenter, 'hostFolder'):
            hostfolder = datacenter.hostFolder
            dc_map['hosts'] = pyvim_hostinfo(hostfolder, [datacenter.name])

        # Flatten vm_tree into list so it can be iterated over
        
        vm_list = flatten(vm_tree)
        for vm in vm_list:
            vm['tags'] = {key_map[x]:y for (x,y) in vm['tags'].items()}


    # --------------------
    # Covert tags from number to text
    # --------------------
    
    customfield = {}
    for field in cfm.field:
        customfield[field.name] = {
            'name': field.name,
            'key': int(field.key)
        }
    res['datacenters'] = dc_list
    res['customfield'] = customfield

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

if not 'vc' in config:
    print("%s: key 'vc' missing from configuration" % (sys.argv[0]), file=sys.stderr)
    exit(1)

if not args.vc in config['vc']:
    print("%s: vc '%s' missing from vc configuration" % (sys.argv[0], args.vc), file=sys.stderr)
    exit(1)

vc_config = config['vc'][args.vc]

# --------------------
# Get list of VMs and hosts
# --------------------

today = datetime.today()

res = {
    'meta': {
        'epoch': today.timestamp(),
        'schema': '2025090900',
        'vcenter': args.vc,
    }
}

vm_tree = pyvim_inv_list(vc_config, res)

if 'json_path' in vc_config:
    path = Path(today.strftime(vc_config['json_path']))
    parent = path.parent
    parent.mkdir(parents=True, exist_ok=True)
    with path.open('w', newline='') as jsonfile:
        json.dump(res, jsonfile)
