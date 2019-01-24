import setuptools

setuptools.setup(
    name="tf-trusted",
    version="0.1.0",
    description="TF-Trusted provides a way to run models inside a Trusted Execution Environment",
    long_description=open("README.md").read(),
    url="https://github.com/dropoutlabs/tf-trusted",
    author="Justin Patriquin",
    author_email="justin@dropoutlabs.com",
    classifiers=[
        "Environment :: Console",
        "Intended Audience :: Developers",
        "Intended Audience :: Education",
        "Intended Audience :: Science/Research",
        "Operating System :: MacOS :: MacOS X",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX",
        "Operating System :: Unix",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
        "Topic :: Software Development :: Libraries",
        "Topic :: Software Development :: Testing",
    ],
    keywords=(
        "ai "
        "artificial intelligence "
        "privacy "
        "cryptography "
        "machine learning "
    ),
    install_requires=[
        "numpy"
    ],
    packages=setuptools.find_packages(),
    zip_safe=True,
    license="Apache License 2.0",
    scripts=['tf_trusted_custom_op/model_run.py'],
    package_data={
        '':['*.so'],
    }
)