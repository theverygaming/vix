{ lib, ... }:

{
  options = {
    boot.loader.limine = {
      extraConfig = lib.mkOption {
        default = "";
        type = lib.types.lines;
      };
    };
  };
}
