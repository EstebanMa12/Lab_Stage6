
#include <linux/kernel.h>
#include <errno.h>

SYSCALL_DEFINE5(avanzatech, int, number, char __user*, name, size_t, name_length, char __user*,dest_buffer, size_t, dest_len){
  long result;
  int num;
  char *kernel_buffer;
  char *kernel_dest_buffer;

  printk(KERN_INFO "[avanzatech]: syscall invoked. Number %d  and Name: %s\n",number, name );



  //Validate the size of buffers, checking for buffer overflows or underflows.
  if (number < -1290 || number > 1290 || name_length<=0){
    printk(KERN_ERR "Error: Invalid parameters\n");
    result = -EINVAL;
    return -1;
  } 

  // Allocation of memory in the kernel space for temporary data
  kernel_buffer = kmalloc(name_length, GFP_KERNEL);
  if (!kernel_buffer){
    printk("Error: Unable to allocate buffer\n");
    result = -ENOMEM;
    return -1;
  }



  kernel_dest_buffer = kmalloc(dest_len, GFP_KERNEL);
  if (!kernel_dest_buffer){
    printk("Error: Unable to allocate buffer\n");
    result = -ENOMEM;
    return result;
  }
  printk("[avanzatech]: Allocation of memory done\n");  //Copy data from user space to kernel space using 'copy_from_user'
  if (copy_from_user(kernel_buffer, name, name_length) ){
    result = -EFAULT;
    kfree(kernel_buffer);
    kfree(kernel_dest_buffer);
  }



  //Implement logic to cube the number.
  num = number * number * number; 

  printk("[avanzatech]: The number is %d", num);


  //Construct a response message like "Hi [username], the cube of [number] is [result]".
  result = snprintf(kernel_dest_buffer, dest_len, "Hi %s, the cube of %d is %d\n", kernel_buffer, number, num);

  printk("[avanzatech]: The result is %ld", result);

  //Handle errors for invalid inputs, buffer sizes, and read/write permissions.
  if (result >= dest_len) {
    printk("Error: Insufficient buffer space for the response message\n");
    result = -EINVAL;
    kfree(kernel_buffer);
    kfree(kernel_dest_buffer);
    return result;
  }

  printk("[avanzatech]: Handle error for the result done, start with the copy response for user");



  //Copy the response back to `dest_buffer` using `copy_to_user`.
  if(copy_to_user(dest_buffer,kernel_dest_buffer, dest_len)){
    result = -EFAULT;
    kfree(kernel_buffer);
    kfree(kernel_dest_buffer);
    return result;
  }

  printk("[avanzatech]: Copy the response back to dest_buffer done");

  printk(KERN_INFO "El resultado es: %ld\n", result);
  kfree(kernel_buffer);
  kfree(kernel_dest_buffer);
  return result;
} 