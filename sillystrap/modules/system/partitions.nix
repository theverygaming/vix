{ lib, ... }:

{
  options = {
    system.partitions = lib.mkOption {
      type = lib.types.attrsOf (
        lib.types.submodule {
          options = {
            files = lib.mkOption {
              type = lib.types.attrsOf lib.types.path;
              default = { };
            };
          };
        }
      );
    };
  };
  config = {
    system.partitions.boot = { };
  };
}
