import os
from PIL import Image, ImageDraw, ImageFont
dirnow = os.path.dirname(os.path.abspath(__file__))
os.chdir(dirnow)

# 思源黑体
TTF_FILE = os.path.join(dirnow, "../bg_photo/SourceHanSansSC-VF.ttf")

# 获得文本的宽度
def get_text_width_height(textsize, text):
    font = ImageFont.truetype(TTF_FILE, size=textsize)
    text_width = font.getlength(text)
    return text_width, textsize

def gen_png(text, save_path):
    # 创建一个 300x300 像素的白色图片
    img = Image.new("RGB", (300, 300), (255, 255, 255))

    # 创建一个用于绘制文字的Drawing对象
    draw = ImageDraw.Draw(img)

    # 设置文字样式
    font = ImageFont.truetype(TTF_FILE, size=30)

    # 获取文字的尺寸
    text_width, text_height = get_text_width_height(30, text)
    text_height = 30

    # 计算文字的起始坐标,使其居中
    x = (img.width - text_width) / 2
    y = (img.height - text_height) / 2

    # 在图片上绘制文字
    draw.text((x, y), text, font=font, fill=(0, 0, 0))

    # 保存图片
    img.save(save_path)

if __name__ == "__main__":
    gen_png("Hello, Apple!", "raw1.png")
    gen_png("Hello, Android!", "raw2.png")