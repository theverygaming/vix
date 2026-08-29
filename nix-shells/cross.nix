{
  pkgs,
  target,
  libgccExtraFlags ? "",
}:
let
  binutils = pkgs.stdenv.mkDerivation rec {
    name = "binutils-${target}-embedded";

    src = pkgs.fetchurl {
      url = "https://ftp.gnu.org/gnu/binutils/binutils-2.47.tar.xz";
      sha256 = "sha256-FUqyO2AHDo8nATwil38RKUJdZ9HorNbhMBDmF4EeTP8=";
    };
    buildInputs = [ ];

    hardeningDisable = [ "format" ];

    configurePhase = ''
      ./configure --target=${target} --disable-nls --disable-werror --prefix=/
    '';

    buildPhase = ''
      make -j$NIX_BUILD_CORES all
    '';

    installPhase = ''
      mkdir -p "$out"
      DESTDIR="$out" make install
    '';
  };
in
{
  binutils = binutils;
  gcc = pkgs.stdenv.mkDerivation rec {
    name = "gcc-${target}-embedded";

    src = pkgs.fetchurl {
      url = "https://ftp.gnu.org/gnu/gcc/gcc-16.2.0/gcc-16.2.0.tar.xz";
      sha256 = "sha256-5nOOKVl/czJwcxqpBgDzf/3ARQed/CfsfoGSzIEIXD4=";
    };
    buildInputs = [
      pkgs.gmp
      pkgs.mpfr
      pkgs.libmpc
      binutils
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
      "--target=${target}"
      "--disable-nls"
      "--disable-multilib"
      "--enable-languages=c,c++"
      "--with-gnu-as"
      "--with-as=${binutils}/bin/${target}-as"
      "--with-gnu-ld"
      "--with-ld=${binutils}/bin/${target}-ld"
      "--prefix=/"
    ];

    buildPhase = ''
      make -j$NIX_BUILD_CORES all-gcc
      make -j$NIX_BUILD_CORES all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 ${libgccExtraFlags}'
    '';

    installPhase = ''
      mkdir -p "$out"
      DESTDIR="$out" make install-gcc
      DESTDIR="$out" make install-target-libgcc
    '';
  };
}
