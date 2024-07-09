import os
import subprocess
import shutil
from PIL import Image

# 当前项目目录
dirnow = os.path.dirname(os.path.abspath(__file__))

# 给定一个合并图片，将其中 macos 视角能看到的效果拆分出来
def dump_ios_preview(merged_img_path:str, aim_path:str):
    shutil.copy(merged_img_path, os.path.join(dirnow, "src", "fin.png")) # 拷贝
    old_cwd = os.getcwd()
    os.chdir(os.path.join(dirnow, "src"))
    _ = subprocess.call(['make', 'all'])            # 编译相关 .out 文件
    _ = subprocess.call(['bash', 'gen_ios_png.sh']) # 制作 PNG 文件
    os.chdir(old_cwd)
    shutil.copy(os.path.join(dirnow, "src", "ios.png"), aim_path) # 拷贝

# 将 sample 目录下的 merged.png 使用考虑 iDOT 风格的解压程序
# 将解压后的部分存入 sample 目录下的 macos_slice.png
def main():
    dump_ios_preview(
        os.path.join(dirnow, "sample", "merged.png"),
        os.path.join(dirnow, "sample", "macos_slice.png"),
    )

if __name__ == "__main__":
    main()