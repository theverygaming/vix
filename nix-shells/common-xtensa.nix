{
  pkgs,
  binutilsPostPatch ? null,
  gccPostPatch ? null,
}:
let
  cross = import ./cross.nix {
    inherit pkgs;
    target = "xtensa-elf";
    inherit binutilsPostPatch gccPostPatch;
  };
in
{
  toolchain = [
    cross.binutils
    cross.gcc
  ];
  espressif-overlays =
    chip:
    pkgs.stdenv.mkDerivation {
      name = "xtensa-overlays-${chip}";

      src = pkgs.fetchFromGitHub {
        owner = "espressif";
        repo = "xtensa-overlays";
        rev = "dd1cf19f6eb327a9db51043439974a6de13f5c7f";
        sha256 = "sha256-guFWS6QAjJ1Z2u2YOIha97EaBGLThWRz6kjrPSf0y9M=";
      };

      installPhase = ''
        mkdir $out
        cp -r xtensa_${chip} $out
      '';
    };
  applyOverlay = overlayDir: ''
    find ${overlayDir} -type f | while read -r overlay; do
      rel=''${overlay#${overlayDir}/}
      dest=./$rel
      echo $overlay $rel $dest
      if [ ! -f "$dest" ]; then
        echo "while processing overlay file $rel: got $overlay but dest $dest doesn't exist"
        exit 1
      fi
      cp $overlay $dest
    done
  '';
}
