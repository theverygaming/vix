{ pkgs, target }:
let
  binutils = pkgs.stdenv.mkDerivation rec {
    name = "binutils-${target}-embedded";

    src = pkgs.fetchurl {
      url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz";
      sha256 = "sha256-rppXieI0WeWWBuZxRyPy0//DHAMXQZHvDQFb3wYAdFA=";
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
      url = "https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz";
      sha256 = "sha256-ESdap7s0zYqxAdAbNBAVSZ+NlGY0KiV07Ok/lU2SJzs=";
    };
    buildInputs = [ pkgs.gmp pkgs.mpfr pkgs.libmpc binutils ];

    hardeningDisable = [ "format" ];

    configurePhase = ''
      ./configure --target=${target} --disable-nls --disable-multilib --enable-languages=c,c++ --with-gnu-as --with-as="${binutils}/bin/${target}-as" --with-gnu-ld --with-ld="${binutils}/bin/${target}-ld" --prefix=/
    '';

    buildPhase = ''
      make -j$NIX_BUILD_CORES all-gcc
    '';

    installPhase = ''
      mkdir -p "$out"
      DESTDIR="$out" make install-gcc
    '';
  };
}
