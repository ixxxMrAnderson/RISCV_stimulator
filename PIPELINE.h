//
// Created by 储浩天 on 2020/7/7.
//

#ifndef RICV_PIPELINE_H
#define RICV_PIPELINE_H

#include "bin.h"

void IF(){
    if (IF_state == CLOSE) return;
    usl if_code;
    memcpy(&if_code, memory + pc, 4);
    if (IF_ID.state == TO_DO){
        IF_state = PAUSE;
        return;
    }
    IF_ID.code = if_code;
    IF_ID.npc = pc + 4;
    IF_ID.state = TO_DO;
    IF_state = AVAILABLE;
    if (if_code == 0xff00513) IF_state = CLOSE;
}

void ID(){
    if (ID_state == CLOSE) return;
    if (IF_ID.state == EMPTY|| ID_EX.state == TO_DO){
        ID_state = PAUSE;
        return;
    }
    IF_ID.state = EMPTY;
    usl df_code = IF_ID.code;
    usl id_op_code = df_code & 0x7f, funct3 = (df_code >> 12) & 0b111, funct7 = df_code >> 25;
    int rd = 0, imm = 0, sext = 0, rs1 = 0, rs2 = 0;
    OPCODE opcode = INIT;
    switch (id_op_code){
        case 0b0110111:
            opcode = LUI;
            rd = (df_code & b_11_7) >> 7;
            imm = df_code >> 12;
            sext = U_imm(df_code);
            break;
        case 0b0010111:
            opcode = AUIPC;
            rd = (df_code & b_11_7) >> 7;
            imm = df_code >> 12;
            sext = U_imm(df_code);
            break;
        case 0b1101111:
            opcode = JAL;
            rd = (df_code & b_11_7) >> 7;
            imm = ((df_code >> 31) << 20) | (((df_code & b_30_21) >> 21) << 1) | (((df_code & (1 << 20)) >> 20) << 11) | (((df_code & b_19_12) >> 12) << 12);
            sext = J_imm(df_code);
            break;
        case 0b1100111:
            opcode = JALR;
            rd = (df_code & b_11_7) >> 7;
            rs1 = (df_code >> 15) & 0b11111;
            imm = df_code >> 20;
            sext = I_imm(df_code);
            break;
        case 0b0000011:
            rd = (df_code & b_11_7) >> 7;
            rs1 = (df_code >> 15) & 0b11111;
            imm = df_code >> 20;
            sext = I_imm(df_code);
            switch (funct3){
                case 0b000:
                    opcode = LB;
                    break;
                case 0b001:
                    opcode = LH;
                    break;
                case 0b010:
                    opcode = LW;
                    break;
                case 0b100:
                    opcode = LBU;
                    break;
                case 0b101:
                    opcode = LHU;
                    break;
            }
            break;
        case 0b1100011:
            rs1 = (df_code >> 15) & 0b11111;
            rs2 = (df_code >> 20) & 0b11111;
            imm = ((df_code >> 31) << 12) | (((df_code & b_30_25) >> 25) << 5) | (((df_code & b_11_8) >> 8) << 1) | (((df_code >> 7) & 1) << 11);
            sext = B_imm(df_code);
            switch (funct3){
                case 0b000:
                    opcode = BEQ;
                    break;
                case 0b001:
                    opcode = BNE;
                    break;
                case 0b100:
                    opcode = BLT;
                    break;
                case 0b101:
                    opcode = BGE;
                    break;
                case 0b110:
                    opcode = BLTU;
                    break;
                case 0b111:
                    opcode = BGEU;
                    break;
            }
            break;
        case 0b0100011:
            rs1 = (df_code >> 15) & 0b11111;
            rs2 = (df_code >> 20) & 0b11111;
            imm = (((df_code & b_31_25) >> 25) << 5) | ((df_code & b_11_7) >> 7);
            sext = S_imm(df_code);
            switch (funct3){
                case 0b000:
                    opcode = SB;
                    break;
                case 0b001:
                    opcode = SH;
                    break;
                case 0b010:
                    opcode = SW;
                    break;
            }
            break;
        case 0b0110011:
            rd = (df_code & b_11_7) >> 7;
            rs1 = (df_code >> 15) & 0b11111;
            rs2 = (df_code >> 20) & 0b11111;
            switch (funct3){
                case 0b000:
                    switch (funct7) {
                        case 0b0000000:
                            opcode = ADD;
                            break;
                        case 0b0100000:
                            opcode = SUB;
                            break;
                    }
                    break;
                case 0b001:
                    opcode = SLL;
                    break;
                case 0b010:
                    opcode = SLT;
                    break;
                case 0b011:
                    opcode = SLTU;
                    break;
                case 0b100:
                    opcode = XOR;
                    break;
                case 0b101:
                    switch (funct7){
                        case 0b0000000:
                            opcode = SRL;
                            break;
                        case 0b0100000:
                            opcode = SRA;
                            break;
                    }
                    break;
                case 0b110:
                    opcode = OR;
                    break;
                case 0b111:
                    opcode = AND;
                    break;
            }
            break;
        case 0b0010011:
            rd = (df_code & b_11_7) >> 7;
            rs1 = (df_code >> 15) & 0b11111;
            imm = df_code >> 20;
            sext = I_imm(df_code);
            switch (funct3){
                case 0b000:
                    opcode = ADDI;
                    break;
                case 0b010:
                    opcode = SLTI;
                    break;
                case 0b011:
                    opcode = SLTIU;
                    break;
                case 0b100:
                    opcode = XORI;
                    break;
                case 0b110:
                    opcode = ORI;
                    break;
                case 0b111:
                    opcode = ANDI;
                    break;
                case 0b001:
                    opcode = SLLI;
                    break;
                case 0b101:
                    imm = (df_code & b_24_20) >> 20;
                    switch (funct7){
                        case 0b0000000:
                            opcode = SRLI;
                            break;
                        case 0b0100000:
                            opcode = SRAI;
                            break;
                    }
                    break;
            }
            break;
    }
    if (df_code == 0xff00513) opcode = SHUT;
    ID_EX.op_code = opcode;
    ID_EX.rd = rd;
    ID_EX.rs1 = rs1;
    ID_EX.r_v1 = reg[rs1];
    ID_EX.rs2 = rs2;
    ID_EX.r_v2 = reg[rs2];
    ID_EX.imm = imm;
    ID_EX.sext = sext;
    ID_EX.npc = IF_ID.npc;
    ID_state = AVAILABLE;
    ID_EX.state = TO_DO;
    if (opcode == SHUT) ID_state = CLOSE;
    //printf("%x : %x, %s, (rd)%s, (rs1)%s, (rs2)%s, %x\n", pc,  df_code, INST_string[opcode], REG_string[rd], REG_string[rs1], REG_string[rs2], sext);
}

void EX(){
    if (EX_state == CLOSE) return;
    if (ID_EX.state == EMPTY|| EX_MEM.state == TO_DO){
        EX_state = PAUSE;
        return;
    }
    ID_EX.state = EMPTY;
    OPCODE opcode = ID_EX.op_code;
    int rd = ID_EX.rd, imm = ID_EX.imm, sext = ID_EX.sext, r_v1 = ID_EX.r_v1, r_v2 = ID_EX.r_v2, ALU_out = 0, rs1 = ID_EX.rs1, rs2 = ID_EX.rs2, npc = ID_EX.npc;
    switch (opcode){
        case LB: case LH: case LW: case LBU: case LHU: case ADDI: case SB: case SH: case SW:
            ALU_out = sext + r_v1;
            break;
        case SLTI:
            ALU_out = r_v1 < sext;
            break;
        case SLTIU:
            ALU_out = (usl)(r_v1) < (usl)(sext);
            break;
        case XORI:
            ALU_out = r_v1 ^ sext;
            break;
        case ORI:
            ALU_out = r_v1 | sext;
            break;
        case ANDI:
            ALU_out = r_v1 & sext;
            break;
        case SLLI:
            ALU_out = r_v1 << imm;
            break;
        case SRLI:
            ALU_out = (usl)(r_v1) >> (usl)(imm);
            break;
        case SRAI:
            ALU_out = r_v1 >> imm;
            break;
        case JALR:
            ALU_out = npc;
            npc = (r_v1 + sext) & (-2);
            break;
        case BEQ:
            if (r_v1 == r_v2) npc += (sext - 4);
            break;
        case BNE:
            if (r_v1 != r_v2) npc += (sext - 4);
            break;
        case BLT:
            if (r_v1 < r_v2) npc += (sext - 4);
            break;
        case BGE:
            if (r_v1 >= r_v2) npc += (sext - 4);
            break;
        case BLTU:
            if ((usl)(r_v1) < (usl)(r_v2)) npc += (sext - 4);
            break;
        case BGEU:
            if ((usl)(r_v1) >= (usl)(r_v2)) npc += (sext - 4);
            break;
        case LUI:
            ALU_out = sext;
            break;
        case AUIPC:
            ALU_out = npc + sext - 4;
            break;
        case JAL:
            ALU_out = npc;
            npc += (sext - 4);
            break;
        case ADD:
            ALU_out = r_v1 + r_v2;
            break;
        case SUB:
            ALU_out = r_v1 - r_v2;
            break;
        case SLL:
            ALU_out = r_v1 << (r_v2 & 0x1f);
            break;
        case SLT:
            ALU_out = r_v1 < r_v2;
            break;
        case SLTU:
            ALU_out = (usl)(r_v1) < (usl)(r_v2);
            break;
        case XOR:
            ALU_out = r_v1 ^ r_v2;
            break;
        case SRL:
            ALU_out = (usl)(r_v1) >> (usl)((r_v2 & 0x1f));
            break;
        case SRA:
            ALU_out = r_v1 >> (r_v2 & 0x1f);
            break;
        case OR:
            ALU_out = r_v1 | r_v2;
            break;
        case AND:
            ALU_out = r_v1 & r_v2;
            break;
    }
    EX_MEM.op_code = opcode;
    EX_MEM.rd = rd;
    EX_MEM.rs1 = rs1;
    EX_MEM.r_v1 = r_v1;
    EX_MEM.rs2 = rs2;
    EX_MEM.r_v2 = r_v2;
    EX_MEM.imm = imm;
    EX_MEM.sext = sext;
    EX_MEM.ALU_out = ALU_out;
    EX_MEM.npc = npc;
    EX_MEM.state = TO_DO;
    EX_state = AVAILABLE;
    if (opcode == SHUT) EX_state = CLOSE;
}

void MEM(){
    if (MEM_state == CLOSE) return;
    pc = EX_MEM.npc;
    if (EX_MEM.state == EMPTY|| MEM_WB.state == TO_DO){
        MEM_state = PAUSE;
        return;
    }
    EX_MEM.state = EMPTY;
    // if (!(mem_cnt % 3) && EX_MEM.state == EMPTY || MEM_WB.state == TO_DO && !((mem_cnt + 1) % 3)){
    //     MEM_state = PAUSE;
    //     return;
    // }
    // if (!(mem_cnt % 3)) EX_MEM.state = EMPTY;
    // mem_cnt ++;
    // if (mem_cnt % 3){
    //     MEM_state = WORKING;
    //     return;
    // }
    int ALU_out = EX_MEM.ALU_out, output = 0, r_v2 = EX_MEM.r_v2;
    usl u_output = 0;
    switch (EX_MEM.op_code){
        case LB:
            memcpy(&output, memory + ALU_out, 1);
            MEM_WB.output = output;
            break;
        case LH:
            memcpy(&output, memory + ALU_out, 2);
            MEM_WB.output = output;
            break;
        case LW:
            memcpy(&output, memory + ALU_out, 4);
            MEM_WB.output = output;
            break;
        case LBU:
            memcpy(&u_output, memory + ALU_out, 1);
            MEM_WB.output = u_output;
            break;
        case LHU:
            memcpy(&u_output, memory + ALU_out, 2);
            MEM_WB.output = u_output;
            break;
        case SB:
            memcpy(memory + ALU_out, &r_v2, 1);
            break;
        case SH:
            memcpy(memory + ALU_out, &r_v2, 2);
            break;
        case SW:
            memcpy(memory + ALU_out, &r_v2, 4);
            break;
        default:
            MEM_WB.output = ALU_out;
    }
    MEM_WB.rd = EX_MEM.rd;
    MEM_WB.op_code = EX_MEM.op_code;
    MEM_state = AVAILABLE;
    if (MEM_WB.op_code == SHUT) MEM_state = CLOSE;
    MEM_WB.state = TO_DO;
}

void WB(){
    if (WB_state == CLOSE) return;
    if (MEM_WB.state == EMPTY){
        WB_state = PAUSE;
        return;
    }
    MEM_WB.state = EMPTY;
    if (MEM_WB.op_code == SHUT) WB_state = CLOSE;
    else {
        switch (MEM_WB.op_code){
            case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                break;
            default:
                //printf("OP: %s, write %x in %s\n", INST_string[MEM_WB.op_code], MEM_WB.output, REG_string[MEM_WB.rd]);
                reg[MEM_WB.rd] = MEM_WB.output;
        }
        WB_state = AVAILABLE;
    }
}

bool FORWARD(){
//    printf("pipeline_state: %s, %s, %s, %s, %s\n", ST_[IF_state], ST_[ID_state], ST_[EX_state], ST_[MEM_state], ST_[WB_state]);
//    printf("station_state: %s, %s, %s, %s\n", ST_[IF_ID.state], ST_[ID_EX.state], ST_[EX_MEM.state], ST_[MEM_WB.state]);
    if (IF_state == CLOSE && ID_state == CLOSE && EX_state == CLOSE && MEM_state == CLOSE && WB_state == CLOSE) return 0;
    return 1;
}
#endif //RICV_PIPELINE_H
