#include "get_rel_blk5.h"

int main() {
    BinData raw {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    BinData rel_com;
    get_rel_blk5(raw, rel_com);
    debug_output("rel_com", rel_com);
    BinData real_com;
    make_reliable_avilable(rel_com, real_com);
    debug_output("real_com", real_com);
    BinData fin_raw;
    unzip_data_unsafe(real_com, fin_raw);
    debug_output("fin_raw", fin_raw);
    return 0;
}