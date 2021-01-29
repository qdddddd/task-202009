# task-202009

This project corresponds to this [task](https://github.com/terryjccg/task-202009).

### Note:
This program is ONLY tested on Ubuntu 18.04.

### Dependencies:
```
sudo apt-get install libxml2-dev uuid-dev libboost-all-dev
```
These are ONLY used for compiling the [Azure Storage Client Library for C++](https://github.com/Azure/azure-storage-cpp) and its depend library [C++ REST SDK](https://github.com/microsoft/cpprestsdk). Both libraries are included in this project as submodules in the `third-party/` directory.


And of course you need to have one of `gcc` or `clang`.

### To compile:
```
$ mkdir build && cd build
$ cmake .. && make -j && cd ..
```

### To run:
```
$ ./bin/r2 <# of threads> <dir to store downloads>
```

For example:
```
$ ./bin/r2 10 logs
```

The program will then use 10 threads to parse the log files downloaded to `./logs/`, and write the calculated r2 values to `r2.csv`.

### Outputs:
The `./bin/` directory contains the compiled executable, and the `./outputs/` directory contains results and program logs containing detailed timestamps: 

- `parse_only.log` is the log for using 10 threads only parsing the files (already downloaded before), with a total execution time around 2.15 minutes;
- `download_parse.log` is the log for using 30 threads to download the files and parse them, with a total execution time around 4.05 minutes (around 20MB/s downloading speed).
