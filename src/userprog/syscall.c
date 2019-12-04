#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{

  int *arg = (int *) f->esp + 1;

  switch (*(arg - 1)) {

    case SYS_WRITE:

      if (*arg == 1) {
        putbuf((const void*) *(arg + 1), (unsigned) *(arg + 2));
        f->eax = (unsigned) *(arg + 2);
      }

      break;
    case SYS_EXIT:

      thread_current()->exit_code = *arg;
      thread_exit();

      break;
    default:

      thread_exit();

      break;
  }

}
