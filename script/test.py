#!/usr/bin/env python3

import argparse
import os
import sys
import subprocess
import shutil


def parse_args():
    parser = argparse.ArgumentParser('Command-line options for build script')
    parser.add_argument('--debug', action='store_true', default=False,
        help='Specify whether debug build should be use')
    parser.add_argument('--cmake', default='', nargs='*',
        help='Pass raw arguments to cmake')
    return parser.parse_args()


def run_steps(steps, work_dir='.'):
    """Run \n separated commands"""
    for step in steps.splitlines():
        step = step.strip()
        if not step:
            continue
        step_name = step.split(' ')[0]
        sys.stdout.write('[{name}]\n'.format(name=step_name))
        subprocess.check_call(step, shell=True, cwd=work_dir)
        sys.stdout.flush()


def test(debug, cmake_args, work_dir='.'):
    t = 'DEBUG' if debug else 'RELEASE'
    build_type = '-DCMAKE_BUILD_TYPE={t}'.format(t=t)
    steps = """
    cmake {build_type} {args} ..
    make VERBOSE=1
    ./tests/clsc_tests
    """.format(build_type=build_type, args=cmake_args)
    run_steps(steps, work_dir=work_dir)


def main():
    """Main entrypoint"""
    args = parse_args()
    workdir = os.path.join(os.path.dirname(__file__), '..', 'build')
    if os.path.exists(workdir):
        shutil.rmtree(workdir)
    os.makedirs(workdir)
    cmake_args = args.cmake
    for i, arg in enumerate(cmake_args):
        if not arg.startswith('-D'):
            cmake_args[i] = '-D' + arg
    test(args.debug, cmake_args, workdir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
