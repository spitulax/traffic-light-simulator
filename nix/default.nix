{ stdenv
, lib
, meson
, ninja
, raylib
, pkg-config
, version ? "git"
}:
stdenv.mkDerivation {
  pname = "traffic-light-simulator";
  inherit version;
  src = lib.cleanSource ./..;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    raylib
  ];

  buildInputs = [

  ];

  mesonBuildType = "release";

  meta = {
    description = "Traffic Light Simulator";
    mainProgram = "traffic-light-simulator";
    homepage = "https://github.com/spitulax/traffic-light-simulator";
    license = lib.licenses.mit;
    platforms = raylib.meta.platforms;
  };
}
