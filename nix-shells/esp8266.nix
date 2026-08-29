{ pkgs }:
let
  common = import ./common.nix { inherit pkgs; };
  common-xtensa = import ./common-xtensa.nix {
    inherit pkgs;
    binutilsPostPatch = common-xtensa.applyOverlay (
      (common-xtensa.espressif-overlays "esp8266") + "/xtensa_esp8266" + "/binutils"
    );
    gccPostPatch = common-xtensa.applyOverlay (
      (common-xtensa.espressif-overlays "esp8266") + "/xtensa_esp8266" + "/gcc"
    );
  };
in
pkgs.stdenv.mkDerivation {
  name = "vix esp8266";
  buildInputs =
    with pkgs;
    [
      esptool
    ]
    ++ common-xtensa.toolchain
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''

    alias vix-build='make MAKE_ARCH=xtensa CROSS_COMPILE=xtensa-elf- -j$NIX_BUILD_CORES'
    alias vix-flash='esptool --chip esp8266 elf2image --flash-mode="dio" --flash-freq "40m" --flash-size "4MB" -o kernel- kernel/kernel.o && esptool --chip esp8266 --baud 115200 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 40m --flash-size detect 0x00000 kernel-0x00000.bin'
  '';
}
