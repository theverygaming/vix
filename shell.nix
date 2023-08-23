with import <nixpkgs> { };
let
in stdenv.mkDerivation {
  name = "vix";
  buildInputs =
    let
      binutils-x86_64-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "binutils-x86_64-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz";
            sha256 = "sha256-rppXieI0WeWWBuZxRyPy0//DHAMXQZHvDQFb3wYAdFA=";
          };
          buildInputs = [ ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=x86_64-elf --disable-nls --disable-werror --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install
          '';
        };
      gcc-x86_64-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "gcc-x86_64-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz";
            sha256 = "sha256-ESdap7s0zYqxAdAbNBAVSZ+NlGY0KiV07Ok/lU2SJzs=";
          };
          buildInputs = [ gmp mpfr libmpc binutils-x86_64-embedded ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=x86_64-elf --disable-nls --disable-multilib --enable-languages=c,c++ --with-gnu-as --with-as="${binutils-x86_64-embedded}/bin/x86_64-elf-as" --with-gnu-ld --with-ld="${binutils-x86_64-embedded}/bin/x86_64-elf-ld" --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all-gcc
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install-gcc
          '';
        };
      binutils-i686-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "binutils-i686-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz";
            sha256 = "sha256-rppXieI0WeWWBuZxRyPy0//DHAMXQZHvDQFb3wYAdFA=";
          };
          buildInputs = [ ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=i686-elf --disable-nls --disable-werror --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install
          '';
        };
      gcc-i686-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "gcc-i686-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz";
            sha256 = "sha256-ESdap7s0zYqxAdAbNBAVSZ+NlGY0KiV07Ok/lU2SJzs=";
          };
          buildInputs = [ gmp mpfr libmpc binutils-i686-embedded ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=i686-elf --disable-nls --disable-multilib --enable-languages=c,c++ --with-gnu-as --with-as="${binutils-i686-embedded}/bin/i686-elf-as" --with-gnu-ld --with-ld="${binutils-i686-embedded}/bin/i686-elf-ld" --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all-gcc
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install-gcc
          '';
        };
      binutils-m68k-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "binutils-m68k-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz";
            sha256 = "sha256-rppXieI0WeWWBuZxRyPy0//DHAMXQZHvDQFb3wYAdFA=";
          };
          buildInputs = [ ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=m68k-elf --disable-nls --disable-werror --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install
          '';
        };
      gcc-m68k-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "gcc-m68k-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz";
            sha256 = "sha256-ESdap7s0zYqxAdAbNBAVSZ+NlGY0KiV07Ok/lU2SJzs=";
          };
          buildInputs = [ gmp mpfr libmpc binutils-m68k-embedded ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=m68k-elf --disable-nls --disable-multilib --enable-languages=c,c++ --with-gnu-as --with-as="${binutils-m68k-embedded}/bin/m68k-elf-as" --with-gnu-ld --with-ld="${binutils-m68k-embedded}/bin/m68k-elf-ld" --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all-gcc
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install-gcc
          '';
        };
      binutils-aarch64-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "binutils-aarch64-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz";
            sha256 = "sha256-rppXieI0WeWWBuZxRyPy0//DHAMXQZHvDQFb3wYAdFA=";
          };
          buildInputs = [ ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=aarch64-none-elf --disable-nls --disable-werror --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install
          '';
        };
      gcc-aarch64-embedded = with pkgs;
        stdenv.mkDerivation rec {
          name = "gcc-aarch64-embedded";

          src = fetchurl {
            url = "https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz";
            sha256 = "sha256-ESdap7s0zYqxAdAbNBAVSZ+NlGY0KiV07Ok/lU2SJzs=";
          };
          buildInputs = [ gmp mpfr libmpc binutils-aarch64-embedded ];

          hardeningDisable = [ "format" ];

          configurePhase = ''
            ./configure --target=aarch64-none-elf --disable-nls --disable-multilib --enable-languages=c,c++ --with-gnu-as --with-as="${binutils-aarch64-embedded}/bin/aarch64-none-elf-as" --with-gnu-ld --with-ld="${binutils-aarch64-embedded}/bin/aarch64-none-elf-ld" --prefix=/
          '';

          buildPhase = ''
            make -j$NIX_BUILD_CORES all-gcc
          '';

          installPhase = ''
            mkdir -p "$out"
            DESTDIR="$out" make install-gcc
          '';
        };
    in
    [
      binutils-x86_64-embedded
      gcc-x86_64-embedded

      binutils-i686-embedded
      gcc-i686-embedded

      binutils-m68k-embedded
      gcc-m68k-embedded

      binutils-aarch64-embedded
      gcc-aarch64-embedded

      gnumake
      xxd
      ncurses
      pkg-config
      bison
      flex
      nasm
      cmake
      python311
      grub2
      xorriso
      qemu
    ];
}
