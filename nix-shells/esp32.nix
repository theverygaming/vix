{ pkgs }:
let
  common = import ./common.nix { pkgs = pkgs; };
  common-xtensa = import ./common-xtensa.nix { inherit pkgs; };
in
pkgs.stdenv.mkDerivation {
  name = "vix esp32";
  buildInputs =
    with pkgs;
    [
      esptool
    ]
    ++ common-xtensa.toolchain
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''
    export XTENSA_GNU_CONFIG=${common-xtensa.espressif-xtensaconfig "esp32"}
    alias vix-build='make MAKE_ARCH=xtensa CROSS_COMPILE=xtensa-elf- -j$NIX_BUILD_CORES'
    alias vix-flash='esptool --chip esp32 elf2image --flash-mode="dio" --flash-freq "40m" --flash-size "4MB" -o kernel.bin kernel/kernel.o && esptool --chip esp32 --baud 115200 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 40m --flash-size detect 0x1000 kernel.bin'
  '';
}
