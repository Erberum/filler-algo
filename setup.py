from setuptools import setup
from Cython.Build import cythonize
from setuptools import Extension

setup(
    ext_modules=cythonize([
        Extension('filler_algo.c_src.minimax',
                  ['src/filler_algo/c_src/minimax.c', 'src/filler_algo/c_src/minimax_wrapper.pyx'])
    ])
)
