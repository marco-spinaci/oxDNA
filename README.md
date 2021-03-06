# oxDNA

**This repository contains a new version of oxDNA. If you are not a developer or a tester we advise you to use the [sourceforge](https://sourceforge.net/projects/oxdna/) repo.**

oxDNA is a simulation code that was initially conceived as an implementation of the coarse-grained DNA model introduced by [T. E. Ouldridge, J. P. K. Doye and A. A. Louis](http://dx.doi.org/10.1063/1.3552946). It has been since reworked and it is now an extensible simulation+analysis framework. It natively supports DNA, RNA, Lennard-Jones and patchy particle simulations of different kinds on both CPU and NVIDIA GPUs.

The development of this software has been partially supported by the European Commission through the Marie Skłodowska−Curie Fellowship No. 702298-DELTAS.

## Requirements

The code requires `cmake` and a c++-14-compliant `g++` (any version >= 4.9 *should* work). The code should be also compilable with the Intel compiler (with the `-DIntel=ON` `cmake` flag, see below), although this has not been tested with newer oxDNA versions.

### CUDA

Compiling with CUDA support requires `cmake` >= 3.5 and a CUDA toolkit >= 9.0. If your current setup cannot meet these requirements we advise you to use older versions of oxDNA.

### Python bindings

Python bindings require `Python 3`'s binaries, libraries and include files. On Debian-derived distros these can be installed by installing the `libpython3-dev` package.

[Sphinx](https://www.sphinx-doc.org/en/master), [sphinx_rtd_theme](https://github.com/readthedocs/sphinx_rtd_theme) and [recommonmark](https://recommonmark.readthedocs.io/en/latest/) are required to generate the Python bindings' documentation. Those can all be installed by using `pip` (for instance with the command `pip3 install --user sphinx sphinx_rtd_theme recommonmark`).

### Documentation

Generating the documentation of the C++ code (with `make docs`, see below) requires [doxygen](http://www.doxygen.nl/).

Doc files for the Python bindings can be browsed by opening the `docs/oxpy/html/index.html` file with a web browser.

## Compiling oxDNA

Extract the oxDNA archive and then:

```
cd oxDNA         # enter the oxDNA folder
mkdir build      # create a new build folder. It is good practice to compile out-of-source
cd build
cmake ..         # here you can specify additional options, see next section
make -j4         # compile oxDNA. The -jX make option makes it compile the code in parallel by using X threads.
```

At the end of the compilation three executables (oxDNA, DNAnalysis and confGenerator) will be placed in the `build/bin` directory. 

Compiling with Python bindings will also generate a `oxpy.so` library file in the `build/oxpy/oxpy` directory that can be imported in Python. Running `make install` will attempt to copy this library to the local user's module directory.

## `cmake` options

* `-DCUDA=ON` Enables CUDA support
* `-DCUDA_COMMON_ARCH=ON` Choose the target CUDA compute architecture based on the nvcc version. Set it to off to autodetect the CUDA compute arch GPU installed.
* `-DDebug=ON` Compiles with debug symbols and without optimisation flags
* `-DG=ON` Compiles with debug symbols + optimisation flags
* `-DINTEL=ON` Uses INTEL's compiler suite
* `-DMPI=ON` Compiles oxDNA with MPI support
* `-DSIGNAL=OFF` Handling system signals is not always supported. Set this flag to OFF to remove this feature
* `-DMOSIX=ON` Makes oxDNA compatible with MOSIX
* `-DPython=ON` Enables Python bindings
* `-DDOUBLE=OFF` Set the numerical precision of the CPU backends to `float`
* `-DCUDA_DOUBLE=ON` Set the numerical precision of the CUDA backends to `double`, which is not compatible with the `mixed` precision.

## `make` targets

* `make` compiles oxDNA
* `make docs` Produces html doxygen documentation for oxDNA (`DOCS/html_oxDNA/index.html`) and for the UTILS folder (`DOCS/html_UTILS/index.html`)
* `make rovigatti` Compiles the observables and interactions in contrib/rovigatti
* `make romano` Compiles the observables and interactions in contrib/romano
* `make install` Copies the `oxpy` library to the user's Python library folder
	
## Testing

* `make test_run` Runs quick tests to check whether oxDNA has been correctly compiled or not.	
* `make test_quick` Runs longer tests to check that oxDNA works (not fully implemented yet, but the main features are supported). 

## Usage

### Binaries

* `oxDNA input_file`
* `confGenerator input_file [box_size|density]` the second argument will be interpreted as a number density (number of particles divided by volume) if it is less than 2.0
* `DNAnalysis input_file`

Most of the options that can be specified in the input file can be found in the `input_options.md` file.

### Python Bindings

The API are still unstable, and only few features are exposed through this interface. A basic documentation can be accessed by browsing `docs/oxpy/html/index.html`.  

The following snippet shows a very simple example:

```python
import numpy as np
import oxpy

with oxpy.Context():
    # init the manager with the given input file
    manager = oxpy.OxpyManager("input")
    manager.load_options()
    manager.init()

    # run 1k steps
    manager.run(1000)

    # run 10k steps more
    manager.run(10000)

    # do some computation with the current configuration
    particles = manager.config_info().particles()
    
    # compute the average position of the particles' backbones
    avg_pos = np.average(list(map(lambda p: p.backbone_site(), particles)), axis=0)
    print("Average final position:", avg_pos)
    
    # and the interaction energy between the first two particles
    print("Interaction energy between particle 0 and particle 1:", manager.config_info().interaction.pair_interaction(particles[0], particles[1]))
```

If you want, you can initialise the input file yourself and change some of the options before initialising the manager:

```python
	my_input = oxpy.InputFile()
	my_input.init_from_filename("input")
	my_input["backend"] = "CUDA"
	my_input["steps"] = "1e9"
	manager = oxpy.OxpyManager(my_input)
```

Finally, you can also generate a default input file and then update or add custom options before initialising the manager:

```python
	my_input = oxpy.utils.generate_default_input()
	my_input["steps"] = "1e9"
	my_input["log_file"] = "log.dat" 
	manager = oxpy.OxpyManager(my_input)
```

## Output files

The energy.dat (default name, can be changed in the configuration file) has this layout for MD:
time potential_energy kinetic_energy total_energy

The energy.dat (default name, can be changed in the configuration file) has this layout for MC:
time potential_energy hydrogen_bonding_energy acc_trasl acc_rot [acc_volume]

Mind that potential, kinetic and total energies are divided by the number of particles.

Configurations are saved in the trajectory file (appended one after the other).

## Observables

The observable infrastructure was devised to help building customized output from `oxDNA` (and `DNAnalysis`) without having to dive in the simulation code itself. 

The relevant keys in the input file are analysis_data_output_* and data_output_* (see below). These take as an argument, between curly brackets, a series of lines that is interpreted as an input file.
An example is:

```
data_output_1 = {
name = caca.dat
print_every = 100
col_1 = {
type = potential_energy
}
col_2 = {
type = step
units = MD
}
}
```

this will print in caca.dat two colums, the first with the potential energy and the second with the steps in MD units (`dt`-aware). 

The lines in between curly brackets are interpreted as input files by the single observables.

See also the doxygen documentation for `Observables/ObservableOutput.h`

oxDNA provides a plugin infrastructure to manage additional 
Observables. See the doxygen documentation for `PluginManagement/PluginManager.h`
