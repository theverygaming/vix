#! /usr/bin/env nix-shell
#! nix-shell -i bash -p bash -p bridge-utils -p tunctl

TAP_INTERFACE_VM=tap0
BRIDGE_INTERFACE=br0
STATIC_IP=192.168.69.123/24

sudo brctl addbr ${BRIDGE_INTERFACE}

sudo tunctl -t ${TAP_INTERFACE_VM} -u $(whoami)

sudo ip addr flush dev ${TAP_INTERFACE_VM}

sudo brctl addif ${BRIDGE_INTERFACE} ${TAP_INTERFACE_VM}

sudo ifconfig ${TAP_INTERFACE_VM} up
sudo ifconfig ${BRIDGE_INTERFACE} up

sudo ip addr add ${STATIC_IP} dev ${BRIDGE_INTERFACE}

echo use "-netdev tap,id=mynet0,ifname=${TAP_INTERFACE_VM},script=no,downscript=no -device rtl8139,netdev=mynet0"
echo "-object filter-dump,id=net0,netdev=net0,file=out.pcap or something might also be useful sometime:tm:"
