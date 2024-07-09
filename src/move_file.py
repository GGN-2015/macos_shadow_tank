import os
from PIL import Image
dirnow = os.path.dirname(os.path.abspath(__file__))
os.chdir(dirnow)

if __name__ == "__main__":
    Image.open("raw1.jpg").save("raw1.png")
    Image.open("raw2.jpg").save("raw2.png")