#pragma once

#include "get_reliable_block.h"

// 从 img_size.txt 读取 width 和 height
void get_width_height(int* p_width, int* p_height) {
    FILE* fpin = fopen("img_size.txt", "r");
    fscanf(fpin, "%d %d", p_width, p_height);
    fclose(fpin);
}

// 从文件加载二进制数据
void load_data_from_file(BinData& bin_data, const char* filename) {
    bin_data.clear();
    FILE* fpin = fopen(filename, "rb");
    assert(fpin != NULL);
    while(!feof(fpin)) {
        bin_data.push_back(fgetc(fpin));
    }
    bin_data.pop_back();
    fclose(fpin);
}

// 将二进制数据写回文件
void dump_data_to_file(const BinData& data, const char* filename) {
    FILE* fpout = fopen(filename, "wb");
    for(uint8_t v: data) { // 逐字节写入文件
        fputc(v, fpout);
    }
    fclose(fpout);
}

// 制作一个可靠块
// 要求这个可靠块的长度 mod 5 必须余数为 res
// res 的值取 (1 + 3 * width) mod 5
bool get_rel_blk5_raw(const BinData& raw, BinData& rel_com, int res = 0) {
    BinData pre_raw = raw; // 记录一个前缀
    while(pre_raw.size() > 0) {
        BinData pre_rc; get_reliable_block(pre_raw, pre_rc);
        // printf("DEBUG: pre_rc.size() + (raw.size() - pre_raw.size()) = %u\n", (uint32_t)(pre_rc.size() + (raw.size() - pre_raw.size())));
        if((pre_rc.size() + (raw.size() - pre_raw.size())) % 5 == res) { // OK
            rel_com = pre_rc;
            uint16_t len  = raw.size() - pre_raw.size();
            uint16_t nlen = ~len;
            if(len != 0) {
                rel_com.push_back(0);                  // 开一个非压缩段
                rel_com.push_back(( len >> 0) & 0xff); // LSB first
                rel_com.push_back(( len >> 8) & 0xff); 
                rel_com.push_back((nlen >> 0) & 0xff); // LSB first
                rel_com.push_back((nlen >> 8) & 0xff); 
                for(int i = raw.size() - len; i < raw.size(); i += 1) {
                    rel_com.push_back(raw[i]);
                } // 追加后若干个字节
            }
            return true; // 获取成功
        }
        pre_raw.pop_back();
    }
    return false; // 获取失败
}

// 可靠的获得可靠块的方法
void get_rel_blk5(const BinData& raw, BinData& rel_com, int res = 0) {
    res = ((res % 5) + 5) % 5;
    bool suc = get_rel_blk5_raw(raw, rel_com, res);
    if(!suc) {
        BinData rcom; get_reliable_block(raw, rcom); // 先得到可靠块
        while(rcom.size() % 5 != res) {
            for(uint8_t v: {0x02, 0x00, 0x00, 0x00, 0xff, 0xff}) {
                rcom.push_back(v);
            } // 长度为六的神奇 fix huffman 填充
        }
        rel_com = rcom; // 此时 size % 5 == res
    }
}

// 检查可靠块是否可靠
bool check_reliable(const BinData& raw, const BinData& rel_com) {
    BinData tmp_com;
    tmp_com.push_back(0x78);
    tmp_com.push_back(0xda); // 头部
    for(auto v: rel_com) {
        tmp_com.push_back(v); // 可靠块部分
    }
    for(auto v: {0x01, 0x00, 0x00, 0xff, 0xff}) {
        tmp_com.push_back(v); // 结尾部分
    }
    BinData tmp_raw;
    unzip_data_unsafe(tmp_com, tmp_raw);
    if(raw.size() != tmp_raw.size()) {
        return false; // 可靠块不正确
    }
    for(int i = 0; i < raw.size(); i += 1) {
        if(raw[i] != tmp_raw[i]) {
            return false; // 可靠块不正确
        }
    }
    return true; // 可靠块正确
}

// 将可靠块构建成一个合法 deflate 压缩序列
void make_reliable_avilable(const BinData& rel_com, BinData& real_com) {
    real_com = {0x78, 0xda}; // 头部
    for(auto v: rel_com) {   // 拷贝数据部分
        real_com.push_back(v);
    }
    for(auto v: {0x01, 0x00, 0x00, 0xff, 0xff}) { // 尾部
        real_com.push_back(v);
    }
    BinData raw;    unzip_data_unsafe(real_com, raw); // 先试图解压
    BinData re_com; zip_data(raw, re_com);            // 重新压缩，得到校验和
    BinData tmp_raw;
    assert(unzip_data(re_com, tmp_raw));
    for(int i = 4; i >= 1; i -= 1) {
        real_com.push_back(re_com[re_com.size() - i]);
    }
    assert(unzip_data(real_com, tmp_raw)); // 应该可以正确解压
}

// int main() {
//     BinData raw {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1};
//     BinData rel_com;
//     get_rel_blk5(raw, rel_com);
//     debug_output("rel_com", rel_com);
//     BinData real_com;
//     make_reliable_avilable(rel_com, real_com);
//     debug_output("real_com", real_com);
//     BinData fin_raw;
//     unzip_data_unsafe(real_com, fin_raw);
//     debug_output("fin_raw", fin_raw);
//     return 0;
// }