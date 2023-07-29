import os
import glob
from setuptools import find_packages, setup, Extension
import pybind11


def get_extensions():
    this_dir = os.path.dirname(os.path.abspath(__file__))
    extensions_dir = os.path.join(this_dir, "compress_op")
    # main_source=os.path.join(extensions_dir, "function.cpp")
    sources = glob.glob(os.path.join(extensions_dir, "*.cpp")) + glob.glob(os.path.join(extensions_dir, "**", "*.cc")) + glob.glob(os.path.join(extensions_dir, ".h"))

    # sources = [main_source] + sources
    extension = Extension
    # extra_compile_args = {"cxx": ["-fopenmp"]}
    include_dirs = [extensions_dir, pybind11.get_include()]
    ext_modules = [
        extension(
            "rule_compress",
            sources,
            include_dirs=include_dirs,
            language="c++",
            extra_compile_args=["-std=c++17", "-fopenmp", "-O3"],
            extra_link_args=['-lgomp']
        )
    ]
    return ext_modules


setup(
    name="rule_compress",
    version="1.0",
    python_requires=">=3.6",
    description="Serial rule compression",
    ext_modules= get_extensions(),
    install_requires=["pytest", "pybind11"]
)