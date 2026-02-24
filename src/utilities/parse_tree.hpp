#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>

enum class NodeType{CONDITION, ACTION };

enum AIAction { IDLE, PURSUE, DISTRACTED };

struct TreeNode {
    int id;
    NodeType type;
    std::string condition_name; // for Condition nodes
    int true_child = -1;        // NodeID
    int false_child = -1;       // NodeID
    AIAction action;            // for Action nodes
};

struct ScriptedTree {
    int root_id = -1;
    std::unordered_map<int, TreeNode> nodes;
};

ScriptedTree load_tree(const std::string &filename);