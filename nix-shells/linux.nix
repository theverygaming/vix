{ pkgs }:
let
  common = import ./common.nix { inherit pkgs; };
in
pkgs.stdenv.mkDerivation {
  name = "vix linux";
  buildInputs =
    with pkgs;
    [
      binutils
      gcc
    ]
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=linux -j$NIX_BUILD_CORES'
    alias vix-run='./kernel/kernel.o'
    alias vix-debug='gdb kernel/kernel.o -ex "set follow-fork-mode child"'
  '';
}
