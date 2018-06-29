#include "json.hpp"
#include <iostream>
#include <fstream>
#include "NodeDatabase.hpp"
#include <uavcan/uavcan.hpp>
#include <uavcan/protocol/node_info_retriever.hpp>
#include <uavcan_linux/uavcan_linux.hpp>
#include <thread>

using namespace std;
using json = nlohmann::json;

class NodeInfoCollector final : public uavcan::INodeInfoListener {
public:
    NodeInfoCollector(uavcan::INode& node) :
    _retriever(node) {
        const int retriever_res = _retriever.start();
        if (retriever_res < 0) {
            throw std::runtime_error("Failed to start the retriever; error: " + std::to_string(retriever_res));
        }

        const int add_listener_res = _retriever.addListener(this);
        if (add_listener_res < 0) {
            throw std::runtime_error("Failed to add listener; error: " + std::to_string(add_listener_res));
        }
    }

private:
    void handleNodeInfoRetrieved(uavcan::NodeID node_id, const uavcan::protocol::GetNodeInfo::Response& node_info) override {
        cout << "node info retrieved " << node_info << endl;
    }

    void handleNodeStatusChange(const uavcan::NodeStatusMonitor::NodeStatusChangeEvent& event) override {
        cout << "node status changed" << endl;
    }

    void handleNodeInfoUnavailable(uavcan::NodeID node_id) override
    {
        cout << "node info unavailable " << node_id.get() << endl;
    }

    uavcan::NodeInfoRetriever _retriever;
};

// void run_node(vector<string> busses) {
//     uavcan_linux::SystemClock clock;
//     uavcan_linux::SocketCanDriver driver(clock);
//
//     for (vector<string>::iterator it = busses.begin(); it != busses.end(); it++) {
//         if (driver.addIface(*it) < 0) {
//             throw std::runtime_error("Failed to add iface");
//         }
//     }
//
//     uavcan::Node<16384> node(driver, clock);
//     node.setNodeID(34);
//     node.setName("com.matternet.bus_monitor");
//
//     const int node_start_res = node.start();
//     if (node_start_res < 0) {
//         throw std::runtime_error("Failed to start the node; error: " + std::to_string(node_start_res));
//     }
//
//     node.setModeOperational();
//
//     NodeInfoCollector collector(node);
//
//     while (true) {
//         const int res = node.spin(uavcan::MonotonicDuration::fromUSec(200));
//         if (res < 0) {
//             std::cerr << "Transient failure: " << res << std::endl;
//         }
//     }
// }
//
// void main_loop(void) {
//
//
//     for()
//     uavcan_linux::SocketCanDriver driver(clock);
// }
//
// int main(int argc, const char **argv) {
//     run_logical_bus_monitor({"slcan0"});
//     return 0;
// }

static uavcan_linux::SystemClock uavcan_system_clock;

class MonitorNodeWrapper {
public:
    MonitorNodeWrapper(string bus_name, NodeDatabase& db, uint8_t node_id) :
    _db(db),
    _bus_name(bus_name) {
        uavcan_linux::SocketCanDriver* driver = new uavcan_linux::SocketCanDriver(uavcan_system_clock);

        {
            const int res = driver->addIface(_bus_name);
            if (res < 0) {
                throw std::runtime_error("Failed to add iface; error: " + std::to_string(res));
            }
        }

        _node = new uavcan::Node<16384>(*driver, uavcan_system_clock);
        _node->setNodeID(node_id);
        _node->setName("com.matternet.bus_monitor");

        {
            const int res = _node->start();
            if (res < 0) {
                throw std::runtime_error("Failed to start the node; error: " + std::to_string(res));
            }
        }


    }

    void spin(void) {
        const int res = _node->spin(uavcan::MonotonicDuration::fromUSec(200));
        if (res < 0) {
            std::cerr << "Transient failure: " << res << std::endl;
        }
    }

private:
    uavcan::Node<16384>* _node;
    string _bus_name;
    NodeDatabase& _db;
};

static vector<MonitorNodeWrapper*> monitor_node_list;

static void spin_local_nodes(void) {
    for (vector<MonitorNodeWrapper*>::iterator it = monitor_node_list.begin(); it != monitor_node_list.end(); it++) {
        (*it)->spin();
    }
}



int main(int argc, const char **argv) {
    if (argc != 2) {
        cerr << "usage: " << argv[0] << " <config_file>" << endl;
        return 1;
    }

    ifstream config_stream(argv[1]);

    json config;
    config_stream >> config;

    NodeDatabase db;

    uint8_t node_id = 34;
    for (json::iterator it = config["buses"].begin(); it != config["buses"].end(); it++) {
        monitor_node_list.push_back(new MonitorNodeWrapper(it->get<string>(), db, node_id++));
    }

    while(true) {
        spin_local_nodes();
    }

    return 0;
}

