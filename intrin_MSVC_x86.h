#pragma once

void __fastcall intrin_SHL (IN tetra value, IN uint8_t shift_value, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_SHR (IN tetra value, IN uint8_t shift_value, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_SAR (IN tetra value, IN uint8_t shift_value, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_ADD (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_ADC (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_SUB (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_SBB (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_XOR (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_XOR_addr (IN tetra *address_of_op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_OR (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_AND (IN tetra op1, IN tetra op2, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_NOT (IN tetra op1, OUT tetra* result, IN OUT tetra* flags);
void __fastcall intrin_NEG (IN tetra op1, OUT tetra* result, IN OUT tetra* flags);
