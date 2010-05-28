#!/usr/bin/env python

#A bad, hacked-up distutils compilation script :D

#############################[ Parameters you should change if install failed ]#########################################
# python_dir should point to the directory where Python header files may be found.. (Inside this dir you should have a Python.h)
python_dir="/usr/include/python"
# qtdir should point to the directory where QT is installed. (Inside this dir you should have a lib and include dir)
qtdir=["/usr/share/qt4/"]      # ["/usr/qt/3/"] if you have a Gentoo system

# QT library to use. If qt-mt is not found, you may need to change it to "qt"
qtlibraries = ["qt-mt",
               ]
#############################[ End of parameters that can be changed ]##################################################

try:
    import sys,commands,traceback,os
    from distutils import sysconfig
    from distutils.core import setup,Extension
    from distutils.command.build_ext import build_ext
    from distutils.errors import CCompilerError
    from string import *
except:
    traceback.print_exc()
    print "Unable to import python distutils."
    print "You may want to install the python-dev package on your distribution."
    sys.exit(1)

############## Init some vars
# python build
extra_compile_args=["-O3","-DLinuxBuild"]
#extra_link_args=['-static'] # won't compile on AMD_64 with "static" 
extra_link_args=[]
include_dirs = []
library_dirs = []
libraries = []

print "#################################### Check Python"
if int(sys.version[0]) < 2:             # check py version and Python include dir
    print "--- WARNING ---\nYou need Python version 2.x or more. If something doesn't work or install fails, please update Python."
if python_dir[-1] != os.sep: python_dir=python_dir+os.sep
if not os.path.exists(python_dir+"Python.h"):
    python_dir=sysconfig.get_python_inc() # get help from distutils sysconfig submodule
    if python_dir[-1] != os.sep: python_dir=python_dir+os.sep
    if not os.path.exists(python_dir+"Python.h"): # give up...
        print "--- WARNING ---\nUnable to find python development files. Make sure you have the python-devel package for your GNU/Linux distribution installed."
        print "If that didn't help, you may need to locate the place where the python C headers are stored and change the line in setup.py where \"/usr/include/python2.3\" is found. (on the beginning of setup.py)"
include_dirs.append(python_dir)
print "Checked."

print "#################################### Check QT"

include_dirs.append("/usr/include/qt4")
library_dirs.append("/usr/lib/qt4")
library_dirs.append("/usr/lib/")
gqtdir="/usr/lib/qt4"

if gqtdir:                              # more checks
    if gqtdir[-1] != os.sep: gqtdir=gqtdir+os.sep
    if gqtdir not in qtdir: qtdir.append(gqtdir)

for qdir in qtdir:
    include_dirs.append(qdir+"include")
    library_dirs.append(qdir+"lib")

libraries = qtlibraries
print "Checked."

class fallible_build_ext(build_ext):
    """the purpose of this class is to know when a compile error ocurred """
    def run(self):
        try:
            build_ext.run(self)
        except CCompilerError:
            traceback.print_exc()


# force C++ linking
from distutils import sysconfig
config_vars = sysconfig.get_config_vars()
for k, v in config_vars.items():
    if k.count('LD') and str(v).startswith('gcc'):
        print "+++++++++++++++++++++++++++++++++++++++++++++"
        config_vars[k] = v.replace('gcc', 'g++')

            
print "#################################### Installing"
setup(#name="isk-daemon",
      #version=0.4,
      #description="",
      #long_description =""".""",
      #author="Ricardo Niederberger Cabral",
      #author_email="rnc000|at|gmail.com",
      #url="http://www.imgseek.net/daemon",
      platforms = ['Linux'],
      cmdclass = { 'build_ext': fallible_build_ext},
      #license = 'Trial-Version',
      #packages=['imgSeekLib', 'Cheetah','iskLib'],
      #scripts= ['isk-daemon.py','settings.py','web.py','isk-genkey.py'],
      ext_modules = [
        Extension("libimgdb",["imgdb.cpp",
                           "haar.cpp",
                           "bloom_filter.cpp"
                                      ],
                  include_dirs = include_dirs,
                  library_dirs = library_dirs,
                  extra_compile_args=extra_compile_args,
                  extra_link_args=extra_link_args,
                  libraries = libraries,
                  swig_opts = ['-c++']
                 ),
        Extension("libimgdb-dbg",["imgdb.cpp",
                           "haar.cpp",
                           "bloom_filter.cpp"
                                      ],
                  include_dirs = include_dirs,
                  library_dirs = library_dirs,
                  extra_compile_args=extra_compile_args + ["-DDebugLib -ggdb"],
                  extra_link_args=extra_link_args + ["-ggdb"],
                  libraries = libraries,
                  swig_opts = ['-c++']
                 )]
      
     )


