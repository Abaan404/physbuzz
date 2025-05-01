{
  description = "Physbuzz Engine flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      lib = pkgs.lib;
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = [
          # tooling
          pkgs.cmake
          pkgs.ninja
          pkgs.valgrind
          pkgs.vscode-extensions.vadimcn.vscode-lldb.adapter

          # libraries
          pkgs.assimp
          pkgs.glfw-wayland
          pkgs.spdlog
          pkgs.glm
          pkgs.catch2_3
          pkgs.python312Packages.glad2
        ];

        # https://github.com/NixOS/nixpkgs/issues/18995
        hardeningDisable = [ "all" ];

        shellHook = ''
          export LD_LIBRARY_PATH="${lib.makeLibraryPath [ pkgs.libGL ]}";
        '';
      };
    };
}
