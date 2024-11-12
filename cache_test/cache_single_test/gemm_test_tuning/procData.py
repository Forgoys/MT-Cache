mport os
import matplotlib.pyplot as plt
from matplotlib import rcParams

# 设置字体支持中文字符
rcParams['font.family'] = 'SimHei'  # 或者 'Microsoft YaHei'
rcParams['axes.unicode_minus'] = False  # 用于显示负号

# 解析数据函数
def parse_data(filename):
    data = {10: set(), 11: set(), 12: set(), 13: set()}  # 使用集合以避免重复
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(', ')
            if len(parts) == 3:
                set_sets_value = int(parts[0].split(' ')[-1])  # 提取 'sets' 值
                set_lines_value = int(parts[1].split(' ')[-1])  # 提取 'lines' 值
                speed_up = float(parts[2].split(': ')[-1])      # 提取 'Speed Up' 值
                set_sum = set_sets_value + set_lines_value
                # 仅保留 set+lines = 11, 12, 13, 14 的数据
                if set_sum in data:
                    data[set_sum].add((set_sets_value, speed_up))  # 添加点以确保唯一性

    # 将集合转换为列表并按第一个元素排序
    for key in data:
        data[key] = sorted(data[key], key=lambda x: x[0])  # 按 set_lines_value（第一个元素）排序
    return data

# 绘制图表函数
def plot_data(data, output_path, title):
    plt.figure(figsize=(10, 6))
    markers = ['o', 's', '^', 'D']  # 每条线的标记
    colors = ['black', 'green', 'red', 'blue']  # 每条线的颜色
    labels = {10: "sets+lines=10", 11: "sets+lines=11", 12: "sets+lines=12", 13: "sets+lines=13"}

    for i, (set_sets_value, points) in enumerate(data.items()):
        if points:  # 检查是否有任何点
            sets, speed_ups = zip(*points)
            plt.plot(sets, speed_ups, marker=markers[i], color=colors[i], label=labels[set_sets_value], linewidth=2)

    # 添加标签和标题
    plt.xlabel("缓存行数量", fontsize=12)
    plt.ylabel("加速比", fontsize=12)
    plt.title("fft算子测试结果", fontsize=14)
    plt.legend(loc="upper right")
    plt.grid(True)

    # 保存图表到指定路径
    plt.savefig(output_path)
    plt.close()

# 主函数：处理所有符合模式的txt文件
def process_all_files(data_dir):
    output_dir = os.path.join(data_dir, "plots")
    os.makedirs(output_dir, exist_ok=True)

    for filename in os.listdir(data_dir):
        if filename.endswith(".txt") and "pro" not in filename:  # 符合命名条件
            file_path = os.path.join(data_dir, filename)
            data = parse_data(file_path)

            # 设置输出路径和图片标题
            title = f"{filename.split('.')[0]} 测试结果"
            output_subdir = os.path.join(output_dir, filename.split('.')[0])
            os.makedirs(output_subdir, exist_ok=True)
            output_path = os.path.join(output_subdir, f"{filename.split('.')[0]}.png")

            # 生成图表
            plot_data(data, output_path, title)
            print(f"生成图表并保存至: {output_path}")

# 指定数据目录路径
data_dir = r"C:\Users\dst\Desktop\fft"
process_all_files(data_dir)

