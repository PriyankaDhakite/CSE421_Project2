#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "userprog/process.h"

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
      verify((void *) *(arg + 1), 0);

      if (*arg == 1) {
        putbuf((const void *) *(arg + 1), (unsigned) *(arg + 2));
        f->eax = (unsigned) *(arg + 2);
      } else {
        struct fd *fd_write = thread_get_file(*arg);
        if (fd_write == NULL) {
          f->eax = 0;
        } else {
          f->eax = file_write(fd_write->file, (arg + 1), *(arg + 2));
        }
      }

      break;
    case SYS_EXIT:
      verify(f->esp, 1);

      thread_current()->exit_code = *arg;
      thread_exit();

      break;
    case SYS_CREATE:
      verify(f->esp, 2);
      verify((void *) *arg, 0);

      if ((const char*) *(arg) == NULL) {
        thread_current()->exit_code = -1;
        thread_exit();
      }
      f->eax = filesys_create((const char *) *(arg), (unsigned) *(arg + 1));

      break;
    case SYS_OPEN:
      verify(f->esp, 1);
      verify((void *) *arg, 0);

      struct file *file_o = filesys_open((const char *) *(arg));
      if (file_o == NULL) {
        f->eax = -1;
      } else {
        struct fd *new_fd = malloc(sizeof(struct fd));
        thread_current()->fd_count += 1;
        new_fd->fd_num = thread_current()->fd_count;
        new_fd->file = file_o;
        list_push_back(&thread_current()->fd_list, &new_fd->fd_elem);
        f->eax = thread_current()->fd_count;
      }

      break;
    case SYS_CLOSE:
      verify(f->esp, 1);

      struct fd *fd_close = thread_get_file(*arg);
      if (fd_close != NULL) {
        file_close(fd_close->file);
        list_remove(&fd_close->fd_elem);
      }

      break;
    case SYS_FILESIZE:
      verify(f->esp, 1);

      struct fd *fd_filesize = thread_get_file(*arg);
      if (fd_filesize == NULL) {
        f->eax = 0;
      } else {
        f->eax = file_length(fd_filesize->file);
      }

      break;
    case SYS_EXEC:
     verify(f->esp, 1);
     verify((void *) *arg, 0);

     tid_t pid = process_execute((const char *) *arg);
     while (thread_current()->load == 0) { barrier(); }
     f->eax = thread_current()->load;

     break;
    case SYS_READ:
      verify(f->esp, 3);
      verify((void *) *(arg + 1), 0);

      if (*arg > 0) {
        struct fd *fd_read = thread_get_file(*arg + 1);
        if (fd_read == NULL) {
          f->eax = -1;
        } else {
          f->eax = file_read(fd_read->file, (arg + 1), *(arg + 2));
        }
      }

      break;
    case SYS_TELL:
      verify(f->esp, 1);

      struct fd *fd_struct;
      struct list_elem *e;
      int status = -1;
      e = list_tail(&thread_current()->fd_list);
      fd_struct = list_entry(e, struct fd, fd_elem);
      if (fd_struct != NULL) {
        status = file_tell(fd_struct->file);
      }
      f->eax = status;

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
