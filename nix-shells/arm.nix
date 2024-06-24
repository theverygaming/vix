{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  cross = import ./cross.nix { pkgs = pkgs; target = "arm-none-eabi"; };
in
pkgs.stdenv.mkDerivation {
  name = "vix arm";
  buildInputs = with pkgs; [
    cross.binutils
    cross.gcc
  ] ++ common.commonPkgs ++ common.limine ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=arm CROSS_COMPILE=arm-none-eabi- -j$NIX_BUILD_CORES && make bootimg-arm'
    alias vix-run='qemu-system-arm -m 1024 -bios "${pkgs.qemu}/share/qemu/edk2-arm-code.fd" -cpu cortex-a53 -smp 4 -machine virt -device ramfb -device qemu-xhci -device usb-kbd -hda vix_uefi.img'
  '';
}
