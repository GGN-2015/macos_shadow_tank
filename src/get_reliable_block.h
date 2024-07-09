#pragma once

#include <algorithm>
#include "zip_utils.h"
using namespace std;

// 制作 “可靠块”
// 可靠块并不是完整的压缩数据
// 而是压缩数据中的一个或者两个 block，并且是整数字节对齐的
//! 假设 deflate 生成的结果中只含一个块，算法才正确
bool get_reliable_block(const BinData& raw, BinData& rel_com) {
    // assert(raw.size() < 32768);
    rel_com.clear();
    if(raw.size() == 0) {
        rel_com = {0x00, 0x00, 0x00, 0xff, 0xff};
        return false; // 00 00 00 ff ff 补充
    }
    BinData zip, unzip;
    zip_data(raw, zip);             // 先进行压缩
    assert(unzip_data(zip, unzip)); // 检查可解压性
    BinData adler32_data;           // 截取最后四个数据：checksum
    for(int i = 0; i < 4; i += 1) {
        adler32_data.push_back(zip[zip.size() - 1]);
        zip.pop_back();
    }
    reverse(adler32_data.begin(), adler32_data.end());
    assert(zip[0] == 0x78);        // 只接受这种压缩方式
                                   // zip[1] 我见过 DA 也见过 9C
    zip[2] &= (0xff - 1);          // 删除末尾块标志
    unzip_data_unsafe(zip, unzip); // 需要保证仍然可解压
    assert(unzip.size() == raw.size());
    for(int i = 0; i < unzip.size(); i += 1) {
        assert(unzip[i] == raw[i]);
    }
    // debug_output("zip", zip);
    // debug_output("adler32_data", adler32_data);
    // 假设 padding bit 有至少三个 0-bit
    // 首先考虑能不能通过补充 00 00 ff ff 01 00 00 ff ff 获得安全块
    BinData copy_zip1 = zip;
    for(uint8_t v: {0x00, 0x00, 0xff, 0xff, 0x01, 0x00, 0x00, 0xff, 0xff}) {
        copy_zip1.push_back(v);
    }
    for(auto v: adler32_data) { // 校验
        copy_zip1.push_back(v);
    }
    // debug_output("copy_zip1", copy_zip1);
    // 其次考虑能不能通过补充 00 00 00 ff ff 01 00 00 ff ff 获得安全块
    BinData copy_zip2 = zip;
    for(uint8_t v: {0x00, 0x00, 0x00, 0xff, 0xff, 0x01, 0x00, 0x00, 0xff, 0xff}) {
        copy_zip2.push_back(v);
    }
    for(auto v: adler32_data) { // 校验
        copy_zip2.push_back(v);
    }
    for(int i = 2; i < zip.size(); i += 1) { // 删除前两个字符 78 da
        rel_com.push_back(zip[i]);
    }
    // debug_output("copy_zip2", copy_zip2);
    if(unzip_data(copy_zip1, unzip)) { // 00 00 ff ff 是可行后缀
        // printf("DEBUG: at least 3 0-bit padding.\n");
        for(uint8_t v: {0x00, 0x00, 0xff, 0xff}) {
            rel_com.push_back(v);
        }
        return true; // 00 00 ff ff 补充
    }else if(unzip_data(copy_zip2, unzip)) { // 00 00 00 ff ff 是可行后缀
        // printf("DEBUG: at most 2 0-bit padding.\n");
        for(uint8_t v: {0x00, 0x00, 0x00, 0xff, 0xff}) {
            rel_com.push_back(v);
        }
        return false; // 00 00 00 ff ff 补充
    }else {
        printf("ERROR: can not get reliable block.\n");
        assert(false);
    }
}

// int main() {
//     BinData test {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
//     BinData rel_com;
//     while(true) {
//         test.push_back(rand() % 256);
//         bool flag = get_reliable_block(test, rel_com);
//         debug_output("rel_com", rel_com);
//         if(!flag) break;
//     }
//     return 0;
// }
