import json
import subprocess
from collections import defaultdict

import matplotlib.pyplot as plt
import os

TEST_DATA_FILE = "test_data"
BINARY_FILE = "../../cmake-build-debug/test/kv_benchmark"


def execute_benchmark():
    pwd = os.getcwd()
    cmd = os.path.join(pwd, BINARY_FILE)
    args = "--benchmark_format=json"
    completed_process = subprocess.run([cmd, args], capture_output=True)
    return completed_process.stdout


def execute_test_mock():
    path = os.path.join(os.getcwd(), "../../cmake-build-debug/test/result")
    with open(path, 'r') as result_file:
        data = result_file.read()
        return data.encode('utf8')


def parse_benchmark_result(data):
    json_data = json.loads(data.decode('utf8'))
    bench_result = json_data['benchmarks']
    plot_data = defaultdict(list)

    for result in bench_result:
        name = result['name']
        group_name = name[:name.find('/')]
        case_name = name[name.find('/') + 1:]
        time = result['cpu_time']
        plot_data[group_name].append((case_name, time))

    return plot_data


def add_labels(line):
    x, y = line.get_data()
    labels = map(','.join, zip(map(lambda s: '%g' % s, x), map(lambda s: '%g' % s, y)))
    map(plt.text, x, y, labels)


def annotate(x, y):
    for i in range(len(x)):
        plt.annotate(str(round(y[i], 4)), (x[i], y[i]), xycoords='data',
                     xytext=(-20, 20), textcoords='offset points', color="r", fontsize=12,
                     arrowprops=dict(arrowstyle="->", color='black'))


def visualize(plot_data, show_data_val):
    plt.figure(figsize=(12, 9), dpi=80)

    for name, val in plot_data.items():
        x, y = list(zip(*val))
        plt.plot(x, y, marker='o', label=name)
        if show_data_val:
            annotate(x, y)
    plt.ylabel('Millisecond')
    plt.xlabel('Batch size')
    plt.legend()
    plt.title("insert")
    plt.show()


def build_executable():
    cmd_str = "/Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake --build " \
              "/Users/bachdao/dev/kv_hash/cmake-build-debug --target kv_benchmark -j 8"
    cmd = cmd_str.split(" ")
    subprocess.run(cmd)


def main():
    build_executable()
    test_data = execute_benchmark()
    result = parse_benchmark_result(test_data)
    visualize(result, False)


main()
