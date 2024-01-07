#substitute.py
#
# Copyright 2019-present Sebastien Vavassori. All rights reserved.
# Use of this source code is governed by a LGPL license that can be
# found in the LICENSE file.
#
import getopt
import os
import sys

from shutil import copytree, make_archive

__all__ = [
    'make_addons',
]


CURRENT_PATH = os.path.dirname(os.path.realpath(__file__))
VERSION_FILE = os.path.join(CURRENT_PATH, "..", "..", "version")
FAKE_VERSION = "0.0.65536"

def apply_version(version, manifest_file):
    """substitute fake version with current version"""
    new_file = manifest_file + "_temp"
    with open(manifest_file, "rt") as my_file:
        with open(new_file, "wt") as output_file:
            n = 0
            for line in my_file:
                n += 1
                if FAKE_VERSION in line:
                    print(f"Replacing version at line {n} in file '{manifest_file}'")
                output_file.write(line.replace(FAKE_VERSION, version))
    os.remove(manifest_file)
    os.rename(new_file, manifest_file)


def _logpath(path, names):
    #print(f"  copy ignore: {path}")
    return []  # nothing will be ignored


def make_addons(current_path, output_path):
    print("Making Browser Addons...")
    print(f"Current directory: {current_path}")
    print(f"Output directory: {os.path.abspath(output_path)}")

    print("Making Chromium Addon...")
    copytree(os.path.join(current_path, "src", "base"), os.path.join(output_path, "chromium"), ignore=_logpath, dirs_exist_ok=True)
    copytree(os.path.join(current_path, "src", "chromium"), os.path.join(output_path, "chromium"), ignore=_logpath, dirs_exist_ok=True)

    print("Making Firefox Addon...")
    copytree(os.path.join(current_path, "src", "base"), os.path.join(output_path, "firefox"), ignore=_logpath, dirs_exist_ok=True)
    copytree(os.path.join(current_path, "src", "firefox"), os.path.join(output_path, "firefox"), ignore=_logpath, dirs_exist_ok=True)

    print("Reading version...")
    application_version = ""
    with open(VERSION_FILE, "r") as my_file:
        application_version = my_file.readline()
        application_version = ''.join(application_version.split())

    print(f"Application version: {application_version}")

    print("Updating version...")
    apply_version(application_version, os.path.join(output_path, "chromium", "manifest.json"))
    apply_version(application_version, os.path.join(output_path, "firefox", "manifest.json"))

    print("Archiving...")  
    base_name = f"ArrowDL_chromium_v{application_version}"
    chromium_full_name = make_archive(base_name=base_name, format="zip", root_dir=os.path.join(output_path, "chromium"), base_dir=".")
    print(f"Created archive: {chromium_full_name}")

    base_name = f"ArrowDL_firefox_v{application_version}"
    name = make_archive(base_name=base_name, format="zip", root_dir=os.path.join(output_path, "firefox"), base_dir=".")
    firefox_full_name = name.replace(".zip", ".xpi")
    if os.path.exists(firefox_full_name):
        os.remove(firefox_full_name)
    os.rename(name, firefox_full_name)
    print(f"Created archive: {firefox_full_name}")

    result = {
        'chromium': chromium_full_name,
        'firefox': firefox_full_name,
    }
    return result

if __name__ == "__main__":
    args = sys.argv[1:]

    output = CURRENT_PATH
    try:
        opts, args = getopt.getopt(args=args, shortopts=":ho", longopts=["help", "output-directory="])
    except getopt.GetoptError as ex:
        print(ex)
        print('Type make.py -h for Help')
        exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('make.py [-o <dir>]')
            print("   -h, --help: this message.")
            print("   -o, --output-directory: Optional. Output directory.")
            print()
            exit(0)
        elif opt in ("-o", "--output-directory"):
            try:
                output = os.path.normpath(arg)
            except:
                print(f"Error: '{arg}' is not a directory.")
                exit(2)

    make_addons(current_path=CURRENT_PATH, output_path=output)
