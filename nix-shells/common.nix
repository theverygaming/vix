{ pkgs }:
{
  commonPkgs = with pkgs; [
    gnumake
    xxd
    python311

    # kconfig deps
    ncurses
    pkg-config
    bison
    flex

    # for building shitshell etc.
    cmake

    qemu

    # for CI
    ansifilter
  ];

  grub = with pkgs; [
    grub2
    xorriso
  ];

  fatTools = with pkgs; [
    dosfstools
    mtools
  ];

  limine = with pkgs; [
    limine
  ];

  shellHook = ''
    unset SOURCE_DATE_EPOCH
    echo "you can use 'vix-build' and 'vix-run' to compile & run vix"
  '';
}
