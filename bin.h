//
// Created by 储浩天 on 2020/7/7.
//

#ifndef RICV_BIN_H
#define RICV_BIN_H

#include <cstdio>
#include <iostream>
#include <bitset>
#include <cstring>
#include <queue>
#define usl unsigned long
using namespace std;

/*--------------------basic declare--------------------*/
enum OPCODE{
    INIT,
    LB, LH, LW, LBU, LHU, ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, JALR, //I
    SB, SH, SW, //S
    BEQ, BNE, BLT, BGE, BLTU, BGEU, //B(S)
    LUI, AUIPC, //U
    JAL, //J(U)
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, //R
    SHUT
};
enum STATE{
    AVAILABLE, PAUSE, WORKING, EMPTY, TO_DO, CLOSE
};
const char ST_[6][15] = {
    "AVAILABLE", "PAUSE", "WORKING", "EMPTY", "TO_DO", "CLOSE"
};
const char REG_string[40][15] = {
        "x0", "RA", "SP", "GP", "TP", "T0",
        "T1", "T2", "S0", "S1", "A0", "A1",
        "A2", "A3", "A4", "A5", "A6", "A7",
        "S2", "S3", "S4", "S5", "S6", "S7",
        "S8", "S9", "S10", "S11", "t3", "T4",
        "T5", "T6", "UNKNOWN_REG"
};
const char INST_string[40][15] = {
        "INIT",
        "LB", "LH", "LW", "LBU", "LHU","ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI",  "SRAI", "JALR",
        "SB", "SH", "SW",
        "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU",
        "LUI", "AUIPC",
        "JAL",
        "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR",  "SRL", "SRA", "OR", "AND"
};
const usl b_31_11 = (-1) ^ ((1 << 11) - 1);
const usl b_31_12 = (-1) ^ ((1 << 12) - 1);
const usl b_31_20 = (-1) ^ ((1 << 20) - 1);
const usl b_31_25 = (-1) ^ ((1 << 25) - 1);
const usl b_30_25 = (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25);
const usl b_30_21 = (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21);
const usl b_24_21 = (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21);
const usl b_24_20 = (1 << 24) | (1 << 23) | (1 << 22) | (1 << 21) | (1 << 20);
const usl b_19_12 = (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16) | (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12);
const usl b_11_8 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8);
const usl b_11_7 = (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7);

unsigned char memory[0x20000];
int reg[32], pc, IF_state = 0, ID_state = 0, EX_state = 0, MEM_state = 0, WB_state = 0, mem_cnt = 0;
//station_state: 0_empty, 1_to-do; pipeline_state: 0_stall, 1_unfinished, 2_finished
struct if_id{
    usl code = 0;
    int npc, state = 0;
} IF_ID;
struct id_ex{
    usl code = 0;
    OPCODE op_code = INIT;
    int rd = 0, rs1 = 0, rs2 = 0, imm = 0, sext = 0, state = 0, r_v1 = 0, r_v2 = 0, npc = 0;
} ID_EX;
struct ex_mem{
    usl code = 0;
    OPCODE op_code = INIT;
    int rd = 0, rs1 = 0, rs2 = 0, imm = 0, sext = 0, state = 0, ALU_out = 0, r_v1 = 0, r_v2 = 0, npc = 0;
} EX_MEM;
struct mem_wb{
    usl code = 0;
    OPCODE op_code = INIT;
    int rd = 0, output = 0, state = 0;
} MEM_WB;

/*--------------------get operand number--------------------*/
int I_imm(usl tar){
    int tmp = tar >> 20;
    if (tmp >> 11) tmp |= b_31_11;
    return tmp;
}
int S_imm(usl tar){
    int tmp = (tar & b_11_7) >> 7;
    tmp |= (((tar & b_31_25) >> 25) << 5);
    if (tmp >> 11) tmp |= b_31_11;
    return tmp;
}
int B_imm(usl tar){
    int tmp = ((tar & b_11_8) >> 8) << 1;
    tmp |= (((tar & b_30_25) >> 25) << 5);
    tmp |= (((tar >> 7) & 1U) << 11);
    if (tar >> 31) tmp |= b_31_12;
    return tmp;
}
int U_imm(usl tar){return tar & b_31_12;}
int J_imm(usl tar){
    int tmp = ((tar & b_30_21) >> 21) << 1;
    tmp |= (tar & b_19_12);
    tmp |= (((tar >> 20) & 1U) << 11);
    if (tar >> 31) tmp |= b_31_20;
    return tmp;
}
#endif //RICV_BIN_H
