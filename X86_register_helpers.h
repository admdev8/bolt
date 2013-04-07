void X86_register_get_value (X86_register r, const CONTEXT *ctx, s_Value* out);
uint64_t X86_register_get_value_as_u64 (X86_register r, const CONTEXT *ctx);
void X86_register_set_value (X86_register r, CONTEXT *ctx, s_Value *val);
