// 读入 img_size.txt 和 idat1_new.bin 以及 idat2_new.bin
// 生成最终 PNG 图像

// 其实我需要的只是 load_data_from_file 和 dump_data_to_file
#include <cstring>
#include "get_rel_blk5.h"
using namespace std;

// 计算 crc32
uint32_t calc_crc32(const BinData& data, int begin, uint32_t len) {
    uint8_t* p = new uint8_t[len];
    for(int i = 0; i < len; i += 1) {
        p[i] = data[begin + i];
    }
    auto ans = crc32(0, p, len);
    delete[] p;
    return ans;
}

// 把 PNG 签名写入字符流
void set_signature(BinData& data) {
    for(uint8_t v: {137, 80, 78, 71, 13, 10, 26, 10}) {
        data.push_back(v);
    }
}

// 写入长度信息
void push_msb_integer(BinData& data, uint32_t v) { // MSB
    for(int i = 3; i >= 0; i -= 1) {
        data.push_back((v >> (8 * i)) & 0xff);
    }
}

// 写入一个长度为 4 的字符串
void push_string(BinData& data, const char* str) {
    assert(strlen(str) == 4);
    for(int i = 0; str[i]; i += 1) {
        data.push_back(str[i]);
    }
}

// 写入 IHDR chunk
void set_ihdr(BinData& data, int width, int height) { // 注意要使用大端模式存入数据
    push_msb_integer(data, 13); // 写入长度信息
    int begin = data.size();
    push_string(data, "IHDR");  // 写入块类型

    // 数据部分
    push_msb_integer(data, width);
    push_msb_integer(data, height);
    data.push_back(8); // bit depth
    data.push_back(2); // color type
    data.push_back(0); // zip method
    data.push_back(0); // filter method
    data.push_back(0); // interlace method

    // 校验和 4 bytes
    push_msb_integer(data, calc_crc32(data, begin, 13 + 4));
}

// 插入苹果特色的 iDOT 段
void set_idot(BinData& data, int width, int height) {
    push_msb_integer(data, 28); // 长度信息
    int begin = data.size();
    push_string(data, "iDOT");  // 写入块类型

    // 数据部分
    push_msb_integer(data, 2); // 两段并行
    push_msb_integer(data, 0);
    push_msb_integer(data, 1);
    push_msb_integer(data, 40);
    push_msb_integer(data, 1);        // part I height
    push_msb_integer(data, height-1); // part II height
    int idot_size = 4 + 4 + 28 + 4;
    int idat_size = 4 + 4 + (7 + (3 * width + 1) + 5) + 4;
    push_msb_integer(data, idot_size + idat_size); // 第二个 IDAT 的偏移量

    // 校验和 4 bytes
    push_msb_integer(data, calc_crc32(data, begin, 28 + 4));
}

// 填写 IDAT 段
void set_idat(BinData& data, int width, int height, const BinData& idat_new) {
    push_msb_integer(data, idat_new.size()); // 长度信息
    int begin = data.size();
    push_string(data, "IDAT");  // 写入块类型
    for(auto v: idat_new) {     // 写入数据部分
        data.push_back(v);
    }
    // 校验和 4 bytes
    push_msb_integer(data, calc_crc32(data, begin, 4 + idat_new.size()));
}

// 填写 IEND 段, 12 bytes
void set_iend(BinData& data) {
    push_msb_integer(data, 0); // 长度信息
    int begin = data.size();
    push_string(data, "IEND"); // 写入块类型
    auto checksum = calc_crc32(data, begin, 4);
    //printf("     : checksum: %08x\n", checksum);
    assert(checksum == 0xae426082);
    push_msb_integer(data, 0xae426082); // 校验和
}

// 生成一个新的 PNG 到指定文件
void gen_new_png(const char* filename, int width, int height, const BinData& idat1_new, const BinData& idat2_new) {
    BinData data; // 存储 PNG 文件中的相关信息
    set_signature(data);
    set_ihdr(data, width, height);
    set_idot(data, width, height); 
    set_idat(data, width, height, idat1_new); // 填写 IDAT 段
    set_idat(data, width, height, idat2_new); 
    set_iend(data);
    dump_data_to_file(data, filename); // 存储回 png 文件
}

int main() {
    BinData idat1_new; load_data_from_file(idat1_new, "idat1_new.bin");
    BinData idat2_new; load_data_from_file(idat2_new, "idat2_new.bin");
    int width, height; get_width_height(&width, &height); height += 1; // 多了一个黑行
    gen_new_png("fin.png", width, height, idat1_new, idat2_new);
    return 0;
}