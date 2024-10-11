╔═══╦═══╦═╗─╔╦═══╦══╦═══╗╔═╗╔═╦╗─╔╦═══╦╗─╔╗─╔═══╗
║╔═╗║╔═╗║║╚╗║║╔══╩╣╠╣╔═╗║║║╚╝║║║─║║╔═╗║║─║║─║╔═╗║
║║─╚╣║─║║╔╗╚╝║╚══╗║║║║─╚╝║╔╗╔╗║║─║║║─╚╣╚═╝║─╚╝╔╝║
║║─╔╣║─║║║╚╗║║╔══╝║║║║╔═╗║║║║║║║─║║║─╔╣╔═╗║───║╔╝
║╚═╝║╚═╝║║─║║║║──╔╣╠╣╚╩═║║║║║║║╚═╝║╚═╝║║─║╠╦╦╗╔╗
╚═══╩═══╩╝─╚═╩╝──╚══╩═══╝╚╝╚╝╚╩═══╩═══╩╝─╚╩╩╩╝╚╝

This is a small C++ library to try and standardize configuration of
applications through protobuf messages.

# How does it work?
You define your configuration as a [protobuf](https://protobuf.dev)
message, get it compiled. Then instantiate a `Parser` in your
application, configure what files it should use, a prefix for
environment variables and hand it a pointer to an instance of your
message for it to be auto-filled for you. See `example/` for a
minimalistic app example.

# Building
config-much is built using cmake:
```sh
cmake -B build
cmake --build build/
```

The previous commands will only work if the project dependencies are
pre-installed on your system, these are protobuf, yaml-cpp and gtest.
You can also build the project using [vcpkg](https://vcpkg.io), this
is the recommended way of building for now. You can use the following
commands for this:
```sh
cmake -B build --preset=default
cmake --build build/
```

If you are making changes to config-much, the devel preset will add
unit tests, an example and asan+ubsan to your build.
