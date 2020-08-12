//
// Created by 储浩天 on 2020/7/7.
//

#ifndef RICV_PIPELINE_H
#define RICV_PIPELINE_H

#include "bin.h"

void IF(){
    if (IF_state == DONE) return;
    uint32_t if_code;
    memcpy(&if_code, memory + pc, 4);
    if (IF_ID.state == TO_DO){
        IF_state = VACANT;
        return;
    }
    IF_ID.code = if_code;
    IF_ID.npc = pc + 4;
    IF_ID.state = TO_DO;
    if (if_code == 0xff00513) IF_state = DONE;
    else if ((if_code & 0x7f) == 0b1100011){
        bool jump = predict(pc);
        // jump = 0;
        if (jump){
            pc += B_imm(if_code);
            IF_ID.branch = pc;
            IF_state = VACANT;
        } else {
            pc += 4;
            IF_ID.branch = pc;
            IF_state = VACANT;
        }
    } else {
        pc += 4;
        IF_ID.branch = pc;
        IF_state = VACANT;
    }
}

void ID() {
    if (ID_state == DONE) return;
    else if (IF_ID.state == EMPTY || ID_EX.state == TO_DO) {
        ID_state = VACANT;
        return;
    } else {
        IF_ID.state = EMPTY;
        uint32_t df_code = IF_ID.code;
        uint32_t id_op_code = df_code & 0x7f, funct3 = (df_code >> 12) & 0b111, funct7 = df_code >> 25;
        int rd = 0, imm = 0, sext = 0, rs1 = 0, rs2 = 0;
        OPCODE opcode = INIT;
        switch (id_op_code) {
            case 0b0000000:
                if (funct3 == 0b010){
                    rd = (df_code & b_11_7) >> 7;
                    rs1 = (df_code >> 15) & 0b11111;
                    imm = df_code >> 20;
                    opcode = MALLOC;
                } else {
                    opcode = INPUT;
                    rd = (df_code & b_11_7) >> 7;
                }
                break;
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
                imm = ((df_code >> 31) << 20) | (((df_code & b_30_21) >> 21) << 1) |
                      (((df_code & (1 << 20)) >> 20) << 11) | (((df_code & b_19_12) >> 12) << 12);
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
                switch (funct3) {
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
                imm = ((df_code >> 31) << 12) | (((df_code & b_30_25) >> 25) << 5) | (((df_code & b_11_8) >> 8) << 1) |
                      (((df_code >> 7) & 1) << 11);
                sext = B_imm(df_code);
                switch (funct3) {
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
                switch (funct3) {
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
                switch (funct3) {
                    case 0b000:
                        switch (funct7) {
                            case 0b0000000:
                                opcode = ADD;
                                break;
                            case 0b0100000:
                                opcode = SUB;
                                break;
                            case 0b0010000:
                                opcode = TIMES;
                                break;
                            case 0b0001000:
                                opcode = DIVIDE;
                                break;
                            case 0b0000100:
                                opcode = MOD;
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
                        switch (funct7) {
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
                switch (funct3) {
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
                        switch (funct7) {
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
        ID_EX.r_v1 = 0;
        ID_EX.r_v2 = 0;
        ID_EX.rs1 = rs1;
        ID_EX.rs2 = rs2;
        ID_EX.imm = imm;
        ID_EX.sext = sext;
        ID_EX.state = TO_DO;
        ID_EX.npc = IF_ID.npc;
        ID_EX.branch = IF_ID.branch;
        if (opcode == SHUT) ID_state = DONE;
        else ID_state = VACANT;
        // printf("%s, rd_%d, rs1_%d, rs2_%d, sext_%d, %08X\n", OPCODE_str[opcode], rd, rs1, rs2, sext, df_code);
    }
}

void EX(){
    if (EX_state == DONE || EX_state == STALL) return;
    else if (ID_EX.state == EMPTY|| EX_MEM.state == TO_DO){
        EX_state = VACANT;
        return;
    } else {
        ID_EX.state = EMPTY;
        OPCODE opcode = ID_EX.op_code;
        int rd = ID_EX.rd, imm = ID_EX.imm, sext = ID_EX.sext, r_v1 = ID_EX.r_v1, r_v2 = ID_EX.r_v2, rs2 = ID_EX.rs2, npc = ID_EX.npc, branch = ID_EX.branch, ALU_out = 0;
        if (!ID_EX.rs1) r_v1 = 0;
        if (!ID_EX.rs2) r_v2 = 0;
        switch (opcode) {
            case LB: case LH: case LW: case LBU: case LHU: case ADDI: case SB: case SH: case SW:
                ALU_out = sext + r_v1;
                break;
            case INPUT:
                scanf("%d", &ALU_out);
                break;
            case MALLOC:
                ALU_out = new_mem_space;
                new_mem_space += (r_v1 + imm);
                break;
            case SLTI:
                ALU_out = r_v1 < sext;
                break;
            case SLTIU:
                ALU_out = (uint32_t) (r_v1) < (uint32_t) (sext);
                break;
            case XORI:
                cal_tot ++;
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
                ALU_out = (uint32_t) (r_v1) >> (uint32_t) (imm);
                break;
            case SRAI:
                ALU_out = r_v1 >> imm;
                break;
            case JALR:
                ALU_out = npc;
                pc = (r_v1 + sext) & (-2);
                clear();
                break;
            case BEQ:
                jump_tot ++;
                if (r_v1 == r_v2){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case BNE:
                if (r_v1 != r_v2){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case BLT:
                jump_tot ++;
                if (r_v1 < r_v2){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case BGE:
                if (r_v1 >= r_v2){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case BLTU:
                if ((uint32_t) (r_v1) < (uint32_t) (r_v2)){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case BGEU:
                if ((uint32_t) (r_v1) >= (uint32_t) (r_v2)){
                    update(npc - 4, 1);
                    npc = npc + sext - 4;
                }
                else update(npc - 4, 0);
                total ++;
                if (npc == branch) correct ++;
                else clear(), pc = npc;
                break;
            case LUI:
                ALU_out = sext;
                break;
            case AUIPC:
                ALU_out = npc + sext - 4;
                break;
            case JAL:
                jump_tot ++;
                ALU_out = npc;
                pc = npc + sext - 4;
                clear();
                break;
            case ADD:
                cal_tot ++;
                ALU_out = r_v1 + r_v2;
                break;
            case SUB:
                cal_tot ++;
                ALU_out = r_v1 - r_v2;
                break;
            case TIMES:
                cal_tot ++;
                ALU_out = r_v1 * r_v2;
                break;
            case DIVIDE:
                cal_tot ++;
                ALU_out = r_v1 / r_v2;
                break;
            case MOD:
                cal_tot ++;
                ALU_out = r_v1 % r_v2;
                break;
            case SLL:
                ALU_out = r_v1 << (r_v2 & 0x1f);
                break;
            case SLT:
                cal_tot ++;
                ALU_out = r_v1 < r_v2;
                break;
            case SLTU:
                ALU_out = (uint32_t) (r_v1) < (uint32_t) (r_v2);
                break;
            case XOR:
                ALU_out = r_v1 ^ r_v2;
                break;
            case SRL:
                ALU_out = (uint32_t) (r_v1) >> (uint32_t) ((r_v2 & 0x1f));
                break;
            case SRA:
                ALU_out = r_v1 >> (r_v2 & 0x1f);
                break;
            case OR:
                cal_tot ++;
                ALU_out = r_v1 | r_v2;
                break;
            case AND:
                cal_tot ++;
                ALU_out = r_v1 & r_v2;
                break;
        }
        EX_MEM.op_code = opcode;
        EX_MEM.rd = rd;
        EX_MEM.r_v2 = r_v2;
        EX_MEM.rs2 = rs2;
        EX_MEM.ALU_out = ALU_out;
        EX_MEM.state = TO_DO;
        if (opcode == SHUT) EX_state = DONE;
        else EX_state = VACANT;
    }
}

void MEM(){
     if (MEM_state == VACANT && EX_MEM.state == EMPTY) return;
     if (MEM_state == VACANT){
         EX_MEM.state = EMPTY;
         MEM_ = EX_MEM;
         switch  (EX_MEM.op_code){
         case LB: case LH: case LW: case LBU: case LHU: case SB: case SH: case SW: case SHUT:
                 break;
             default:
                 MEM_WB.op_code = MEM_.op_code;
                 MEM_WB.output = MEM_.ALU_out;
                 MEM_WB.rd = MEM_.rd;
                 MEM_state = VACANT;
                 MEM_WB.state = TO_DO;
                 return;
         }
     }
     else if (MEM_state == DONE) return;
     mem_cnt ++;
     if (mem_cnt % 3){
         MEM_state = WORKING;
         return;
     }
    int ALU_out = MEM_.ALU_out, r_v2 = MEM_.r_v2;
    mem_tot ++;
    switch (MEM_.op_code){
        case LB:
            int8_t  tmp_LB;
            memcpy(&tmp_LB, memory + ALU_out, 1);
            MEM_WB.output = tmp_LB;
            break;
        case LH:
            int16_t  tmp_LH;
            memcpy(&tmp_LH, memory + ALU_out, 2);
            MEM_WB.output = tmp_LH;
            break;
        case LW:
            int32_t  tmp_LW;
            memcpy(&tmp_LW, memory + ALU_out, 4);
            MEM_WB.output = tmp_LW;
            break;
        case LBU:
            uint8_t  tmp_LBU;
            memcpy(&tmp_LBU, memory + ALU_out, 1);
            MEM_WB.output = tmp_LBU;
            break;
        case LHU:
            uint16_t  tmp_LHU;
            memcpy(&tmp_LHU, memory + ALU_out, 2);
            MEM_WB.output = tmp_LHU;
        case SB:
            memcpy(memory + ALU_out, &r_v2, 1);
            break;
        case SH:
            memcpy(memory + ALU_out, &r_v2, 2);
            break;
        case SW:
            memcpy(memory + ALU_out, &r_v2, 4);
//            if (ALU_out == 65556 && !(r_v2 % 5000)) printf("STORE %d on tot\n", r_v2);
            break;
        default:
            MEM_WB.output = ALU_out;
    }
    MEM_WB.rd = MEM_.rd;
    MEM_WB.op_code = MEM_.op_code;
    if (MEM_WB.op_code == SHUT) MEM_state = DONE;
    else MEM_state = VACANT;
    MEM_WB.state = TO_DO;
}

void WB(){
    if (WB_state == DONE) return;
    else if (MEM_WB.state == EMPTY){
        WB_state = VACANT;
        return;
    } else {
        MEM_WB.state = EMPTY;
        if (MEM_WB.op_code == SHUT) WB_state = DONE;
        else {
            switch (MEM_WB.op_code) {
                case SB: case SH: case SW: case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                    break;
                default:
                    reg[MEM_WB.rd] = MEM_WB.output;
            }
            WB_state = VACANT;
        }
    }
}

bool FORWARD(){
    if (IF_state == DONE && ID_state == DONE && EX_state == DONE && MEM_state == DONE && WB_state == DONE) return 0;
    if (ID_EX.state == TO_DO) {
        switch (ID_EX.op_code) {
        case LB: case LH: case LW: case LBU: case LHU: case ADDI: case MALLOC: case SB: case SH: case SW:
            case XORI: case ORI: case ANDI: case SLLI: case SRLI: case SRAI: case SLTI: case SLTIU: case JALR:
                DETECT(ID_EX.rs1);
                break;
                case ADD: case SUB: case TIMES: case DIVIDE: case MOD: case XOR: case OR: case AND: case SLL: case SRL: case SRA: case SLT: case SLTU:
            case BEQ: case BLT: case BLTU: case BGEU: case BGE: case BNE:
                DETECT(ID_EX.rs1, ID_EX.rs2);
                break;
        }
    }
    //printf("%08X, IF_ID: %s, ID: %s, ID_EX: %s, MEM: %s\n", IF_ID.code, state_str[IF_ID.state], state_str[ID_state], state_str[ID_EX.state], state_str[MEM_state]);
    if (!((mem_cnt + 1) % 3) && MEM_WB.state == EMPTY) MEM_.r_v2 = ((MEM_.rs2 == 0) ? 0 : reg[MEM_.rs2]);
    return 1;
}
#endif //RICV_PIPELINE_H
