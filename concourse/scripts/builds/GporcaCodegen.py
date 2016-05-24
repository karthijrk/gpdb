import os
import subprocess
import sys

from GporcaCommon import GporcaCommon

class GporcaCodegen(GporcaCommon):
    def configure(self):
        return subprocess.call(["./configure",
                                "--enable-orca",
                                "--enable-codegen",
                                "--enable-mapreduce",
                                "--with-perl",
                                "--with-libxml",
                                "--with-python",
                                "--disable-gpfdist",
                                "--prefix=/usr/local/gpdb"], cwd="gpdb_src")
    
