#!/usr/bin/python2

import optparse
import subprocess
import sys
from builds import GporcaCodegen

def make():
    ciCodegen = GporcaCodegen()
    return subprocess.call(["make",
                            "-j" + str(ciCodegen.num_cpus())], cwd="gpdb_src")

def unittest():
    return subprocess.call(["make", "-s", "-C", "src/backend", "unittest-check"], cwd="gpdb_src")

def install(output_dir):
    subprocess.call(["make", "install"], cwd="gpdb_src")
    subprocess.call("mkdir -p " + output_dir, shell=True)
    return subprocess.call("cp -r /usr/local/gpdb/* " + output_dir, shell=True)

def verifystyle():
    return subprocess.call("./third_party/cpplint/lint_everything.py", cwd="gpdb_src/src/backend/codegen")
    
def main():
    parser = optparse.OptionParser()
    parser.add_option("--build_type", dest="build_type", default="RELEASE")
    parser.add_option("--compiler", dest="compiler")
    parser.add_option("--cxxflags", dest="cxxflags")
    parser.add_option("--output_dir", dest="output_dir", default="install")
    (options, args) = parser.parse_args()
    ciCodegen = GporcaCodegen()
    status = ciCodegen.install_system_deps()
    if status:
        return status
    for dependency in args:
        status = ciCodegen.install_dependency(dependency)
        if status:
            return status
    status = ciCodegen.configure()
    if status:
        return status
    status = make()
    if status:
        return status
    
    status = verifystyle()
    if status:
        return status
    status = unittest()
    if status:
        return status
    status = install(options.output_dir)
    if status:
        return status
    return 0

if __name__ == "__main__":
    sys.exit(main())
