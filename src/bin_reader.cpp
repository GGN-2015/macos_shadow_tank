#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>
using namespace std;

vector<uint8_t> content;
void readin(const char* filename) {
    FILE* fpin = fopen(filename, "rb");
    assert(fpin != NULL);
    while(!feof(fpin)) { // 读入全文
        content.push_back(fgetc(fpin));
    }
    content.pop_back();
    printf("DEBUG: readin %u char.\n", (uint32_t)content.size());
    fclose(fpin);
}

void query() {
    printf("INPUT: from len: ");
    int from, len; scanf("%d%d", &from, &len);
    printf("BDATA: ");
    for(int i = 0; i < len; i += 1) {
        int p = from + i;
        if(0 <= p && p < content.size()) {
            printf("%02x ", content[p]);
        }
    }
    putchar('\n');
}

int main() {
    printf("INPUT: filename: ");
    char filename[256]; scanf("%s", filename);
    readin(filename);
    while(true) query();
    return 0;
}