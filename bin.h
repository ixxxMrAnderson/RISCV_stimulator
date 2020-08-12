//
// Created by 储浩天 on 2020/7/7.
//

#ifndef RICV_BIN_H
#define RICV_BIN_H

#include <cstdio>
#include <iostream>
#include <bitset>
#include <cstring>
using namespace std;
/*--------------------basic declare--------------------*/
enum OPCODE{
    INIT,
    LB, LH, LW, LBU, LHU, ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, JALR, //I
    SB, SH, SW, //S
    BEQ, BNE, BLT, BGE, BLTU, BGEU, //B(S)
    LUI, AUIPC, //U
    JAL, //J(U)
    ADD, SUB, TIMES, DIVIDE, MOD, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, //R
    SHUT, INPUT, MALLOC
};
const char OPCODE_str[44][7]{
    "INIT",
    "LB", "LH", "LW", "LBU", "LHU", "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI", "JALR", //I
    "SB", "SH", "SW", //S
    "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU", //B(S)
    "LUI", "AUIPC", //U
    "JAL", //J(U)
    "ADD", "SUB", "TIMES", "DIVIDE", "MOD", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND", //R
    "SHUT", "INPUT", "MALLOC"
};
const char state_str[6][10]{
        "VACANT", "STALL", "WORKING", "DONE",
        "EMPTY", "TO_DO"
};
enum STATE{
    VACANT, STALL, WORKING, DONE,
    EMPTY, TO_DO
};

const uint32_t b_31_11 = (-1) ^ ((1 << 11) - 1);
const uint32_t b_31_12 = (-1) ^ ((1 << 12) - 1);
const uint32_t b_31_20 = (-1) ^ ((1 << 20) - 1);
const uint32_t b_31_25 = (-1) ^ ((1 << 25) - 1);
const uint32_t b_30_25 = (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25);
const uint32_t b_30_21 = (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21);
const uint32_t b_24_20 = (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21) | (1 << 20);
const uint32_t b_19_12 = (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16) | (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12);
const uint32_t b_11_8 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8);
const uint32_t b_11_7 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7);

uint8_t memory[0x20000];
int reg[32], pc, IF_state = 0, ID_state = 0, EX_state = 0, MEM_state = 0, WB_state = 0, mem_cnt = 0, total = 0, correct = 0;
long long new_mem_space = 0, cal_tot = 0, jump_tot = 0, mem_tot = 0;

struct if_id{
    uint32_t code = 0;
    int state = EMPTY, npc = 0, branch = 0;
} IF_ID;
struct id_ex{
    OPCODE op_code = INIT;
    int32_t imm = 0, sext = 0, r_v1 = 0, r_v2 = 0;
    int state = EMPTY, rd = 0, rs1 = 0, rs2 = 0, npc = 0, branch = 0;
} ID_EX;
struct ex_mem{
    OPCODE op_code = INIT;
    int state = EMPTY, rd = 0, rs2 = 0;
    int32_t ALU_out = 0, r_v2 = 0;
} EX_MEM, MEM_;
struct mem_wb{
    OPCODE op_code = INIT;
    int rd = 0, state = EMPTY;
    int32_t output = 0;
} MEM_WB;
/*--------------------get operand number--------------------*/
int I_imm(uint32_t tar){
    int tmp = tar >> 20;
    if (tmp >> 11) tmp |= b_31_11;
    return tmp;
}
int S_imm(uint32_t tar){
    int tmp = (tar & b_11_7) >> 7;
    tmp |= (((tar & b_31_25) >> 25) << 5);
    if (tmp >> 11) tmp |= b_31_11;
    return tmp;
}
int B_imm(uint32_t tar){
    int tmp = ((tar & b_11_8) >> 8) << 1;
    tmp |= (((tar & b_30_25) >> 25) << 5);
    tmp |= (((tar >> 7) & 1U) << 11);
    if (tar >> 31) tmp |= b_31_12;
    return tmp;
}
int U_imm(uint32_t tar){return tar & b_31_12;}
int J_imm(uint32_t tar){
    int tmp = ((tar & b_30_21) >> 21) << 1;
    tmp |= (tar & b_19_12);
    tmp |= (((tar >> 20) & 1U) << 11);
    if (tar >> 31) tmp |= b_31_20;
    return tmp;
}
/*--------------------detect data--------------------*/
void DETECT(int rs){
    if (!rs){
        ID_EX.r_v1 = 0;
        EX_state = VACANT;
        return;
    }
    if (EX_MEM.state == TO_DO && EX_MEM.rd == rs){
        switch (EX_MEM.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                EX_state = STALL;
                return;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v1 = EX_MEM.ALU_out;
                EX_state = VACANT;
                return;
        }
    }
    if (MEM_state == WORKING && MEM_.rd == rs){
        switch (MEM_.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                EX_state = STALL;
                return;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v1 = MEM_.ALU_out;
                EX_state = VACANT;
                return;
        }
    }
    if (MEM_WB.state == TO_DO && MEM_WB.rd == rs){
        ID_EX.r_v1 = MEM_WB.output;
        EX_state = VACANT;
    } else {
        ID_EX.r_v1 = reg[ID_EX.rs1];
        EX_state = VACANT;
    }
}
void DETECT(int rs1, int rs2){
    bool jd1 = 0, jd2 = 0, jdjd1 = 0, jdjd2 = 0;
    if (!rs1 && !jdjd1){
        ID_EX.r_v1 = 0;
        jd1 = 1, jdjd1 = 1;
    }
    if (EX_MEM.state == TO_DO && EX_MEM.rd == rs1 && !jdjd1){
        switch (EX_MEM.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                jd1 = 0, jdjd1 = 1;
                break;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v1 = EX_MEM.ALU_out;
                jd1 = 1, jdjd1 = 1;
        }
    }
    if (MEM_state == WORKING && MEM_.rd == rs1 && !jdjd1){
        switch (MEM_.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                jd1 = 0, jdjd1 = 1;
                break;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v1 = MEM_.ALU_out;
                jd1 = 1, jdjd1 = 1;
        }
    }
    if (MEM_WB.state == TO_DO && MEM_WB.rd == rs1 && !jdjd1){
        ID_EX.r_v1 = MEM_WB.output;
        jd1 = 1, jdjd1 = 1;
    }
    if (!jdjd1){
        ID_EX.r_v1 = reg[ID_EX.rs1];
        jd1 = 1;
    }
    if (!rs2 && !jdjd2){
        ID_EX.r_v2 = 0;
        jd2 = 1, jdjd2 = 1;
    }
    if (EX_MEM.state == TO_DO && EX_MEM.rd == rs2 && !jdjd2){
        switch (EX_MEM.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                jd2 = 0, jdjd2 = 1;
                break;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v2 = EX_MEM.ALU_out;
                jd2 = 1, jdjd2 = 1;
        }
    }
    if (MEM_state == WORKING && MEM_.rd == rs2 && !jdjd2){
        switch (MEM_.op_code){
            case LB: case LH: case LW: case LBU: case LHU:
                jd2 = 0, jdjd2 = 1;
                break;
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                ID_EX.r_v2 = MEM_.ALU_out;
                jd2 = 1, jdjd2 = 1;
        }
    }
    if (MEM_WB.state == TO_DO && MEM_WB.rd == rs2 && !jdjd2){
        ID_EX.r_v2 = MEM_WB.output;
        jd2 = 1, jdjd2 = 1;
    }
    if (!jdjd2){
        ID_EX.r_v2 = reg[ID_EX.rs2];
        jd2 = 1;
    }
    if (jd1 && jd2) EX_state = VACANT;
    else EX_state = STALL;
}
/*--------------------branch prediction--------------------*/
uint8_t BHT[1 << 8], PHT[16];
void update(int pcx, bool jump){
    int index = (pcx >> 2) & 0xff;
    if (jump){
        if (PHT[BHT[index] & 0b1111] != 0b11){
            PHT[BHT[index] & 0b1111] ++;
        }
    } else {
        if (PHT[BHT[index] & 0b1111] != 0b00){
            PHT[BHT[index] & 0b1111] --;
        }
    }
    BHT[index] = ((BHT[index] << 1) + jump) & 0b1111;
}
bool predict(int x){
    return (PHT[BHT[(x >> 2) & 0xff] & 0b1111] & 0b10) >> 1;
}
void clear(){
    IF_ID.state = EMPTY;
    ID_EX.state = EMPTY;
    if (IF_state == DONE) IF_state = VACANT;
    if (ID_state == DONE) ID_state = VACANT;
}

#endif //RICV_BIN_H
