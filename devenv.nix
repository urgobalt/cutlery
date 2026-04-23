{
  pkgs,
  lib,
  config,
  inputs,
  ...
}: {
  packages = with pkgs; [
    gcc
    clang-tools
    just
    cpplint
  ];
}
