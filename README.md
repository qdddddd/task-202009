# task-202009

This project corresponds to this [task](https://github.com/terryjccg/task-202009).

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

The `./bin/` directory contains the compiled executable, and the `./outputs/` directory contains results and program logs containing detailed timestamps: 

- `parse_only.log` is the log for using 10 threads only parsing the files (already downloaded before), with a total execution time around 2.15 minutes;
- `download_parse.log` is the log for using 30 threads to download the files and parse them, with a total execution time around 4.05 minutes (around 20MB/s downloading speed).
