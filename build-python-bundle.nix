{pkgs ? import <nixpkgs> {}}:
let
  nix-bundle =
    let
      tarball = builtins.fetchTarball {
        url = "https://github.com/matthewbauer/nix-bundle/archive/223f4ffc4179aa318c34dc873a08cb00090db829.tar.gz";
      };
    in pkgs.callPackage tarball { nixpkgs = pkgs; };
  python-env = pkgs.python3.withPackages (pkgs: with pkgs; [ numpy ]);
in nix-bundle.nix-bootstrap {
  target = python-env;
  run = "/bin/python";
}
