{ pkgs, funnyarch }:
let
  common = import ./common.nix { inherit pkgs; };
in
pkgs.stdenv.mkDerivation {
  name = "vix funnyarch";
  buildInputs =
    with pkgs;
    [
      funnyarch.toolchain
      funnyarch.gcc
    ]
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=funnyarch CROSS_COMPILE=funnyarch-none-none- -j$NIX_BUILD_CORES'
  '';
}
