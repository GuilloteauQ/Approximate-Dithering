{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/23.05";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {

      devShells.${system} = {
        default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            gnumake
            linuxKernel.packages.linux_6_1.perf
            valgrind
            gdb
          ];
        };
        rshell = pkgs.mkShell {
          buildInputs = with pkgs; [
            (rWrapper.override{ packages = with rPackages; [ tidyverse ]; })
          ];
        };
        notes = pkgs.mkShell {
          buildInputs = with pkgs; [
            emacs
          ];
          shellHook = ''
            ${pkgs.emacs}/bin/emacs -q -l ./.init.el notes.org
            exit
          '';
        };
      };

    };
}
