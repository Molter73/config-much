#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "config-much.h"
#include "config.pb.h"

void usage(const char* prog) {
    std::cout << prog << " <CONFIG_DIR>" << std::endl << std::endl;
    std::cout << "<CONFIG_DIR>" << std::endl << "\tDirectory holding the configuration files to read." << std::endl;
}

int main(int argc, const char* argv[]) {
    using namespace google::protobuf;

    if (argc != 2) {
        std::cerr << "Missing required argument" << std::endl;
        usage(argv[0]);
        return -1;
    }

    std::filesystem::path base{argv[1]};

    test_config::Config protoConfig;
    ConfigMuch::Builder{}
        .add_file(base / "config.yml")
        .add_file(base / "second.yml")
        .set_env_var_prefix("MY_APP")
        .build()
        .parse(&protoConfig);

    std::cout << protoConfig.DebugString() << std::endl;

    return 0;
}
