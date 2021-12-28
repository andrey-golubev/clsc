CLSC (Cute Little Software Capital)
===

[![Coverity Scan Build Status](https://scan.coverity.com/projects/17074/badge.svg)](https://scan.coverity.com/projects/clsc)
[![Build Status](https://travis-ci.com/andrgolubev/clsc.svg?branch=master)](https://travis-ci.com/andrey-golubev/clsc)


## About

This is a toy-project dedicated to creating useful utilities that
relatively modern `C++` (`C++17`) misses. There is also an on-going
work on quite interesting features not currently available in `master`.

## Fast start

~~~bash
git clone https://github.com/andrey-golubev/clsc.git --recursive
cd clsc
python3 script/test.py
~~~

> Note: [googletest](https://github.com/google/googletest) is used as a
**git submodule**. Thus, it is required to clone it in order to build and run
repository tests.
