import os
import subprocess
from PIL import Image

# 当前项目目录
dirnow = os.path.dirname(os.path.abspath(__file__))

# 将两个大小一致的图片合并为一个合并图
# img_path1 将成为 iOS 端看到的图像
# img_path2 将成为其他端看到的图像
# 最终合并得到的图像将存储在 aim_path 指定的目录
def combine_picutre(img_path1: str, img_path2: str, aim_path: str):
    Image.open(img_path1).save(os.path.join(dirnow, "src", "raw1.png"))
    Image.open(img_path2).save(os.path.join(dirnow, "src", "raw2.png"))
    old_cwd = os.getcwd()
    os.chdir(os.path.join(dirnow, "src"))
    _ = subprocess.call(['make', 'all'])              # 编译相关 .out 文件
    _ = subprocess.call(['bash', 'png_pipe_line.sh']) # 制作 PNG 文件
    os.chdir(old_cwd)                                 # 恢复原先的工作目录
    Image.open(os.path.join(dirnow, "src", "fin.png")).save(aim_path)

# 将 sample 目录下的 ios.png 和 android.png 合并
# 并存储到 sample 目录下的 merged.png 中
def main():
    combine_picutre(
        os.path.join(dirnow, "sample", "ios.png"),
        os.path.join(dirnow, "sample", "android.png"),
        os.path.join(dirnow, "sample", "merged.png"),
    )

if __name__ == "__main__":
    main()