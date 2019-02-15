# Measurement Kit nlohmann/json wrappers library

[![GitHub license](https://img.shields.io/github/license/measurement-kit/mkjson.svg)](https://raw.githubusercontent.com/measurement-kit/mkjson/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/mkjson.svg)](https://github.com/measurement-kit/mkjson/releases) [![Build Status](https://img.shields.io/travis/measurement-kit/mkjson/master.svg?label=travis)](https://travis-ci.org/measurement-kit/mkjson) [![codecov](https://codecov.io/gh/measurement-kit/mkjson/branch/master/graph/badge.svg)](https://codecov.io/gh/measurement-kit/mkjson) [![Build status](https://img.shields.io/appveyor/ci/bassosimone/mkjson/master.svg?label=appveyor)](https://ci.appveyor.com/project/bassosimone/mkjson/branch/master)

This header-only library contains Measurement Kit nlohmann/json wrappers. Its
main use case is vendoring of a specific tag inside Measurement Kit. Because
this is just a basic building block, we do not provide any stable API guarantee
for this library. For this reason, we'll never release `v1.0.0`.

## Regenerating build files

Possibly edit `MKBuild.yaml`, then run:

```
go get -v github.com/measurement-kit/mkbuild
mkbuild
```

## Building

```
mkdir build
cd build
cmake -GNinja ..
cmake --build .
ctest -a -j8 --output-on-failure
```

## Testing with docker

```
./docker.sh <build-type>
```
