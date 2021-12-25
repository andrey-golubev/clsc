#!/usr/bin/env python3

# Copyright 2018-2021 Andrey Golubev
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# ANDANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import argparse
import os
import sys
import subprocess
import shutil


def parse_args():
    parser = argparse.ArgumentParser('Command-line options for build script',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--rebuild', action='store_true', default=False,
                        help='Rebuild.')
    parser.add_argument('--debug', action='store_true', default=False,
                        help='Build in debug mode.')
    parser.add_argument('--cmake-args', default=[], nargs='*',
                        help='Pass raw arguments to cmake.')
    parser.add_argument('--dev-build', action='store_true', default=False,
                        help='Enable developer build configuration.')
    return parser.parse_args()


def run_steps(steps, work_dir='.'):
    """Run \n separated commands"""
    for step in steps.splitlines():
        step = step.strip()
        if not step:
            continue
        sys.stdout.write('[{command}]\n'.format(command=step))
        subprocess.check_call(step, shell=True, cwd=work_dir)
        sys.stdout.flush()


def test(debug_mode, cmake_args, work_dir='.'):
    if debug_mode:
        cmake_args.append('-DCMAKE_BUILD_TYPE=DEBUG')
    steps = """
    cmake {args} ..
    make VERBOSE=1
    ./tests/clsc_tests
    """.format(args=' '.join(cmake_args))
    run_steps(steps, work_dir=work_dir)


def main():
    """Main entrypoint"""
    args = parse_args()
    sourcedir = os.path.join(os.path.dirname(__file__), '..')
    workdir = os.path.join(sourcedir, 'build')

    # setup build/ dir
    if os.path.exists(workdir):
        if args.rebuild:
            shutil.rmtree(workdir)
            os.makedirs(workdir)
    else:
        os.makedirs(workdir)

    cmake_args = args.cmake_args
    for i, arg in enumerate(cmake_args):
        if not arg.startswith('-D'):
            cmake_args[i] = '-D' + arg

    if args.dev_build:
        cmake_args.append('-DCMAKE_EXPORT_COMPILE_COMMANDS=1')

    test(args.debug, cmake_args, workdir)

    if not args.dev_build:
        return 0

    try:
        os.symlink(os.path.join(workdir, 'compile_commands.json'),
                   os.path.join(sourcedir, 'compile_commands.json'))
    except:
        print('os.symlink failed')
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
