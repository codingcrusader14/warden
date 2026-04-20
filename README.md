# WardenOS
Warden is a UNIX inspired operating system that is specficially target for the cortex-a57 chip on the Armv8-a architecture. To emulate the chip I have elected to use Qemu which does a wonderful job that dosen't require up front hardware costs. This OS is far from finished and there bugs that exist in weird edge cases that I still yet to attack. Eventually I will add more features as Warden only supports super basic user programs that exist within the shell. 

## Why build an OS in 2026? 
The reason I first set out to build an OS was because I was simply curious on what went on under the hood of the regular applications I used day to day. I asked myself what other way than starting with the foundation. I didn't know much about operating systems and how much work went into even building a minimal such as Warden. Though, overtime I started to break down the complexity into manageable pieces and now I'm at a point where I benchmarked my kernel on basic kernel tasks, such as context switching, system calls, and file I/O.

## Steps to build 
To download and build through Qemu, follow the steps below.
```
git clone https://github.com/codingcrusader14/warden.git 
make clean 
make all
make qemu
```
It is assumed that Qemu and an Arm cross compiler is installed on your system. 

