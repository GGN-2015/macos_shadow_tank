from PIL import Image
import os
from tqdm import tqdm

# 读取 raw1 和 raw2 两张图
def get_raw_png():
    img1 = Image.open("raw1.png").convert("RGB")
    img2 = Image.open("raw2.png").convert("RGB")
    if img1.size != img2.size: # 两个图片尺寸必须一致
        img2 = img2.resize(img1.size)
        img2.save("raw2.png")
    width, height = img1.size
    assert img1.size == img2.size
    return width, height, img1, img2

# 将加了 filter-type 的数据写入二进制文件
def dump_data(img: Image, bin_file):
    fp = open(bin_file, "wb")
    width, height = img.size
    for line in tqdm(range(height)):
        lis = [0x00] # non-filter-type
        for pos in range(width):
            r, g, b = img.getpixel((pos, line))
            lis.append(r)
            lis.append(g)
            lis.append(b)
        fp.write(bytes(lis))
    fp.close()

if __name__ == "__main__":
    dirnow = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dirnow)
    width, height, img1, img2 = get_raw_png()
    dump_data(img1, "img1.raw.bin")
    dump_data(img2, "img2.raw.bin")
    open("img_size.txt", "w").write("%d %d" % (width, height))