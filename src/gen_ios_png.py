import os
from PIL import Image

def load_image_from_raw(width, height, raw_file:str, dump_file:str):
    img = Image.new('RGB', (width, height), color=(0, 0, 0))
    n_width, n_height = img.size
    assert n_width == width and n_height == height
    content = open(raw_file, "rb").read()
    line_size = 1 + 3 * width
    assert len(content) == line_size * height
    pos = 0
    line_cnt = 0
    while pos < len(content):
        line_raw_data = content[pos: pos + line_size] # 第一个字符描述参照方式
        flag = line_raw_data[0]
        rest = line_raw_data[1:]
        assert flag in [0] # 我的程序只实现了 non-filter 模式
        for i in range(width):
            p = i * 3
            img.putpixel((i, line_cnt), tuple(rest[p: p+3]))
        pos += line_size
        line_cnt += 1
    img.save(dump_file)

if __name__ == "__main__":
    dirnow = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dirnow)
    width, height = open("size.txt", "r").read().split()
    load_image_from_raw(int(width), int(height), "idat.ios.bin", "ios.png")