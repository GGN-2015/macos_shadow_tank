// 读取 img1.raw.bin 和 img2.raw.bin
// 生成 idat1_new.bin 和 idat2_new.bin
// 两个被生成的文件是未来融合图片的 IDAT Chunk 的 data 部分

#include <cassert>
#include <cstdio>
#include "get_rel_blk5.h"
using namespace std;

// #define STEP_EQUALS_TWO // 不允许跨多行

// 生成一个黑色行以及一个非压头 (含 78 da)
void gen_idat1_new(BinData& idat1_new_bin, int width, int height) {
    idat1_new_bin.clear();
    uint16_t LEN  = width * 3 + 1;
    uint16_t NLEN = ~LEN;

    // 插入 zlib 头
    idat1_new_bin.push_back(0x78);
    idat1_new_bin.push_back(0xda);

    // 插入一个非压头
    idat1_new_bin.push_back(0x00); // 3 个 0-bit 表示一个不压缩段
    idat1_new_bin.push_back(( LEN >> 0) & 0xff); // LSB first
    idat1_new_bin.push_back(( LEN >> 8) & 0xff);
    idat1_new_bin.push_back((NLEN >> 0) & 0xff); // LSB first
    idat1_new_bin.push_back((NLEN >> 8) & 0xff);

    for(int i = 0; i < LEN; i += 1) { // 插入若干个零
        idat1_new_bin.push_back(0x00);
    }

    // 再插入一个非压头
    idat1_new_bin.push_back(0x00); // 3 个 0-bit 表示一个不压缩段
    idat1_new_bin.push_back(( LEN >> 0) & 0xff); // LSB first
    idat1_new_bin.push_back(( LEN >> 8) & 0xff);
    idat1_new_bin.push_back((NLEN >> 0) & 0xff); // LSB first
    idat1_new_bin.push_back((NLEN >> 8) & 0xff);
}

// 生成半个数据对
void gen_data_part(BinData& data_pair, int line_id, int line_cnt, int width, int height,
    const BinData& img_raw_bin) {
    data_pair.clear();
    uint16_t LEN = 3 * width + 1;
    uint16_t NLEN = ~LEN;
    int res = LEN % 5; // 长度余数
    for(uint8_t v: {0x00, 0x00, 0x00, 0xff, 0xff}) { // 左 padding
        data_pair.push_back(v);
    }
    BinData line_raw_data; // 获取未压缩的行数据
    for(int j = 0; j < line_cnt; j += 1) {
        for(int i = 0; i < LEN; i += 1) {
            int p = (line_id + j) * LEN + i;
            line_raw_data.push_back(img_raw_bin[p]);
        }
    }
    // 获取并装入可靠块
    BinData comp; get_rel_blk5(line_raw_data, comp, res);
    for(uint8_t v: comp) {
        data_pair.push_back(v);
    }
    printf("     : comp_size: %d\n", (int)comp.size());
    assert(data_pair.size() < LEN);
    assert(data_pair.size() % 5 == res); // 余数校验
    while(data_pair.size() < LEN - 5) {
        for(uint8_t v: {0x00, 0x00, 0x00, 0xff, 0xff}) { // 右 padding
            data_pair.push_back(v);
        }
    }
    assert(data_pair.size() == LEN - 5);
    // 插入非压头
    data_pair.push_back(0x00); // 3 个 0-bit 表示一个不压缩段
    data_pair.push_back(( LEN >> 0) & 0xff); // LSB first
    data_pair.push_back(( LEN >> 8) & 0xff);
    data_pair.push_back((NLEN >> 0) & 0xff); // LSB first
    data_pair.push_back((NLEN >> 8) & 0xff);
    assert(data_pair.size() == LEN);
}

// 生成一个数据对
void gen_data_pair(BinData& data_pair, bool islast, int line_id, int step, int width, int height, 
    const BinData& img1_raw_bin, 
    const BinData& img2_raw_bin) {
    data_pair.clear();
    assert(step >= 2);
    printf("DEBUG: generating data pair line_id: %d\n", line_id);

    BinData data_pair1;
    gen_data_part(data_pair1, line_id + 0, step-1, width, height, img1_raw_bin);
    BinData data_pair2;
    gen_data_part(data_pair2, line_id + 1, step-1, width, height, img2_raw_bin);

    if(islast) { // 特殊处理最后一行
        for(int i = 1; i <= 5; i += 1) { // 删除最后一个非压头
            data_pair2.pop_back();
        }
        for(uint8_t v: {0x00, 0x00, 0x00, 0xff, 0xff}) { // 同步两端
            data_pair2.push_back(v);
        }
    }

    for(auto v: data_pair1) { // 追加两个数据部分
        data_pair.push_back(v);
    }
    for(auto v: data_pair2) {
        data_pair.push_back(v);
    }
}

// 追加黑色的行，理论上会被各种端解析
void add_black_line(BinData& idat2_new_bin, int width) {
    uint16_t LEN  = 3 * width + 1;
    uint16_t NLEN = ~LEN;
    idat2_new_bin.push_back(0x00);               // 不压缩块
    idat2_new_bin.push_back(( LEN >> 0) & 0xff); // LSB first
    idat2_new_bin.push_back(( LEN >> 8) & 0xff);
    idat2_new_bin.push_back((NLEN >> 0) & 0xff); // LSB first
    idat2_new_bin.push_back((NLEN >> 8) & 0xff);
    for(int i = 0; i < LEN; i += 1) {
        idat2_new_bin.push_back(0x00); // black pixels
    }
}

// 获取压缩后的体积
int get_zip_len(const BinData& img_raw_bin, int width, int begin_line_id, int line_cnt) {
    BinData tmp_raw;
    int LEN = 3 * width + 1;
    int res = LEN % 5;
    for(int j = 0; j < line_cnt; j += 1) { // 获取若干行数据，用于压缩
        for(int i = 0; i < LEN; i += 1) {
            int p = LEN * (begin_line_id + j) + i;
            tmp_raw.push_back(img_raw_bin[p]);
        }
    }
    BinData tmp_rel_com;
    get_rel_blk5(tmp_raw, tmp_rel_com, res);
    return tmp_rel_com.size(); // 返回压缩后的体积
}

// 分析最多能连续处理多少行
int get_max_step(int line_id, int width, int height, const BinData& img1_raw_bin, const BinData& img2_raw_bin) {
    int step = 2;
#ifndef STEP_EQUALS_TWO
    int LEN = 3 * width + 1;
    for(int i = 3; i <= height - line_id; i += 1) {
        int l_ziplen = get_zip_len(img1_raw_bin, width, line_id + 0, i - 1);
        int r_ziplen = get_zip_len(img2_raw_bin, width, line_id + 1, i - 1);
        if(l_ziplen + 10 <= LEN && r_ziplen + 10 <= LEN) { // 能容纳这些行
            step = i;
        }else {
            break;
        }
    }
#endif
    return step;
}

// 生成二号数据部分（不含 78 da）
void gen_idat2_new(BinData& idat2_new_bin, int width, int height, 
    const BinData& img1_raw_bin,
    const BinData& img2_raw_bin) {
    idat2_new_bin.clear();
    int res = 0;

    // raw1 的偶数行和 raw2 的奇数行凑到一起
    for(int line_id = 0; line_id < height; ) {
        int step = 2;
        if(line_id == height - 1) { // 只剩下一行的情况要特殊处理
            res = 1;
            break;
        }
        step = get_max_step(line_id, width, height, img1_raw_bin, img2_raw_bin); // 分析至多能压缩多少行

        bool islast = (line_id + step == height); // 最后一组特殊处理
        BinData data_pair;                     // 生成数据对
        gen_data_pair(data_pair, islast, line_id, step, width, height, img1_raw_bin, img2_raw_bin);
        for(uint8_t v: data_pair) {            // 数据对追加到 idat2_new_bin 尾部
            idat2_new_bin.push_back(v);
        }
        line_id += step;
    }

    if(res == 1) { // 只剩下一行，所以要特殊处理，先把最后五个字符改成 islast 风格的
        for(int i = 5; i >= 1; i -= 1) {
            idat2_new_bin[idat2_new_bin.size() - i] = 0x00;
        }
        for(int i = 2; i >= 1; i -= 1) {
            idat2_new_bin[idat2_new_bin.size() - i] = 0xff;
        }
        add_black_line(idat2_new_bin, width); // 然后再追加一个黑色的行
    }

    // 追加一个公共尾巴
    for(uint8_t v: {0x01, 0x00, 0x00, 0xff, 0xff}) { // 一个没有信息的结尾 block
        idat2_new_bin.push_back(v);
    }
    printf("     : idat2_new_bin.size(): %d\n", (int)idat2_new_bin.size());
}

// 在二号数据的末尾补充一个校验和，校验和是连续解压的校验和
void make_adler32(const BinData& idat1_new_bin, BinData& idat2_new_bin, int width, int height) {
    BinData link_data, rezip_data, raw_data;
    for(auto v: idat1_new_bin) {
        link_data.push_back(v);
    }
    for(auto v: idat2_new_bin) {
        link_data.push_back(v);
    }
    assert(unzip_data(link_data, raw_data) == false); // 因为缺尾巴
    unzip_data_unsafe(link_data, raw_data);
    int new_file_size = (height + 1) * (3 * width + 1); // 多一个黑行
    printf("     : raw_size: %d, file_size: %d\n", (int)raw_data.size(), (int)new_file_size);
    assert(raw_data.size() == new_file_size);
    zip_data(raw_data, rezip_data); // 根据解压得到的 raw_data 重新压缩，为了得到 ADLER32
    for(int i = 4; i >= 1; i -= 1) {
        link_data    .push_back(rezip_data[rezip_data.size() - i]);
        idat2_new_bin.push_back(rezip_data[rezip_data.size() - i]);
    }
    assert(unzip_data(link_data, raw_data)); // 重新尝试解压
}

// 生成 idat1_new.bin 和 idat2_new.bin
void process() {
    int width, height; get_width_height(&width, &height);
    printf("DEBUG: width: %d, height: %d\n", width, height);
    int raw_line_length = 3 * width + 1; // 计算单行的字节数
    printf("DEBUG: raw_line_length: %d\n", raw_line_length);
    int file_size = height * raw_line_length; // 文件大小
    BinData img1_raw_bin; // 加载两个二进制文件
    load_data_from_file(img1_raw_bin, "img1.raw.bin");
    BinData img2_raw_bin;
    load_data_from_file(img2_raw_bin, "img2.raw.bin");
    assert(img1_raw_bin.size() == file_size); // 检查文件大小是否正确
    assert(img2_raw_bin.size() == file_size);
    BinData idat1_new_bin;
    BinData idat2_new_bin;
    gen_idat1_new(idat1_new_bin, width, height);
    gen_idat2_new(idat2_new_bin, width, height, img1_raw_bin, img2_raw_bin);
    make_adler32(idat1_new_bin, idat2_new_bin, width, height);
    dump_data_to_file(idat1_new_bin, "idat1_new.bin");
    dump_data_to_file(idat2_new_bin, "idat2_new.bin");
}

int main() {
    process();
    return 0;
}