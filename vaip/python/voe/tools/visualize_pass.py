##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
import copy
import json

import graphviz


class FakeNodeInput(object):
    def __init__(self, node):
        self._node = copy.deepcopy(node)
        self.node_arg = self.node
        self.as_node_input = self.node
        self.create_const = self.const_data

    def attr(self, a):
        return self

    def const_data(self, a, b):
        return [1]

    def shape(self):
        return [1, 1, 1, 1, 1]

    def node(self):
        return self

    def id(self):
        return self._node["id"]

    def __str__(self) -> str:
        return str(self._node)


class VisualizeFakeGraph(object):
    def __init__(self):
        self._node = {"call_node": {}}
        self.clone_inputs = self.clone_input_args
        self.clone_data_type = self.set_anchor_point1
        self.set_data_type = self.set_anchor_point1
        self.clone_shape = self.set_anchor_point1
        self.clone_attrs = self.set_anchor_point1
        self.set_attr = self.set_anchor_point2
        self.nodes = []
        self.id = 0

    def builder(self, vaip_pass):
        return self

    def set_op_type(self, type, domain):
        self._node["call_node"]["op_type"] = domain + ":" + type

    def clone_op_type(self, node):
        self._node["call_node"]["op_type"] = node._node["call_node"]["op_type"]

    def clone_input_args(self, node):
        self._node["call_node"]["args"] = node._node["call_node"]["args"]
        self._node["call_node"]["optional_args"] = node._node["call_node"][
            "optional_args"
        ]

    def set_input_args(self, node_args):
        self._node["call_node"]["args"] = [{"name": n.id()} for n in node_args]

    def build(self):
        op_type = self._node["call_node"]["op_type"]
        if ":" in op_type:
            suffix = op_type[op_type.index(":") + 1 :]
        else:
            suffix = op_type

        self._node["id"] = "auto_id_" + suffix + "_" + str(self.id)

        self.id += 1
        self.nodes.append(copy.deepcopy(self._node))
        return FakeNodeInput(self._node)

    def build_with_node(self, node):
        self.nodes.append(copy.deepcopy(node))
        return FakeNodeInput(node)

    def set_anchor_point1(self, a):
        pass

    def set_anchor_point2(self, a, b):
        pass

    def set_anchor_point3(self, a, b, c):
        pass


class PatternNode(object):
    def __init__(self, patterns, pattern):
        self._nodes = patterns
        self._node = pattern
        id = self._node.get("id", "auto_id_" + str(len(self._nodes)))
        self._node["id"] = id
        if not id in self._nodes:
            self._nodes[id] = self._node

    def children(self):
        ret = []
        if "call_node" in self._node:
            if "optional_args" in self._node["call_node"]:
                all_args = (
                    self._node["call_node"]["args"]
                    + self._node["call_node"]["optional_args"]
                )
            else:
                all_args = self._node["call_node"]["args"]
            for arg in all_args:
                if "name" in arg:
                    ret.append(PatternNode(self._nodes, self._nodes[arg["name"]]))
                elif "pattern" in arg:
                    ret.append(PatternNode(self._nodes, arg["pattern"]))
                else:
                    raise "never goes here"
        return ret

    def __repr__(self):
        return str(self._node)

    def id(self):
        return str(self._node["id"])

    def label(self):
        node = self._node
        has_id = not node["id"].startswith("auto_id_")
        var = node["id"] + "=\n" if has_id else ""
        if "call_node" in node:
            label = var + node["call_node"]["op_type"]
        elif "wildcard" in node:
            label = var + "*"
        elif "graph_input" in node:
            label = var + "<o>"
        else:
            label = json.dumps(node)
        return label


class PatternNodeInput(PatternNode):
    def id(self):
        return str(self._node["id"]) + "1"

    def children(self):
        ret = []
        if "call_node" in self._node:
            if "optional_args" in self._node["call_node"]:
                all_args = (
                    self._node["call_node"]["args"]
                    + self._node["call_node"]["optional_args"]
                )
            else:
                all_args = self._node["call_node"]["args"]
            for arg in all_args:
                if "name" in arg:
                    ret.append(PatternNodeInput(self._nodes, self._nodes[arg["name"]]))
                elif "pattern" in arg:
                    ret.append(PatternNodeInput(self._nodes, arg["pattern"]))
                else:
                    raise "never goes here"
        return ret

    def label(self):
        node = self._node
        has_id = not node["id"].startswith("auto_id_")
        if has_id:
            return node["id"]
        elif "call_node" in node:
            return node["call_node"]["op_type"]
        elif "wildcard" in node:
            return "*"
        elif "graph_input" in node:
            return "<o>"
        return json.dumps(node)


def _process_node(g, node, visited):
    id = node.id()

    if id in visited:
        return

    for child in node.children():
        _process_node(g, child, visited)
    label = node.label()
    g.node(id, label=label)
    visited[id] = node._node

    for child in node.children():
        g.edge(child.id(), node.id())


def show(pattern_def):
    g = graphviz.Digraph(comment="The Pattern")
    visited = {}
    all_patterns = {node["id"]: node for node in pattern_def["patterns"]}
    root_pattern = PatternNode(all_patterns, pattern_def["patterns"][-1])
    for pattern in pattern_def["patterns"]:
        if "is_root" in pattern:
            root_pattern = PatternNode(all_patterns, pattern)
    with g.subgraph(name="cluster_0", node_attr={"shape": "box"}) as c:
        c.attr(color="blue")
        c.graph_attr["label"] = "pattern"
        c.graph_attr["labelloc"] = "t"
        c.graph_attr["labeljust"] = "l"
        c.node_attr["style"] = "filled"
        _process_node(c, root_pattern, visited)
    return (all_patterns, g)


def show_action(g, nodes, pattern_nodes):
    visited = {}
    all_patterns = {node["id"]: node for node in nodes}
    root_pattern = PatternNodeInput(all_patterns, nodes[-1])
    with g.subgraph(name="cluster_1", node_attr={"shape": "box"}) as c:
        c.attr(color="yellow")
        c.graph_attr["label"] = "action"
        c.graph_attr["labelloc"] = "t"
        c.graph_attr["labeljust"] = "l"
        c.node_attr["style"] = "filled"
        _process_node(c, root_pattern, visited)

    for action_id, node in visited.items():
        if node["id"] in pattern_nodes:
            g.edge(
                node["id"],
                action_id,
                label="identity",
                constraint="false",
                style="dotted",
            )
