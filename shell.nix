{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  packages = with pkgs; [
    cmake
    gnutls
    gcc
    just
    gnumake
    icu76.dev
    kakoune
    ncurses
    ftxui
    gpm
  ];
}
