#! /usr/bin/env nix-shell
#! nix-shell -i bash -p bash -p bridge-utils -p tunctl

TAP_INTERFACE_VM=tap0
BRIDGE_INTERFACE=br0

sudo brctl delif ${BRIDGE_INTERFACE} ${TAP_INTERFACE_VM}

sudo tunctl -d ${TAP_INTERFACE_VM}

sudo ifconfig ${BRIDGE_INTERFACE} down

sudo brctl delbr ${BRIDGE_INTERFACE}
