#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/net.h>
#include <linux/dirent.h>
#include <linux/mutex.h>

#define BUFFER_SIZE 128

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kaveev");
MODULE_DESCRIPTION("OS Lab2 from Roman Kaveev");
MODULE_VERSION("1.0");

struct mutex struct_mutex;
static struct dentry *kmod_dir;
static struct dentry *args_file;
static struct socket *sock = NULL;
static struct task_struct* task = NULL;

static void print_socket(struct seq_file *file, struct socket *sock);
static void print_task(struct seq_file *file, struct task_struct *task);
static int print_struct(struct seq_file *file, void *data);

static ssize_t kmod_write(struct file *file, const char __user *buffer, size_t length, loff_t *ptr_offset) {
  printk(KERN_INFO "kaveev: get arguments\n");
  char user_data[BUFFER_SIZE];
  int pid, fd;
  copy_from_user(user_data, buffer, length);
  sscanf(user_data, "pid: %d, fd: %d", &pid, &fd);
  task = get_pid_task(find_get_pid(pid),PIDTYPE_PID);
  if (fd != -1) {
    int err = 0;
    sock = sockfd_lookup(fd, &err);
  }
  single_open(file, print_struct, NULL);
  return strlen(user_data);
}

static struct file_operations fops = {
  .read = seq_read,
  .write = kmod_write,
};

static int __init mod_init(void) {
  printk(KERN_INFO "kaveev: module loading.\n");
  mutex_init(&struct_mutex);
  kmod_dir = debugfs_create_dir("laba2", NULL);
  args_file = debugfs_create_file("result", 0777, kmod_dir, NULL, &fops);
  return 0;
}

static void __exit mod_exit(void) {
  debugfs_remove_recursive(kmod_dir);
  printk(KERN_INFO "kaveev: module unloaded\n");
}

static int print_struct(struct seq_file *file, void *data) {
  print_socket(file, sock);
  print_task(file, task);
  return 0;
}

static void print_task(struct seq_file *file, struct task_struct *task) {
  mutex_lock(&struct_mutex);
  if (task == NULL) {
    seq_printf(file, "Can't find task_struct with this pid\n");
  } else {
    seq_printf(file, "task_struct structure: {\n");
    seq_printf(file, "  pid: %u,\n", task->pid);	
    seq_printf(file, "  state: %u,\n", task->__state);
    seq_printf(file, "  tgid: %u,\n", task->tgid);
    seq_printf(file, "  parent: %u,\n", task->parent);
    seq_printf(file, "}\n");
  }
  mutex_unlock(&struct_mutex);
}

static void print_socket(struct seq_file *file, struct socket *sock) {
  mutex_lock(&struct_mutex);
  if (sock == NULL) {
    seq_printf(file, "Can't find socket with this params\n"); 
  } else {
    seq_printf(file, "socket structure: {\n"); 
    seq_printf(file, "  socket state: %u,\n", sock->state);
    seq_printf(file, "  socket type: %u,\n", sock->type);
    seq_printf(file, "  socket flags: %u,\n", sock->flags);
  }
  mutex_unlock(&struct_mutex);
}

module_init(mod_init);
module_exit(mod_exit);
