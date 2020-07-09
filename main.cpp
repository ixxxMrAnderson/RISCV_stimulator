#include "bin.h"
#include "READIN.h"
#include "PIPELINE.h"
#include <fstream>

int main() {
    READIN();
    while (1){
        IF();
        ID();
        EX();
        MEM();
        WB();
        reg[0] = 0;
        if (!FORWARD()) break;
    }
    printf("%d\n", reg[10] & 0xff);
}