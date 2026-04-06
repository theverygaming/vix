{
  description = "sillystrap";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  };

  outputs =
    inputs@{ self, ... }:
    {
      lib.sillystrapSystem =
        args:
        inputs.nixpkgs.lib.evalModules (
          {
            modules = args.modules ++ [
              ./modules
              # no need to write our own, these modules will work just fine for us
              (inputs.nixpkgs.outPath + "/nixos/modules/misc/nixpkgs.nix")
              (inputs.nixpkgs.outPath + "/nixos/modules/system/build.nix")
            ];
          }
          // removeAttrs args [ "modules" ]
        );
    };
}
