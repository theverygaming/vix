{
  pkgs,
  funnyarch,
  littleboot,
}:
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
      funnyarch.emulator
      funnyarch.emulator-nographics
    ]
    ++ common.commonPkgs
    ++ common.fatTools;

  LITTLEBOOT_FUNNYARCH = littleboot.littleboot-funnyarch;
  LITTLEBOOT_INCLUDEPATH = "${littleboot.littleboot-headers}/include";
  FUNNYARCH_ROM_PATH = funnyarch.rom;

  shellHook = common.shellHook + ''
    alias funnyarch-emu='funnyarch-emulator -r ''${FUNNYARCH_ROM_PATH}'
    alias vix-build='make MAKE_ARCH=funnyarch CROSS_COMPILE=funnyarch-none-none- -j$NIX_BUILD_CORES && make bootimg-funnyarch MAKE_ARCH=funnyarch CROSS_COMPILE=funnyarch-none-none-'
    alias vix-run='funnyarch-emu -d vix.img -m 1M'
  '';
}
