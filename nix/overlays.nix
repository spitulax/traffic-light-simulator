{ self
, lib
}:
{
  default = self.overlays.traffic-light-simulator;

  traffic-light-simulator = final: prev: {
    traffic-light-simulator = prev.callPackage ./default.nix { inherit (prev.myLib) version; };
  };

  libs = final: prev: {
    myLib = import ./lib.nix { inherit self lib; };
  };
}
