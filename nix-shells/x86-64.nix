{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  cross = import ./cross.nix { pkgs = pkgs; target = "x86_64-elf"; };
in
pkgs.stdenv.mkDerivation {
  name = "vix x86-64";
  buildInputs = [
    cross.binutils
    cross.gcc
  ] ++ common.commonPkgs ++ common.limine ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=x86 CROSS_COMPILE=x86_64-elf- -j$NIX_BUILD_CORES && make bootimg-x86-64 MAKE_ARCH=x86'
    alias vix-run='qemu-system-x86_64 -hda vix_uefi.img -m 512'
  '';
}
