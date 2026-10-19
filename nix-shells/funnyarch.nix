{ pkgs }:
let
  common = import ./common.nix { inherit pkgs; };
  funnyarch-tools = pkgs.stdenv.mkDerivation rec {
    name = "funnyarch-tools";

    src = pkgs.fetchFromGitHub {
      owner = "theverygaming";
      repo = "funnyarch";
      rev = "551ef159384b8c56f11cb9e6fa82cf8d9b60f3d1";
      sha256 = "sha256-Z9iqfp/7EDxSVhgCHymvZuclVibpzlxNGslzBlO1s3U=";
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
          rev = "bfc122fed1076ca70f9d084287e85b5ec68f265a";
          sha256 = "sha256-rm5XpmeCRsuhks/rQi4k3SLa3BkN/fQCZgU6SFFxmzA=";
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
