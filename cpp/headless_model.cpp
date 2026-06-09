#include "basicsim/jaamsim_model.h"
#include "basicsim/simulation.h"
#include "events/event_manager.h"
#include "json/json_parser.h"
#include "json/json_writer.h"
#include "remote/zmq_bridge.h"

#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config-file>\n";
        return 1;
    }

    try {
        jaamsim::basicsim::JaamSimModel model;

        model.auto_load();

        std::ifstream config_file(argv[1]);
        if (!config_file) {
            std::cerr << "Cannot open config file: " << argv[1] << '\n';
            return 1;
        }
        std::stringstream buffer;
        buffer << config_file.rdbuf();
        std::string config_text = buffer.str();

        auto config = jaamsim::json::parse_json(config_text);
        model.configure(config);

        jaamsim::remote::ZmqBridge bridge;
        bridge.start(model);

        std::cout << "ZMQ bridge started on tcp://*:5556 (PUB) and tcp://*:5557 (ROUTER)\n";
        std::cout << "Running simulation...\n";

        auto run_result = model.start();

        std::cout << "Simulation ended. Sim time: " << run_result.sim_time << " s\n";

        bridge.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
