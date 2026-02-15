{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
    wayland-scanner
    wayland-protocols
    wlr-protocols
    wayland-utils
    wayland
    libxkbcommon
    fontconfig
  ];
  shellHook = ''
    #     bash
      [ -x ~/.bashrc ] && source ~/.bashrc
      export WAYLAND_PROTOCOL_LOC=${pkgs.wayland-protocols}
      export WLR_PROTOCOL_LOC=${pkgs.wlr-protocols}'';
}
