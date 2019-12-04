#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
void verify (const void *pointer, int argc);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{

  int *arg = (int *) f->esp + 1;
  verify(arg, 0);

  switch (*(arg - 1)) {

    case SYS_WRITE:
      verify(f->esp, 3);

      if (*arg == 1) {
        putbuf((const void*) *(arg + 1), (unsigned) *(arg + 2));
        f->eax = (unsigned) *(arg + 2);
      }

      break;
    case SYS_EXIT:
      verify(f->esp, 1);

      thread_current()->exit_code = *arg;
      thread_exit();

      break;
    default:

      thread_exit();

      break;
  }

}

void
verify (const void *pointer, int argc)
{
  for (int i = 0; i <= argc; i++) {
    if (pointer + i == NULL || (pointer + i) >= PHYS_BASE || !pagedir_get_page(thread_current()->pagedir, pointer + i)) {
      thread_current()->exit_code = -1;
      thread_exit();
    }
  }
}
