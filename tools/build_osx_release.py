#!/usr/bin/env python

# Copyright (C) 2011 Fabiano Fidencio
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.


from os import chdir, environ, getcwd, listdir, mkdir, path
from shutil import copy2, rmtree
from subprocess import PIPE, Popen
from sys import argv, exit
from argparse import ArgumentParser


def environment_prepare():
    abisource_path="/tmp/abisource"
    mkdir(abisource_path, 0755)
    path = getcwd()
    chdir(abisource_path)
    return path

def environment_clean(path):
    chdir(path)
    abisource_path="/tmp/abisource"
    rmtree(abisource_path)

def _macports_source_get():
    source = "https://distfiles.macports.org/MacPorts/MacPorts-2.0.0.tar.gz"
    cmd = "curl -O %s" % source
    p = Popen(cmd, shell=True)
    p.wait()

def _macports_source_extract():
    cmd = "tar xzvpf MacPorts-2.0.0.tar.gz"
    p = Popen(cmd, shell=True)
    p.wait()

def _macports_install():
    current_dir = getcwd()
    chdir("MacPorts-2.0.0")
    cmd = "./configure --prefix=/tmp/abisource/macports \
            && make \
            && sudo make install"
    p = Popen(cmd, shell=True)
    p.wait()
    chdir(current_dir)

def _macports_env():
    macports_path = "/tmp/abisource/macports/"
    envs = environ
    env = "%s/bin:%s/sbin:%s" % (macports_path, macports_path, envs["PATH"])
    return env

def _macports_sync():
    envs = _macports_env()
    cmd = "sudo port -v selfupdate"
    p = Popen(cmd, shell=True, env={"PATH":envs})
    p.wait()

def macports_install():
    _macports_source_get()
    _macports_source_extract()
    _macports_install()
    _macports_sync()

def dependencies_install():
    envs = _macports_env()
    pkgs = "cairo +quartz+no_x11 \
            pango +quartz+no_x11 \
            fribidi \
            libgsf +no_gnome \
            redland \
            wv +no_x11 \
            enchant \
            boost"
    cmd = "sudo port install %s" % pkgs
    p = Popen(cmd, shell=True, env={"PATH":envs})
    p.wait()

def _abiword_source_get():
    cmd = "svn co http://svn.abisource.com/abiword/trunk abiword"
    p = Popen(cmd, shell=True)
    p.wait()

def _abiword_fix_macports_path():
    cmd = "sed -i -e \
            's/\\/opt\\/local/\\/tmp\\/abisource\\/macports/g' \
            configure.in"
    p = Popen(cmd, shell=True)
    p.wait()

def _abiword_install():
    envs = _macports_env()
    current_dir = getcwd()
    chdir("abiword")
    _abiword_fix_macports_path()
    cmd = "./autogen.sh \
            --with-darwinports \
            --enable-maintainer-mode \
            --disable-static \
            --enable-shared \
            --enable-plugins=\"docbook epub latex openwriter openxml opml\" \
            && make && DESTDIR=`pwd` make install"
    p = Popen(cmd, shell=True, env={"PATH":envs})
    p.wait()
    chdir(current_dir)

def abiword_install():
    _abiword_source_get()
    _abiword_install()

def _dep_list_get(lib):
    #otool -L path
    cmd =  "otool -L %s " %lib
    #get all .dylib from otool -L
    cmd += "| grep macports | sed -e 's/.dylib.*$/.dylib/'"
    #remove white spaces before and after the lib path/name
    cmd += "| sed 's/^[ \t]*//;s/[ \t]*$//'"

    p = Popen(cmd, shell=True, stdout=PIPE)
    p.wait()
    stdout = p.communicate()

    return stdout[0].split('\n')[:-1]

def _rdeps_get():
    contents_path = "abiword/AbiWord.app/Contents"

    libabiword = ""
    libabiword_deps = []
    for content in listdir(contents_path + "/Frameworks"):
        if content.endswith(".dylib"):
            libabiword = contents_path + "/Frameworks/" + content
            libabiword_deps = _dep_list_get(libabiword)
            break

    plugins = []
    plugins_deps = []
    for content in listdir(contents_path + "/PlugIns"):
        if content.endswith(".so"):
            plugin = contents_path + "/PlugIns/" + content
            plugins.append(plugin)
            plugins_deps = _dep_list_get(plugin)

    abiword = contents_path + "/MacOS/AbiWord"
    abiword_deps = _dep_list_get(abiword)

    rdeps = []
    for lib in libabiword_deps:
        rdeps.append(lib)

    for lib in plugins_deps:
        if lib not in rdeps:
            rdeps.append(lib)

    for lib in abiword_deps:
        if lib not in rdeps:
            rdeps.append(lib)

    rdeps_deps = []
    for lib in rdeps:
        rdeps_deps += _dep_list_get(lib)

    for lib in rdeps_deps:
        if lib not in rdeps_deps:
            rdeps.append(lib)

    return rdeps, libabiword, abiword, plugins

def _rdeps_copy(rdeps):
    rdeps_path = "abiword/AbiWord.app/Contents/Frameworks/rdeps"
    mkdir(rdeps_path, 0755)

    n_rdeps = []
    for dep in rdeps:
        dep_path, dep_name = path.split(dep)
        copy2(dep, rdeps_path)
        d = "%s/%s" % (rdeps_path, dep_name)
        cmd = "chmod 755 " + d
        n_rdeps.append(d)
        p = Popen(cmd, shell=True)
        p.wait()

    return n_rdeps

def _fix(lib, new):
    dep_list = _dep_list_get(lib)

    for d in dep_list:
        d_path, d_name = path.split(d)
        n = "@executable_path/../Frameworks/rdeps/" + d_name
        cmd = "install_name_tool -change %s %s %s" % (d, n, lib)
        p = Popen(cmd, shell=True)
        p.wait()

    lib_path, lib_name = path.split(lib)
    cmd = "install_name_tool -id %s %s" % (new, lib)
    p = Popen(cmd, shell=True)
    p.wait()

def _rdeps_fix(rdeps):
    for r in rdeps:
        file_path, file_name = path.split(r)
        new = "@executable_path/../Frameworks/rdeps/" + file_name
        _fix(r, new)

def _libabiword_fix(libabiword):
    file_path, file_name = path.split(libabiword)
    new = "@executable_path/../Frameworks/" + file_name
    _fix(libabiword, new)

def _abiword_fix(abiword):
    file_path, file_name = path.split(abiword)
    new = "@executable_path/" + file_name
    _fix(abiword, new)

def _plugins_fix(plugins):
    for p in plugins:
        file_path, file_name = path.split(p)
        new = "@executable_path/../PlugIns/" + file_name
        _fix(p, new)

def do_app():
    rdeps, libabiword, abiword, plugins = _rdeps_get()
    n_rdeps = _rdeps_copy(rdeps)
    _rdeps_fix(n_rdeps)
    _libabiword_fix(libabiword)
    _abiword_fix(abiword)
    _plugins_fix(plugins)

def do_dmg():
    mkdir("dmg", 0755)
    cmd = "cp -a abiword/AbiWord.app dmg/"
    p = Popen(cmd, shell = True)
    p.wait()
    cmd = "ln -s /Applications dmg/"
    p = Popen(cmd, shell=True)
    p.wait()
    cmd = "hdiutil create \
            -srcfolder \"dmg\" \
            -volname \"AbiWord\" \
            -fs HFS+ \
            -fsargs \"-c c=64,a=16,e=16\" \
            -format UDRW \"AbiWord.dmg\""
    p = Popen(cmd, shell=True)
    p.wait()
    rmtree("dmg")
    copy2("AbiWord.dmg", environ["HOME"] + "/Desktop/")


if __name__ == "__main__":
    parser = ArgumentParser(description="Automated dmg generator")
    parser.add_argument("--macports_path",
                        action="store",
                        dest="macports_path",
                        help="This option will use your current macports' \
                              installation from MACPORTS_PATH.\n\
                              ATTENTION: Without this option, macports will \
                              be downloaded and installed in: \
                              /tmp/abisource/macports")

    parser.add_argument("--abisource_path",
                        action="store",
                        dest="abi_path",
                        default=False,
                        help="This option will consider that you have \
                              AbiWord's sources in your computer, located at \
                              ABISOURCE_PATH and want to build it and NOT a \
                              specific version from our SVN.")

    parser.add_argument("--abisource_revision",
                        action="store",
                        dest="abi_rev",
                        help="This option will get a specific revision from \
                              AbiWord's SVN. \
                              ATTETION: If this option isn't passed, SVN's \
                              trunk will be used.")

    parser.add_argument("--abiword_version",
                        action="store",
                        dest="abi_version",
                        help="This option will get a specific version from \
                              AbiWord's SVN. \
                              ATTETION: If this option isn't passed, SVN's \
                              trunk will be used.")


    parser.add_argument("--no_deps",
                        action="store_true",
                        dest="no_deps",
                        default=False,
                        help="This option won't install AbiWord's \
                              dependencies in your computer. So, is YOUR \
                              WORK install all needed dependencies. Of \
                              course, you'll need to install macports before.")

    parser.add_argument("--start_from_build",
                        action="store_true",
                        dest="start_from_build",
                        default=False,
                        help="This option will consider that you have \
                              macports and all AbiWord's dependencies \
                              installed. \
                              ATTENTION: This options will build AbiWord and \
                              create a dmg file. So, is REALLY NECESSARY \
                              that you pass --abisource_path option.")

    parser.add_argument("--start_from_app",
                        action="store",
                        dest="start_from_app",
                        help="This option will use a generated .app file \
                              to fix all linkage and put all nedded libs \
                              into .app in a specific folder. After that a \
                              dmg file will be created. \
                              ATTENTION: Is REALLY NECESSARY that you pass \
                              --macports_path option.")

    parser.add_argument("--start_from_linkage_fixed",
                        action="store",
                        dest="start_from_linkage_fixed",
                        help="This option will use a generated .app file \
                              with linkage working properly to create a \
                              .dmg file.\
                              ATTENTION: Is REALLY NECESSARY that you pass \
                              --macports_path option.")

    if len(argv) < 2:
        parser.print_help()
        exit()
    else:
        args = parser.parse_args()

    current_dir = getcwd()
    environment_prepare()
    macports_install()
    dependencies_install()
    abiword_install()
    do_app()
    do_dmg()
    environment_clean(current_dir)
    print "****************************************************"
    print "* AbiWord.dmg was created in you ~/Desktop. Enjoy! *"
    print "****************************************************"
