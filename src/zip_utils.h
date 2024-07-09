#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <zlib.h>
using namespace std;

const int MAX_BUF = 1048576 * 24;
typedef vector<uint8_t> BinData;

// 解压算法
bool unzip_data(const BinData& compressed, BinData& uncompressed) {
    uncompressed.clear();
    uint8_t* com_buf = new uint8_t[compressed.size()];
    for(int i = 0; i < compressed.size(); i += 1) {
        com_buf[i] = compressed[i]; // 构建被解压数据
    }
    static uint8_t uncom_buf[MAX_BUF]; // 解压缓冲区
    uLongf decompressed_size = MAX_BUF;
    int ret = uncompress((Bytef*)uncom_buf, &decompressed_size, (const Bytef*)com_buf, compressed.size());
    if (ret != Z_OK) {
        //fprintf(stderr, "DEBUG: Decompression error: %d\n", ret);
        return false; // 解压失败
    }else {
        //fprintf(stderr, "DEBUG: Decompression successfully.\n");
    }
    for(int i = 0; i < decompressed_size; i += 1) {
        uncompressed.push_back(uncom_buf[i]);
    }
    delete[] com_buf;
    return true; // 解压成功
}

// 压缩算法
bool zip_data(const BinData& raw, BinData& compressed) {
    compressed.clear();
    uint8_t* raw_buf = new uint8_t[raw.size()]; // 拷贝
    for(int i = 0; i < raw.size(); i += 1) {
        raw_buf[i] = raw[i];
    }
    static uint8_t com_buf[MAX_BUF];
    uLongf output_size = MAX_BUF;
    int ret = compress((Bytef*)com_buf, &output_size, (const Bytef*)raw_buf, raw.size());
    if (ret != Z_OK) {
        //fprintf(stderr, "DEBUG: Compression error: %d\n", ret);
        return false; // 压缩失败
    }else {
        //fprintf(stderr, "DEBUG: Compression successfully.\n");
    }
    for(int i = 0; i < output_size; i += 1) {
        compressed.push_back(com_buf[i]);
    }
    delete[] raw_buf;
    return true; // 压缩成功
}

// 不带检查和校验的解压算法
void unzip_data_unsafe(const BinData& com, BinData& uncom) {
    uncom.clear();
    uint8_t* compressed_data = new uint8_t[com.size()];  // 获取压缩数据
    for(int i = 0; i < com.size(); i += 1) {
        compressed_data[i] = com[i];
    }
    size_t compressed_size = com.size();         // 获取压缩数据大小
    static uint8_t decompressed_data[MAX_BUF];
    size_t decompressed_size = MAX_BUF;
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = (uInt)compressed_size;
    stream.next_in = (Bytef*)compressed_data;
    stream.avail_out = (uInt)decompressed_size;
    stream.next_out = (Bytef*)decompressed_data;
    assert (inflateInit(&stream) == Z_OK);// 处理初始化失败的情况
    int result = inflate(&stream, Z_FINISH);
    if (result != Z_STREAM_END) { // 处理解压失败的情况
        inflateEnd(&stream);
        //printf("WARN : stream.total_out = %u, when failed.\n", (uint32_t)stream.total_out);
    }
    decompressed_size = stream.total_out;
    inflateEnd(&stream);
    delete[] compressed_data;
    for(int i = 0; i < decompressed_size; i += 1) {
        uncom.push_back(decompressed_data[i]);
    }
}

// 用于调试输出数组信息
void debug_output(const char* name, const BinData& d) {
    printf("DEBUG: %20s: ", name);
    for(int i = 0; i < d.size(); i += 1) {
        printf("%02x ", d[i]);
    }
    putchar('\n');
}

// int main() {
//     BinData raw {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
//     BinData com; zip_data(raw, com);
//     BinData unz; unzip_data_unsafe(com, unz);
//     for(int i = 0; i < unz.size(); i += 1) {
//         printf("%02x ", unz[i]);
//     }
//     putchar('\n');
//     return 0;
// }
