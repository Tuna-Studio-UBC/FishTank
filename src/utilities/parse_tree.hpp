#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>

enum class NodeType{ CONDITION, ACTION, RANDOM };

enum AIAction { IDLE, PURSUE, DISTRACTED, PATROL };

struct TreeNode {
    int id;
    NodeType type;
    std::string condition_name;
    int true_child = -1;
    int false_child = -1;
    float probability = 0.f;
    AIAction action;
};

struct ScriptedTree {
    int root_id = -1;
    std::unordered_map<int, TreeNode> nodes;
};

ScriptedTree load_tree(const std::string &filename);