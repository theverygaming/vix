{ pkgs }:
let
  common = import ./common.nix { inherit pkgs; };
  funnyarch-tools = pkgs.stdenv.mkDerivation rec {
    name = "funnyarch-tools";

    src = pkgs.fetchFromGitHub {
      owner = "theverygaming";
      repo = "funnyarch";
      rev = "223e1c4bb1acadca287fce108b8f357f769328b5";
      sha256 = "sha256-XR/aXXQvIOUVOVSIJutCfs6QwM00IwHHPo5UObAglFY=";
    };

    buildInputs = [
      pkgs.python313
    ];

    installPhase = ''
      mkdir -p "$out/bin"
      cp -r assembler/python/lib*.py "$out/bin"
      cp -r assembler/python/preprocessor.py "$out/bin/preprocessor.py"
      cp -r assembler/python/assembler.py "$out/bin/funnyarch-none-none-as"
      cp -r assembler/python/linker.py "$out/bin/funnyarch-none-none-ld"
    '';
  };
in
pkgs.stdenv.mkDerivation {
  name = "vix funnyarch";
  buildInputs =
    with pkgs;
    [
      (pkgs.stdenv.mkDerivation rec {
        name = "gcc-funnyarch-none-none-embedded";

        src = pkgs.fetchFromGitHub {
          owner = "theverygaming";
          repo = "gcc";
          rev = "25ef612b151ecd6fa6c0af74d76b64a13a06c55f";
          sha256 = "sha256-JQ00wmzPkjuY+9xEHhxrYjFsHs6s6BFloi6WCekHCB8=";
        };

        # somehow breaks our config.sub modifications???
        updateAutotoolsGnuConfigScriptsPhase = "true";

        buildInputs = [
          pkgs.gmp
          pkgs.mpfr
          pkgs.libmpc
          pkgs.flex
          pkgs.bison
        ];

        hardeningDisable = [ "format" ];

        # gcc does not support in-tree build
        # if we build in-tree with libgcc it will fail with something like: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=32212
        # without libgcc the in-tree build works fine
        preConfigure = ''
          mkdir build
          cd build
        '';

        configureScript = "../configure";

        configureFlags = [
          "--target=funnyarch-none-none"
          "--disable-nls"
          "--disable-multilib"
          "--enable-languages=c,c++"
          "--without-gnu-as"
          "--with-as=${funnyarch-tools}/bin/funnyarch-none-none-as"
          "--without-gnu-ld"
          "--with-ld=${funnyarch-tools}/bin/funnyarch-none-none-ld"
          "--prefix=/"
          "AR_FOR_TARGET=ar"
          "RANLIB_FOR_TARGET=ranlib"
        ];

        buildPhase = ''
          make -j$NIX_BUILD_CORES all-gcc
          make -j$NIX_BUILD_CORES all-target-libgcc CFLAGS_FOR_TARGET='-fomit-frame-pointer -fno-merge-constants -munaligned-access'
        '';

        installPhase = ''
          mkdir -p "$out"
          DESTDIR="$out" make install-gcc
          DESTDIR="$out" make install-target-libgcc
        '';
      })
      funnyarch-tools
    ]
    ++ common.commonPkgs
    ++ common.fatTools;

  shellHook = common.shellHook + ''
    alias vix-build='make MAKE_ARCH=funnyarch CROSS_COMPILE=funnyarch-none-none- -j$NIX_BUILD_CORES'
  '';
}
