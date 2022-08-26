#!/bin/bash

module_name="chardev-test-module.ko"
log_tag="my_chardev"


sudo rmmod ${module_name}

sudo insmod ${module_name}

sudo rmmod ${module_name}

#show log
dmesg | grep ${log_tag}
