{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  cross = import ./cross.nix {
    pkgs = pkgs;
    target = "m68k-elf";
  };
in
pkgs.stdenv.mkDerivation {
  name = "vix m68k";
  buildInputs =
    with pkgs;
    [
      cross.binutils
      cross.gcc
    ]
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=m68k CROSS_COMPILE=m68k-elf- -j$NIX_BUILD_CORES && make bootimg-m68k MAKE_ARCH=m68k CROSS_COMPILE=m68k-elf-'
    alias vix-run='minivmacPlus -r "../Mini vMac/vMac.ROM" disk.img'
  '';
}
