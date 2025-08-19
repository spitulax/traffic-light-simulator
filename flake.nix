{
  description = "Traffic Light Simulator";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs, ... }:
    let
      inherit (nixpkgs) lib;
      systems = [ "x86_64-linux" "aarch64-linux" ];
      eachSystem = f: lib.genAttrs systems f;
      pkgsFor = eachSystem (system:
        import nixpkgs {
          inherit system;
          overlays = [
            self.overlays.libs
            self.overlays.default
          ];
        });
    in
    {
      overlays = import ./nix/overlays.nix { inherit self lib; };

      packages = eachSystem (system:
        let
          pkgs = pkgsFor.${system};
        in
        {
          default = self.packages.${system}.traffic-light-simulator;
          inherit (pkgs) traffic-light-simulator;
        });

      devShells = eachSystem (system:
        let
          pkgs = pkgsFor.${system};
        in
        {
          default = pkgs.callPackage ./nix/shell.nix { inherit self; };
        }
      );
    };
}
