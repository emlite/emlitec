#include <emlite/emlite.h>

em_Val em_Val_from_bool(bool i) { return (em_Val){.h = emlite_val_make_bool(i)}; }

em_Val em_Val_from_int(int i) { return (em_Val){.h = emlite_val_make_int(i)}; }

em_Val em_Val_from_uint(unsigned int i) { return (em_Val){.h = emlite_val_make_uint(i)}; }

em_Val em_Val_from_bigint(long long i) { return (em_Val){.h = emlite_val_make_bigint(i)}; }

em_Val em_Val_from_biguint(unsigned long long i) {
    return (em_Val){.h = emlite_val_make_biguint(i)};
}

em_Val em_Val_from_double(double i) { return (em_Val){.h = emlite_val_make_double(i)}; }
em_Val em_Val_from_string(const char *s) {
    return (em_Val){.h = emlite_val_make_str(s, strlen(s))};
}

em_Val em_Val_from_string_utf16(const uint16_t *s) {
    // Calculate length of uint16_t string
    size_t len = 0;
    const uint16_t *ptr = s;
    while (ptr && *ptr != 0) { ++len; ++ptr; }
    return (em_Val){.h = emlite_val_make_str_utf16(s, len)};
}

em_Val em_Val_from_val(void *s) { return (em_Val){.h = ((em_Val *)s)->h}; }

em_Val em_Val_from_handle(Handle v) { return (em_Val){.h = v}; }

em_Val em_Val_global(const char *name) {
    return em_Val_from_handle(emlite_val_get(EMLITE_GLOBALTHIS, em_Val_from_string(name).h));
}

em_Val em_Val_global_this() { return em_Val_from_handle(EMLITE_GLOBALTHIS); }

em_Val em_Val_null() { return em_Val_from_handle(EMLITE_NULL); }

em_Val em_Val_undefined() { return em_Val_from_handle(EMLITE_UNDEFINED); }

em_Val em_Val_object() { return em_Val_from_handle(emlite_val_new_object()); }

em_Val em_Val_array() { return em_Val_from_handle(emlite_val_new_array()); }

em_Val em_Val_make_fn(Callback f, Handle data) {
    uint32_t fidx = (uint32_t)f;
    return em_Val_from_handle(emlite_val_make_callback(fidx, data));
}

void em_Val_delete(em_Val v) { emlite_val_dec_ref(v.h); }

void em_Val_throw(em_Val v) { emlite_val_throw(v.h); }

Handle em_Val_as_handle(em_Val self) { return self.h; }

em_Val em_Val_get(em_Val self, em_Val prop) {
    return em_Val_from_handle(emlite_val_get(self.h, prop.h));
}

void em_Val_set(em_Val self, em_Val idx, em_Val val) { emlite_val_set(self.h, idx.h, val.h); }

bool em_Val_has(em_Val self, em_Val val) { return emlite_val_has(self.h, val.h); }

bool em_Val_has_own_property(em_Val self, const char *prop) {
    return emlite_val_obj_has_own_prop(self.h, prop, strlen(prop));
}

char *em_Val_typeof(em_Val self) { return emlite_val_typeof(self.h); }

em_Val em_Val_at(em_Val self, em_Val elem) {
    return em_Val_from_handle(emlite_val_get(self.h, elem.h));
}

em_Val em_Val_await(em_Val self) {
    return emlite_eval_v(
        "(async() => { let obj = "
        "EMLITE_VALMAP.toValue(%d); let "
        "ret = await obj; "
        "return EMLITE_VALMAP.toHandle(ret); })()",
        self.h
    );
}

bool em_Val_is_bool(em_Val self) { return emlite_val_is_bool(self.h); }

bool em_Val_is_number(em_Val self) { return emlite_val_is_number(self.h); }

bool em_Val_is_string(em_Val self) { return emlite_val_is_string(self.h); }

bool em_Val_is_error(em_Val self) { return em_Val_instanceof(self, em_Val_global("Error")); }

bool em_Val_is_function(em_Val self) { return em_Val_instanceof(self, em_Val_global("Function")); }

bool em_Val_is_undefined(em_Val self) { return self.h == EMLITE_UNDEFINED; }

bool em_Val_is_null(em_Val self) { return self.h == EMLITE_NULL; }

bool em_Val_instanceof(em_Val self, em_Val v) { return emlite_val_instanceof(self.h, v.h); }

bool em_Val_not(em_Val self) { return emlite_val_not(self.h); }

bool em_Val_seq(em_Val self, em_Val other) { return emlite_val_strictly_equals(self.h, other.h); }

bool em_Val_eq(em_Val self, em_Val other) { return emlite_val_equals(self.h, other.h); }

bool em_Val_neq(em_Val self, em_Val other) { return !emlite_val_strictly_equals(self.h, other.h); }

bool em_Val_gt(em_Val self, em_Val other) { return emlite_val_gt(self.h, other.h); }

bool em_Val_gte(em_Val self, em_Val other) { return emlite_val_gte(self.h, other.h); }

bool em_Val_lt(em_Val self, em_Val other) { return emlite_val_lt(self.h, other.h); }

bool em_Val_lte(em_Val self, em_Val other) { return emlite_val_lte(self.h, other.h); }

int em_Val_as_int(em_Val self) { return emlite_val_get_value_int(self.h); }

unsigned int em_Val_as_uint(em_Val self) { return emlite_val_get_value_uint(self.h); }

long long em_Val_as_bigint(em_Val self) { return emlite_val_get_value_bigint(self.h); }

unsigned long long em_Val_as_biguint(em_Val self) { return emlite_val_get_value_biguint(self.h); }

bool em_Val_as_bool(em_Val self) { return !emlite_val_not(self.h); }

double em_Val_as_double(em_Val self) { return emlite_val_get_value_double(self.h); }

char *em_Val_as_string(em_Val self) { return emlite_val_get_value_string(self.h); }

uint16_t *em_Val_as_string_utf16(em_Val self) { return emlite_val_get_value_string_utf16(self.h); }

em_Val em_Val_as_val(em_Val self) { return em_Val_from_val(&self); }

em_Val em_Val_call_v(em_Val self, const char *method, int n, va_list ap) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_copy(args, ap);
    for (int i = 0; i < n; ++i) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    em_Val ret = em_Val_from_handle(emlite_val_obj_call(self.h, method, (int)strlen(method), arr));
    emlite_val_dec_ref(arr);
    return ret;
}

em_Val em_Val_call_(em_Val self, const char *method, int n, ...) {
    va_list ap;
    va_start(ap, n);
    em_Val ret = em_Val_call_v(self, method, n, ap);
    va_end(ap);
    return ret;
}

em_Val em_Val_new_v(em_Val self, int n, va_list ap) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_copy(args, ap);
    for (int i = 0; i < n; ++i) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    em_Val ret = em_Val_from_handle(emlite_val_construct_new(self.h, arr));
    emlite_val_dec_ref(arr);
    return ret;
}

em_Val em_Val_new_(em_Val self, int n, ...) {
    va_list ap;
    va_start(ap, n);
    em_Val ret = em_Val_new_v(self, n, ap);
    va_end(ap);
    return ret;
}

em_Val em_Val_invoke_v(em_Val self, int n, va_list ap) {
    Handle arr = emlite_val_new_array();
    va_list args;
    va_copy(args, ap);
    for (int i = 0; i < n; ++i) {
        em_Val c = va_arg(args, em_Val);
        emlite_val_push(arr, em_Val_as_handle(c));
    }
    va_end(args);
    em_Val ret = em_Val_from_handle(emlite_val_func_call(self.h, arr));
    emlite_val_dec_ref(arr);
    return ret;
}

em_Val em_Val_invoke_(em_Val self, int n, ...) {
    va_list ap;
    va_start(ap, n);
    em_Val ret = em_Val_invoke_v(self, n, ap);
    va_end(ap);
    return ret;
}

em_Val emlite_eval(const char *src) {
    em_Val eval   = em_Val_global("eval");
    em_Val js_src = em_Val_from_string(src);
    em_Val ret    = em_Val_invoke(eval, js_src);
    em_Val_delete(js_src);
    em_Val_delete(eval);
    return ret;
}

em_Val emlite_eval_v(const char *src, ...) {
    va_list args;
    va_start(args, src);
    va_list args_len;
    va_copy(args_len, args);
    size_t len = vsnprintf(NULL, 0, src, args_len);
    va_end(args_len);
    char *ptr = (char *)malloc(len + 1);
    if (!ptr) {
        va_end(args);
        return em_Val_null();
    }
    (void)vsnprintf(ptr, len + 1, src, args);
    va_end(args);
    em_Val ret = emlite_eval(ptr);
    free(ptr);
    return ret;
}