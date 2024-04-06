#include <libmem/libmem.h>
#include "minunit.h"
#include "helpers.h"

static int (*target_function_trampoline)(char *, int);
static lm_address_t hook_address;

int target_function(char *mystr, int mynum)
{
	if (!mystr || mynum < 0)
		return 0;

	printf("<STRING: %s> <NUMBER: %d> ", mystr, mynum);

	return 1;
}

int hk_target_function(char *mystr, int mynum)
{
	int orig_ret;

	mystr = "Hooked Target Function";
	mynum = 1337;

	orig_ret = ((int (*)(char *, int))target_function_trampoline)(mystr, mynum);
	printf("<ORIG RET: %d> ", orig_ret);

	return mynum;
}

char *test_LM_HookCode(struct hook_args *arg)
{
	lm_inst_t inst;

	hook_address = (lm_address_t)target_function;
	/*
	 * NOTE: this resolves dummy functions for Windows.
	 * On Windows (x86), the function 'target_function' is
	 * just a gateway, having the instruction `jmp <real_target_function>`.
	 * This invalidates the gateway, which will just jump to the real function.
	 * The following code will resolve that.
	 */
	LM_Disassemble((lm_address_t)target_function, &inst);
	if (!strcmp(inst.mnemonic, "jmp")) {
		hook_address += *(lm_address_t *)&inst.bytes[1] + inst.size; /* Calculate real address from 'jmp' offset */
	}
	
	arg->hksize = LM_HookCode(hook_address, (lm_address_t)hk_target_function, &arg->trampoline);
	mu_assert("failed to hook target function", arg->hksize > 0);
	target_function_trampoline = (int (*)(char *, int))arg->trampoline;
	mu_assert("target function not hooked", target_function(NULL, -1) == 1337);
	mu_assert("function attempted to run with bad arguments (invalid from)", LM_HookCode(LM_ADDRESS_BAD, (lm_address_t)hk_target_function, LM_NULLPTR) == 0);
	mu_assert("function attempted to run with bad arguments (invalid to)", LM_HookCode(hook_address, LM_ADDRESS_BAD, LM_NULLPTR) == 0);
	
	return NULL;
}

char *test_LM_UnhookCode(struct hook_args *arg)
{
	mu_assert("failed to unhook target function", LM_UnhookCode(hook_address, arg->trampoline, arg->hksize) == LM_TRUE);
	mu_assert("target function is not unhooked properly", target_function("hello world", 123) == 1);
	mu_assert("function attempted to run with bad arguments (invalid from)", LM_UnhookCode(LM_ADDRESS_BAD, arg->trampoline, arg->hksize) == LM_FALSE);
	mu_assert("function attempted to run with bad arguments (invalid trampoline)", LM_UnhookCode((lm_address_t)target_function, LM_ADDRESS_BAD, arg->hksize) == LM_FALSE);
	mu_assert("function attempted to run with bad arguments (invalid size)", LM_UnhookCode((lm_address_t)target_function, arg->trampoline, 0) == LM_FALSE);

	return NULL;
}
