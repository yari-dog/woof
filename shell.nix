{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    wayland-scanner
    wayland-protocols
    wayland-utils
    wayland
    libxkbcommon
    fontconfig
  ];
  shellHook = ''
    #     zsh
      [ -x ~/.zshrc ] && source ~/.zshrc'';
}
