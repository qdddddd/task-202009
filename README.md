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
