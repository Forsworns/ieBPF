# ieBPF

![C](https://img.shields.io/badge/language-C-orange.svg)
![eBPF](https://img.shields.io/badge/IoT-eBPF-blue.svg)
![STM32MP157A](https://img.shields.io/badge/board-STM32MP157A-yellow.svg)

## Environment

The testing environments are

- Ubuntu 18.04 on Cortex-A7, the related codes and scripts are under `{$this_repo}/Cortex-A7`.  In fact, the weston-openstlinux is enough, but considering the extensibility in the following experiments, it is still recommended to install Ubuntu on Cortex-A7.
- TencentOS-tiny on Cortex-M4, the related STM32CubeIDE project is under `{$this_repo}/Cortex-A7`. I haven't found the suitable application cases, so the feature of the OS is nearly not utilized. 

## Testing

```bash
# Build the project under `/Cortex-M4`
# Load the compiled `elf` file to the Cortex-M4 through 
# STM32CubeIDE on host machine or 
# STM32CubeIDE generated scripts on the ubuntu
# Then run the testcases in `/Cortex-A7` 
```

## Todo
- [x] Migrate Tencent-OS tiny to board 
- [x] Migrate the uBPF  
- [ ] Test M4 on both assembler and elf files (have placed in the `/Cortex-M7`)
- [ ] Find suitable application scenes and conduct related experiments
- [ ] Try to provide  JIT on ARMv7-M for Cortex-M4 (refer to the JIT implementation of uBPF)
- [ ] Performance evaluation 

## Related Notes
Please entering the subdirs `Cortex-M4` and  `/Cortex-M7` for more details on configurations and testing.

[STM32MP157A和IPCC](https://forsworns.github.io/zh/blogs/20210223/)

[BPF和eBPF](https://forsworns.github.io/zh/blogs/20210311/)

[BPF 和 eBPF （续：实现相关）](https://forsworns.github.io/zh/blogs/20210329/)

## Related Resources

[iovisor/ubpf: Userspace eBPF VM ](https://github.com/iovisor/ubpf)

[OpenAMP/open-amp](https://github.com/OpenAMP/open-amp)

