#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
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
        putbuf((const void *) *(arg + 1), (unsigned) *(arg + 2));
        f->eax = (unsigned) *(arg + 2);
      }

      break;
    case SYS_EXIT:
      verify(f->esp, 1);

      thread_current()->exit_code = *arg;
      thread_exit();

      break;
    case SYS_CREATE:
      verify(f->esp, 2);

      if ((const char*) *(arg) == NULL) {
        thread_current()->exit_code = -1;
        thread_exit();
      }
      f->eax = filesys_create((const char *) *(arg), (unsigned) *(arg + 1));

      break;
    case SYS_OPEN:
      verify(f->esp, 1);

      struct file *file_open = filesys_open((const char *) *(arg));
      if (f == NULL) {
        f->eax = -1;
      } else {
        struct fd *new_fd = malloc(sizeof(struct fd));
        thread_current()->fd_count += 1;
        new_fd->fd_num = thread_current()->fd_count;
        new_fd->file = file_open;
        list_push_back(&thread_current()->fd_list, &new_fd->fd_elem);
        f->eax = thread_current()->fd_count;
     }

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
    if (!is_user_vaddr(pointer + i) || !pagedir_get_page(thread_current()->pagedir, pointer + i)) {
      thread_current()->exit_code = -1;
      thread_exit();
    }
  }
}
