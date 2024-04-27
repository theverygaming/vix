{
  description = "vix";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: { } // flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells = {
        IA-32 = import ./nix-shells/IA-32.nix { inherit pkgs; };
        x86-64 = import ./nix-shells/x86-64.nix { inherit pkgs; };
        aarch64 = import ./nix-shells/aarch64.nix { inherit pkgs; };
        m68k = import ./nix-shells/m68k.nix { inherit pkgs; };
      };
    });
}
