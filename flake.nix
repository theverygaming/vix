{
  description = "vix";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    sillystrap = {
      url = "path:./sillystrap";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      rust-overlay,
      sillystrap,
    }:
    { }
    // flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ rust-overlay.overlays.default ];
        };
      in
      {
        devShells = {
          IA-32 = import ./nix-shells/IA-32.nix { inherit pkgs; };
          x86-64 = import ./nix-shells/x86-64.nix { inherit pkgs; };
          aarch64 = import ./nix-shells/aarch64.nix { inherit pkgs; };
          m68k = import ./nix-shells/m68k.nix { inherit pkgs; };
        };
        test = sillystrap.lib.sillystrapSystem {
          modules = [
            (
              { pkgs, lib, ... }:
              {
                options = {
                  testPkgs = lib.mkOption {
                    type = lib.types.package;
                  };
                };
                config = {
                  nixpkgs.buildPlatform = system;
                  nixpkgs.system = "i686-linux";
                  testPkgs = pkgs.bash;
                };
              }
            )
            (
              { pkgs, lib, ... }:
              {
                boot.loader.limine.extraConfig = ''
                  default_entry: 1
                  timeout: 3
                  verbose: yes

                  /vix
                      protocol: multiboot2
                      kernel_path: boot():/boot/kernel.elf
                      module_path: boot():/boot/initramfs.bin
                '';
                system.partitions.boot.files = {
                  "/boot/kernel.elf" = ./kernel/kernel.o;
                  "/boot/initramfs.bin" = ./roramfs.fs;
                };
              }
            )
          ];
        };
      }
    );
}
