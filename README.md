# fty-common-rest

This is a library providing :
* Common methods and functions for REST API, or which require use of tntnet

## How to build

To build, run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=usr -DBUILD_TESTING=On ..
make
sudo make install
```

## List of Available Headers

### main header
* fty\_common\_rest.h

### secondary headers
* fty\_common\_rest\_audit\_log.h
* fty\_common\_rest\_helpers.h
* fty\_common\_rest\_sasl.h
* fty\_common\_rest\_utils\_web.h
* fty\_common\_rest\_tokens.h

