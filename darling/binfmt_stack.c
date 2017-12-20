/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2017 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#if defined(GEN_64BIT)
#define FUNCTION_NAME setup_stack64
#define user_long_t unsigned long
#elif defined(GEN_32BIT)
#define FUNCTION_NAME setup_stack32
#define user_long_t unsigned int
#else
#error See above
#endif

int FUNCTION_NAME(struct linux_binprm* bprm, struct load_results* lr)
{
	int err = 0;
	// unsigned char rand_bytes[16];
	char *executable_path, *executable_buf;
	user_long_t __user* argv;
	user_long_t __user* envp;
	user_long_t __user* applep;
	user_long_t __user* sp;
	char __user* exepath_user;
	size_t exepath_len;

	// Produce executable_path=... for applep
	executable_buf = kmalloc(4096, GFP_KERNEL);
	if (!executable_buf)
		return -ENOMEM;

	executable_path = d_path(&bprm->file->f_path, executable_buf, 4095);
	if (IS_ERR(executable_path))
	{
		err = -ENAMETOOLONG;
		goto out;
	}

	// printk(KERN_NOTICE "Stack top: %p\n", bprm->p);
	exepath_len = strlen(executable_path);
	sp = (user_long_t*) (bprm->p & ~(sizeof(user_long_t)-1));
	sp -= bprm->argc + bprm->envc + 6 + exepath_len;
	exepath_user = (char __user*) bprm->p - exepath_len - sizeof(EXECUTABLE_PATH);

	if (!find_extend_vma(current->mm, (unsigned long) sp))
	{
		err = -EFAULT;
		goto out;
	}

	if (copy_to_user(exepath_user, EXECUTABLE_PATH, sizeof(EXECUTABLE_PATH)-1))
	{
		err = -EFAULT;
		goto out;
	}
	if (copy_to_user(exepath_user + sizeof(EXECUTABLE_PATH)-1, executable_path, exepath_len + 1))
	{
		err = -EFAULT;
		goto out;
	}

	kfree(executable_buf);
	executable_buf = NULL;
	bprm->p = (unsigned long) sp;

	// XXX: skip this for static executables, but we don't support them anyway...
	if (__put_user((user_long_t) lr->mh, sp++))
	{
		err = -EFAULT;
		goto out;
	}
	if (__put_user((user_long_t) bprm->argc, sp++))
	{
		err = -EFAULT;
		goto out;
	}

	unsigned long p = current->mm->arg_start;
	int argc = bprm->argc;

	argv = sp;
	envp = argv + argc + 1;
	applep = envp + bprm->envc + 1;

	// Fill in argv pointers
	while (argc--)
	{
		if (__put_user((user_long_t) p, argv++))
		{
			err = -EFAULT;
			goto out;
		}

		size_t len = strnlen_user((void __user*) p, MAX_ARG_STRLEN);
		if (!len || len > MAX_ARG_STRLEN)
		{
			err = -EINVAL;
			goto out;
		}

		p += len;
	}
	if (__put_user((user_long_t) 0, argv++))
	{
		err = -EFAULT;
		goto out;
	}
	current->mm->arg_end = current->mm->env_start = p;

	// Fill in envp pointers
	int envc = bprm->envc;
	while (envc--)
	{
		if (__put_user((user_long_t) p, envp++))
		{
			err = -EFAULT;
			goto out;
		}

		size_t len = strnlen_user((void __user*) p, MAX_ARG_STRLEN);
		if (!len || len > MAX_ARG_STRLEN)
		{
			err = -EINVAL;
			goto out;
		}

		p += len;
	}
	if (__put_user((user_long_t) 0, envp++))
	{
		err = -EFAULT;
		goto out;
	}
	current->mm->env_end = p;

	if (__put_user((user_long_t)(unsigned long) exepath_user, applep++))
	{
		err = -EFAULT;
		goto out;
	}
	if (__put_user((user_long_t) 0, applep++))
	{
		err = -EFAULT;
		goto out;
	}

	// get_random_bytes(rand_bytes, sizeof(rand_bytes));

	// TODO: produce stack_guard, e.g. stack_guard=0xcdd5c48c061b00fd (must contain 00 somewhere!)
	// TODO: produce malloc_entropy, e.g. malloc_entropy=0x9536cc569d9595cf,0x831942e402da316b
	// TODO: produce main_stack?
	
out:
	if (executable_buf)
		kfree(executable_buf);
	return err;
}

#undef FUNCTION_NAME
#undef user_long_t
