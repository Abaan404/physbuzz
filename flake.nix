{
  description = "Physbuzz Engine flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = [
          # compiling
          pkgs.cmake
          pkgs.ninja
          pkgs.shader-slang
          pkgs.clang-tools
          pkgs.shaderc

          # debugging
          pkgs.valgrind
          pkgs.renderdoc
          pkgs.tracy
          pkgs.vscode-extensions.vadimcn.vscode-lldb.adapter

          # libraries
          pkgs.assimp
          pkgs.glfw-wayland
          pkgs.spdlog
          pkgs.glm
          pkgs.catch2_3
          pkgs.vulkan-headers
          pkgs.vulkan-loader
        ];

        # https://github.com/NixOS/nixpkgs/issues/18995
        hardeningDisable = [ "all" ];
      };
    };
}
