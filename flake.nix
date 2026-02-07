{
  description = "Physbuzz Engine flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.11";
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
      devShells.${system}.default = pkgs.mkShell.override { stdenv = pkgs.llvmPackages.libcxxStdenv; } {
        packages = [
          # compiling
          pkgs.cmake
          pkgs.ninja
          pkgs.shader-slang
          (pkgs.llvmPackages.clang-tools.override { enableLibcxx = true; })
          pkgs.shaderc

          # debugging
          pkgs.valgrind
          pkgs.renderdoc
          pkgs.tracy
          pkgs.vscode-extensions.vadimcn.vscode-lldb.adapter

          # libraries
          pkgs.assimp
          pkgs.glfw
          pkgs.spdlog
          pkgs.glm
          pkgs.catch2_3
          pkgs.vulkan-headers
          pkgs.vulkan-loader
          pkgs.vulkan-memory-allocator
          pkgs.vulkan-tools
          pkgs.vulkan-tools-lunarg
          pkgs.vulkan-validation-layers
        ];

        # https://github.com/NixOS/nixpkgs/issues/18995
        hardeningDisable = [ "all" ];

        VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
      };
    };
}
