#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "zip_utils.h" // deflate/inflate
using namespace std;

// 签名的长度
const int SIGNATURE_LENGTH = 8;
const int LENGTH_SIZE = 4;
const int TYPE_SIZE = 4;
const int CHECKSUM_SIZE = 4;
const int BYTE_BITS = 8;

// 缓存
vector<uint8_t>       content;
vector<uint8_t>  idat_content;
vector<vector<uint8_t>> idats; // 记录每个数据块

// 将 PNG 文件读入到内存
void get_content(const char* filename) {
    content.clear();
    idat_content.clear();
    FILE* fpin = fopen(filename, "rb");
    assert(fpin != NULL); // 文件必须存在
    while(!feof(fpin)) {
        content.push_back(fgetc(fpin));
    }
    content.pop_back();
}


// 检查 PNG 的文件签名是否正确
bool check_signature() {
    assert(content.size()>= SIGNATURE_LENGTH);
    const uint8_t magic[] = {137, 80, 78, 71, 13, 10, 26, 10};
    for(int i = 0; i < SIGNATURE_LENGTH; i += 1) {
        if(magic[i] != content[i]) {
            return false; // 签名错误
        }
    }
    return true; // 签名正确
}

// 显示一段连续区域的 16 进制数据
void debug_show(int begin, int end) {
    assert(end > begin);
    assert(0 <= begin);
    assert(end <= content.size());
    printf("DEBUG: %10d - %10d: ", begin, end-1);
    for(int i = begin; i < end; i += 1) {
        printf("%02x ", content[i]);
    }
    putchar('\n');
}

// 从存储器中读取整数
uint32_t get_integer(int begin, int len) {
    uint32_t ans = 0;
    for(int i = 0; i < len; i += 1) {
        uint8_t val = content[begin + i];
        ans = (ans << 8) | val;
    }
    return ans;
}

// 解析 iDOT 块的内容
// 参考 https://www.hackerfactor.com/blog/index.php?/archives/895-Connecting-the-iDOTs.html
void check_idot_content(int posnow) {
    int p = posnow + 8; // 数据起始位置
    printf("               : Height Divisor   : %u\n", get_integer(p + 0*4, 4));
    printf("               : Always 0         : %u\n", get_integer(p + 1*4, 4));
    printf("               : Half Height      : %u\n", get_integer(p + 2*4, 4));
    printf("               : Always 40        : %u\n", get_integer(p + 3*4, 4));
    printf("               : Height Part I    : %u\n", get_integer(p + 4*4, 4));
    printf("               : Height Part II   : %u\n", get_integer(p + 5*4, 4));
    printf("               : IDAT offset      : %u\n", get_integer(p + 6*4, 4));
}

uint32_t g_width  = 0; // 获取图片尺寸
uint32_t g_height = 0;
// 解析 IHDR 块的内容
// 参考 http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IHDR
void check_ihdr_content(int posnow) {
    int p = posnow + 8;
    printf("               : Width            : %u\n", g_width   = get_integer(p + 0*4, 4));
    printf("               : Height           : %u\n", g_height = get_integer(p + 1*4, 4));
    printf("               : Bit Depth        : %u\n", get_integer(p +   8, 1));
    printf("               : Color Type       : %u\n", get_integer(p +   9, 1));
    printf("               : Zip Method       : %u\n", get_integer(p +  10, 1));
    printf("               : Filter Method    : %u\n", get_integer(p +  11, 1));
    printf("               : Interlace Method : %u\n", get_integer(p +  12, 1));
    FILE* fpout = fopen("size.txt", "w"); // 把图片尺寸输出到文件
    fprintf(fpout, "%u %u", g_width, g_height);
    fclose(fpout);
}

// 使用正常顺序，获取 PNG 的块序列
void get_block_list() {
    int posnow = SIGNATURE_LENGTH;
    while(posnow < content.size()) {
        uint32_t blk_data_length = get_integer(posnow, LENGTH_SIZE);
        char blk_type[TYPE_SIZE + 1] = {};
        for(int i = 0; i < LENGTH_SIZE; i += 1) {
            blk_type[i] = content[posnow + LENGTH_SIZE + i];
        }
        uint32_t crc32 = get_integer(posnow + LENGTH_SIZE + TYPE_SIZE + blk_data_length, CHECKSUM_SIZE);
        printf("BLOCK: %s pos: %8u, datalen: %7u, crc32: 0x%08x\n", blk_type, posnow, blk_data_length, crc32);
        if(strcmp(blk_type, "IHDR") == 0) { // 检查 IHDR 块的内容
            check_ihdr_content(posnow);
        }
        if(strcmp(blk_type, "iDOT") == 0) { // 检查 iDOT 块的内容
            check_idot_content(posnow);
        }
        if(strcmp(blk_type, "IDAT") == 0) { // 获取所有 IDAT 内容
            int p = posnow + 8;             // 块数据起始地址
            vector<uint8_t> data_now;
            for(int i = 0; i < blk_data_length; i += 1) {
                idat_content.push_back(content[p + i]);
                data_now    .push_back(content[p + i]);
            }
            idats.push_back(data_now); // 单独记录每个数据块
        }
        posnow += (LENGTH_SIZE + TYPE_SIZE + blk_data_length + CHECKSUM_SIZE);
    }
}

// 在全局范围内扫描 IDAT 块的可能存在处
void idat_scan() {
    for(int i = 0; i + TYPE_SIZE <= content.size(); i += 1) {
        char type_name[TYPE_SIZE + 1] = {};
        for(int j = 0; j < TYPE_SIZE; j += 1) {
            type_name[j] = content[j + i];
        }
        auto len = get_integer(i - 4, 4);
        if(strcmp(type_name, "IDAT") == 0) {
            printf("BSCAN: IDAT found at %u, len: %u\n", i - 4, len);
        }
    }
}

// 把所有的 IDAT 段连接起来，放到一个文件里
void idat_dump(const char* filename) {
    FILE* fpout = fopen(filename, "wb");
    for(int i = 0; i < idat_content.size(); i += 1) {
        fputc(idat_content[i], fpout);
    }
    fclose(fpout);
    printf("     : dump raw data into %s.\n", filename);
}

// 解压 idat_content 中的内容并存入 idat_content_unzip
void unzip_idat_content(const char* output_filename) {
    BinData unzipped_data;
    unzip_data(idat_content, unzipped_data);
    
    FILE* fpout = fopen(output_filename, "wb"); // 输出到文件
    for(int i = 0; i < unzipped_data.size(); i += 1) {
        fputc(unzipped_data[i], fpout);
    }
    fclose(fpout);
    printf("     : dump unzipped data into %s.\n", output_filename);
}

// 计算 alder32 检查和，补充到结尾
void add_adler32(BinData& part) {
    uint8_t* arr = new uint8_t[part.size()];
    for(int i = 0; i < part.size(); i += 1) {
        arr[i] = part[i];
    }
    uint32_t checksum = adler32(0L, Z_NULL, 0);
             checksum = adler32(checksum, arr + 7, part.size() - 7);
    delete[] arr;
    for(int i = 3; i >= 0; i -= 1) {
        uint8_t byte_now = (checksum >> (i * 8)) & 0xff; // MSB first
        part.push_back(byte_now);
    }
}

// 按照 ios 的解压风格对数据进行解压
void ios_unzip(const char* dump_file) {
    printf("DEBUG: generating ios unzip.\n");
    assert(idats.size() == 2);
    BinData part1 = idats[0];     // 删除 part1 的最后五个字节
    for(int i = 1; i <= 5; i += 1) part1.pop_back();
    part1[2] |= 1;                // final tag
    add_adler32(part1);           // 补充结尾的检查和
    BinData part2 = {0x78, 0xda}; // 补充 deflate 头部信息
    for(int i = 0; i < idats[1].size(); i += 1) {
        part2.push_back(idats[1][i]);
    }
    printf("     : part1.size() = %u, part2.size() = %u\n", (uint32_t)part1.size(), (uint32_t)part2.size());
    BinData unzip_p1, unzip_p2;
    unzip_data(part1, unzip_p1);
    unzip_data_unsafe(part2, unzip_p2);
    FILE* fpout = fopen(dump_file, "wb"); // 输出到文件
    for(int i = 0; i < unzip_p1.size(); i += 1) {
        fputc(unzip_p1[i], fpout);
    }
    for(int i = 0; i < unzip_p2.size(); i += 1) {
        fputc(unzip_p2[i], fpout);
    }
    fclose(fpout);
    printf("     : dump ios unzip into %s.\n", dump_file);
}

// 运行 PNG 块检查
void run_check(const char* filename) {
    get_content(filename);
    if(!check_signature()) { // 检查签名是否正确
        printf("FATAL: wrong signature!\n");
        exit(1);
    }else {
        printf("DEBUG: signature correct.\n");
    }
    get_block_list();
    idat_dump("idat.zip.bin");
    unzip_idat_content("idat.raw.bin");
    ios_unzip("idat.ios.bin");
}

int main() {
    printf("INPUT: filename: "); // 从键盘读入文件名
    char filename[256]; scanf("%s", filename);
    run_check(filename);
    return 0;
}