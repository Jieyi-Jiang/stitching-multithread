import yaml

# with open('config.yaml', 'r') as f:
#     data = yaml.safe_load(f)
# # print(data)
# camera1_intrinsics = data["camera2"]["intrinsics"]
# camera1_intrinsics["fx"] = 10000
# print(camera1_intrinsics["fx"])
# # data["camera2"]["intrinsics"] = camera1_intrinsics

# with open('config.yaml', 'w') as f:
#     yaml.dump(data, f, default_flow_style=False)

import yaml

class FlowStyleList(list):
    pass

def flow_style_list_representer(dumper, data):
    return dumper.represent_sequence('tag:yaml.org,2002:seq', data, flow_style=True)

yaml.add_representer(FlowStyleList, flow_style_list_representer)

data = {
    'name': 'camera1',
    'matrix': FlowStyleList([
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1]
    ]),
    'params': {
        'fx': 1000,
        'fy': 1000
    }
}

with open('output.yaml', 'w') as f:
    yaml.dump(data, f, default_flow_style=False)


# matrix = [
#     [1, 0, 0],
#     [0, 1, 0],
#     [0, 0, 1]
# ]

# with open('matrix.yaml', 'w') as f:
#     yaml.dump({'matrix': matrix}, f, default_flow_style=True)
