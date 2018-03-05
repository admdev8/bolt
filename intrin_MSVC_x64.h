#pragma once

void intrin_SHL (IN octa value, IN uint8_t shift_value, OUT octa* result, IN OUT octa* flags);
void intrin_SHR (IN octa value, IN uint8_t shift_value, OUT octa* result, IN OUT octa* flags);
void intrin_SAR (IN octa value, IN uint8_t shift_value, OUT octa* result, IN OUT octa* flags);
void intrin_ADD (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_ADC (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_SUB (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_SBB (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_XOR (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_XOR_addr (IN octa *address_of_op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_OR (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_AND (IN octa op1, IN octa op2, OUT octa* result, IN OUT octa* flags);
void intrin_NOT (IN octa op1, OUT octa* result, IN OUT octa* flags);
void intrin_NEG (IN octa op1, OUT octa* result, IN OUT octa* flags);
