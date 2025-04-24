#!/usr/bin/python3.8

# ----------------------------------------------------------------------
#
# 2025-04-24: Move configuration into YAML
#
# 2023-06-05: Add host_list function
#
# 2023-03-13: https://github.com/oVirt/ovirt-engine-sdk/tree/master/sdk
#
# ----------------------------------------------------------------------

import ovirtsdk4 as sdk
import ovirtsdk4.types as types
import json
from yaml import load, Loader as Loader
import humanfriendly

# ----------------------------------------------------------------------
#
# vm_list
#
# ----------------------------------------------------------------------

def vm_list(system_service):

# Find the service that manages the collection of virtual
# machines:

    vms_service = system_service.vms_service()
    vms = vms_service.list()

    for vm in vms:
        vm_service = vms_service.vm_service(vm.id)
        disk_attachments = vm_service.disk_attachments_service().list()
        disk_size = 0
        for disk_attachment in disk_attachments:
            disk = connection.follow_link(disk_attachment.disk)
            disk_size += disk.provisioned_size

        cpu = vm.cpu
        print("%-30s %8s  %8s  [%s × %s × %s] %s" % (vm.name,
                                                     humanfriendly.format_size(disk_size, binary = True),
                                                     humanfriendly.format_size(vm.memory, binary = True),
                                                     cpu.topology.sockets, cpu.topology.cores, cpu.topology.threads,
                                                     vm.status))


# ----------------------------------------------------------------------
#
# host_list(system_service):
#
# ----------------------------------------------------------------------

def host_list(system_service):

    hosts_service = system_service.hosts_service()
    hosts = hosts_service.list()
    for host in hosts:
        cpu = host.cpu
        topology = cpu.topology
        print("%s %s [%d × %d × %d] %s" % (host.name, cpu.name, topology.sockets, topology.cores, topology.threads, humanfriendly.format_size(host.memory, binary=True)))


# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

configuration = load(open("ovirt.yaml"), Loader=Loader)

connection = sdk.Connection(
    url=configuration['url'],
    username=configuration['username'],
    password=configuration['password'],
    ca_file=configuration['ca_file'],
    )

system_service = connection.system_service()

vm_list(system_service)
print()
host_list(system_service)

connection.close()
