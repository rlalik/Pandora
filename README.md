This library allows to organize and manage Root objects like histograms and canvases in easy way. Alows for writing and reading them from file with minimum amount of work (see examples).

# Installation
To use the library you need installed ROOT (https://root.cern/install/) and exported `$ROOTSYS` variable.
1. Easy way to install library:
```bash
git clone https://github.com/dziadu/Pandora
mkdir Pandora/build
cd Pandora/build
cmake ..
sudo make install
```
1. Testing library:
```bash
make test
```
1. To install in custom location, replace two last steps with:
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=my_custom_location
make install
```
1. To run eamples, stay in the build directory and execute
```bash
./example_write
./example_read
```

# Features

1. Placeholders
Pandora allows for using placeholders in the histogram names:
 - @@@d -- this one will be used for directory name
 - @@@a -- this one will be used for collection name

By default, `@@@d` = `@@@a` and will be equal to collection name given in the constructor. Both can be changes using `rename(std::string)` and `chdir(std::string)` functions respectively. Placeholders are placed with real values at the wiritng to file time.
