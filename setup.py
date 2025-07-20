from setuptools import setup, Extension
from Cython.Build import cythonize

setup(
    ext_modules=cythonize([
        Extension('minimax',
                  ['c_src/minimax_wrapper.pyx', 'c_src/minimax.c'],
                  extra_compile_args=['-Wall', '-O3'])
    ]),
)
