{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  cross = import ./cross.nix { pkgs = pkgs; target = "i686-elf"; };
in
pkgs.stdenv.mkDerivation {
  name = "vix IA-32";
  buildInputs = with pkgs; [
    cross.binutils
    cross.gcc
    nasm
  ] ++ common.commonPkgs ++ common.limine ++ common.grub ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=x86 CROSS_COMPILE=i686-elf- -j$NIX_BUILD_CORES && make bootimg-x86-32 MAKE_ARCH=x86'
    alias vix-run='qemu-system-x86_64 -cdrom vix.iso -m 512'
  '';
}
