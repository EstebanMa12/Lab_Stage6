  asmlinkage long avanzatech(int number, char __user *buffer, size_t length, char __user *dest_buffer, char __user *username, size_t dest_len);

  asmlinkage long avanzatech (
    int number, 
    char __user *buffer, 
    size_t length, 
    char __user *dest_buffer, 
    char __user *username, 
    size_t dest_len
  ) {
    char *kernel_buffer;
    char *kernel_dest_buffer;
    char *kernel_username;
    long result;

    if (length < 0 || dest_len)return -EINVAL;

    kernel_buffer = kmalloc(length, GFP_KERNEL);
    kernel_dest_buffer = kmalloc(dest_len, GFP_KERNEL);
    kernel_username = kmalloc(dest_len, GFP_KERNEL);

    if (!kernel_buffer || !kernel_dest_buffer || !kernel_username) {
      result = -ENOMEN;
      goto free_and_exit;
    }

    if (copy_from_user(kernel_buffer, buffer, length)) {
      result = -EFAULT;
      goto free_and_exit;
    }

    if (copy_from_user(kernel_dest_buffer, dest_buffer, dest_len)) {
      result = -EFAULT;
      goto free_and_exit;
    }
    if (copy_from_user(kernel_username, username, dest_len)) {
      result = -EFAULT;
      goto free_and_exit;
    }

    result = number*number*number;

    //Construct the response message
    snprintf(kernel_dest_buffer, dest_len, "Hi %s, the cube of %d is %ld", kernel_username, number, result);

    // Copy the response back to dest_buffer
    if (copy_to_user(dest_buffer, kernel_dest_buffer, dest_len)) {
      result = -EFAULT;
      goto free_and_exit;
    }
    
    free_and_exit:
      kfree(kernel_buffer);
      kfree(kernel_dest_buffer);
      kfree(kernel_username);

  }