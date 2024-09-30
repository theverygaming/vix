{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  cross = import ./cross.nix { pkgs = pkgs; target = "aarch64-none-elf"; };
in
pkgs.stdenv.mkDerivation {
  name = "vix aarch64";
  buildInputs = with pkgs; [
    cross.binutils
    cross.gcc
  ] ++ common.commonPkgs ++ common.limine ++ common.fatTools;

  shellHook = common.shellHook + ''
    export VIX_QEMU_UEFI_BIOS_PATH="${pkgs.qemu}/share/qemu/edk2-aarch64-code.fd"
    alias vix-build='make MAKE_ARCH=aarch64 CROSS_COMPILE=aarch64-none-elf- -j$NIX_BUILD_CORES && make bootimg-aarch64'
    alias vix-run='qemu-system-aarch64 -m 1024 -bios "$VIX_QEMU_UEFI_BIOS_PATH" -cpu cortex-a53 -smp 4 -machine virt -device ramfb -device qemu-xhci -device usb-kbd -hda vix_uefi.img'
  '';
}
