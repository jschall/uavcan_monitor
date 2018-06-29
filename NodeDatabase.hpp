#pragma once
#include <string>
#include "json.hpp"

class NodeConfig;
class ExternalNode;

class NodeConfig {
public:
    NodeConfig(const nlohmann::json config) :
    _config(config) {}

    enum NodeMatch {
        NO_MATCH,
        PARTIAL_MATCH,
        FULL_MATCH
    };

    enum NodeMatch match(ExternalNode extnode);
private:
    nlohmann::json _config;
};

class ExternalNode {
public:
private:
};

class NodeDatabase {
public:
    void addNodeConfig(NodeConfig node_config) {
        _node_configs.push_back(node_config);
    }
private:
    std::vector<NodeConfig> _node_configs;
};
