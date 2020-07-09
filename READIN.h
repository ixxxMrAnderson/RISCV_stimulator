//
// Created by 储浩天 on 2020/7/7.
//

#ifndef RICV_READIN_H
#define RICV_READIN_H

#include "bin.h"

void READIN() {
    char buffer[10];
    int mem_pointer = 0;
    while (scanf("%s", buffer) != EOF){
        if (buffer[0] == '@'){
            sscanf(buffer + 1, "%x", &mem_pointer);
            continue;
        }
        usl t1, t2, t3, t4;
        sscanf(buffer, "%x", &t1);
        scanf("%x %x %x", &t2, &t3, &t4);
        usl tmp_code = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1;
        memcpy(memory + mem_pointer, &tmp_code, sizeof(tmp_code));
        mem_pointer += 4;
    }
}

#endif //RICV_READIN_H
