#include "bin.h"
#include "READIN.h"
#include "PIPELINE.h"
#include <fstream>
long long total_loop = 0;
int main() {
//    freopen("D:\\ClionProjects\\RISCV\\test.txt", "r", stdin);
    READIN();
//    freopen("CON", "r", stdin);
    MEM_.state = VACANT;
    while (1){
        WB();
        reg[0] = 0;
        MEM();
        EX();
        ID();
        IF();
        total_loop ++;
        if (!FORWARD()) break;
    }
//    printf("return:\t%d\n", reg[10]);
//    printf("cal:\t%lld\n", cal_tot);
//    printf("jump:\t%lld\n", jump_tot);
//    printf("mem:\t%lld\n", mem_tot);
    //printf("correct_prediction: %d/%d (%.2f%)", correct, total, double(correct)/double(total)*100);
}
