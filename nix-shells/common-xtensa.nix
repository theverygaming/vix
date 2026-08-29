{ pkgs }:
let
  cross = import ./cross.nix {
    pkgs = pkgs;
    target = "xtensa-elf";
  };
in
{
  toolchain = [
    cross.binutils
    cross.gcc
  ];
  espressif-xtensaconfig =
    chip:
    let
      conf = pkgs.stdenv.mkDerivation {
        name = "xtensa-overlays-${chip}";

        src = pkgs.fetchFromGitHub {
          owner = "espressif";
          repo = "xtensa-overlays";
          rev = "dd1cf19f6eb327a9db51043439974a6de13f5c7f";
          sha256 = "sha256-guFWS6QAjJ1Z2u2YOIha97EaBGLThWRz6kjrPSf0y9M=";
        };

        # fix "error: zero or negative size array" during esp8266 build
        postPatch = ''
          substituteInPlace xtensa_esp8266/binutils/bfd/xtensa-modules.c --replace-fail \
            'funcUnits[]' \
            'funcUnits[1]'
          substituteInPlace xtensa_esp8266/binutils/bfd/xtensa-modules.c --replace-fail \
            'interfaces[]' \
            'interfaces[1]'
        '';

        installPhase = ''
          mkdir $out
          cp -r xtensa_${chip} $out
        '';
      };
    in
    pkgs.stdenv.mkDerivation {
      name = "xtensa-dynconfig-${chip}";

      src = pkgs.fetchFromGitHub {
        owner = "jcmvbkbc";
        repo = "xtensa-dynconfig";
        rev = "c545876fb73c0ada60cb6413c61554f5b93d4e8a";
        sha256 = "sha256-FB6x/xWl3Sw5nUnzgGsL5B/mI/mr0DUS1uNNcGR8AOo=";
      };

      buildPhase = ''
        make -j$NIX_BUILD_CORES CONF_DIR=${conf}
      '';

      installPhase = ''
        cp xtensa_${chip}.so $out
      '';
    };
}
