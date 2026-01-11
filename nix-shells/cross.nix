{ pkgs, target }:
let
  binutils = pkgs.stdenv.mkDerivation rec {
    name = "binutils-${target}-embedded";

    src = pkgs.fetchurl {
      url = "https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz";
      sha256 = "sha256-9uTUH9X8d4sGt4kUV7NiDaXs6hAGxqSkGumYEJ+FqAA=";
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
      url = "https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz";
      sha256 = "sha256-4nXnZEKmBnNBon8Exca4PYYTFEAEwEE1KIY9xrXHQ9o=";
    };
    buildInputs = [
      pkgs.gmp
      pkgs.mpfr
      pkgs.libmpc
      binutils
    ];

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
