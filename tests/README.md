# Local core tests

These tests do not launch an example or require a windowing system. They exercise numerical and
container code shared by the desktop OpenGL and GLES backends.

Build and run the desktop test in a separate directory:

```sh
mkdir -p /tmp/qwtplot3d-core-tests-desktop
cd /tmp/qwtplot3d-core-tests-desktop
qmake /path/to/qwtplot3d/tests/core_tests.pro
make
./qwtplot3d-core-tests
```

For GLES, use `qmake CONFIG+=gles` instead. Do not reuse the same build directory when switching
between backends.
