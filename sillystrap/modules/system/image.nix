{
  pkgs,
  config,
  lib,
  ...
}:

{
  system.build.image = pkgs.runCommand "boot-image" { } ''
    LIMINE_PATH=$(${pkgs.buildPackages.limine}/bin/limine --print-datadir)

    dd if=/dev/zero of=image.img bs=1M count=10 # create empty 10MB disk image

    # https://superuser.com/a/984637
    sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | ${pkgs.buildPackages.util-linux}/bin/fdisk image.img
      o    # create DOS partition table
      n    # new partition
      p    # primary partition
      1    # partition number 1
      2048 # start partition at 2048
          # default - end of disk
      a    # set bootable flag on created partition
      p    # print partition table
      w    # write the partition table
      q    # quit
    EOF

    dd if=image.img of=image_extracted.img bs=512 skip=2048 # extract partition
    ${pkgs.buildPackages.dosfstools}/bin/mkfs.fat image_extracted.img

    ${pkgs.buildPackages.mtools}/bin/mmd -i image_extracted.img ::boot

    ${pkgs.buildPackages.mtools}/bin/mcopy -i image_extracted.img ''${LIMINE_PATH}/limine-bios.sys ::boot/
    ${pkgs.buildPackages.mtools}/bin/mcopy -i image_extracted.img ${pkgs.writeText "limine.conf" config.boot.loader.limine.extraConfig} ::boot/limine.conf
    ${lib.strings.concatStringsSep "\n" (
      lib.attrsets.mapAttrsToList (
        p: f: "${pkgs.buildPackages.mtools}/bin/mcopy -i image_extracted.img \"${f}\" \"::${p}\""
      ) config.system.partitions.boot.files
    )}

    ${pkgs.buildPackages.mtools}/bin/mmd -i image_extracted.img ::EFI
    ${pkgs.buildPackages.mtools}/bin/mmd -i image_extracted.img ::EFI/BOOT
    ${pkgs.buildPackages.mtools}/bin/mcopy -i image_extracted.img ''${LIMINE_PATH}/BOOTIA32.EFI ::EFI/BOOT/BOOTIA32.EFI

    dd if=image_extracted.img of=image.img bs=512 seek=2048 # write partition back to image
    ${pkgs.buildPackages.limine}/bin/limine bios-install image.img

    cp image.img $out
  '';
}
