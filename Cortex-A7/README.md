# ieBPF Cortex-A7

首先需要在 SD 卡上安装 Ubuntu 18.04，参考华清远见提供的 SD 卡上的镜像烧写教程即可。他们提供的镜像默认的用户是 `linux`，密码是`1`。

安好后，在实验上会有一些注意点，可以参考我之前记录的相关笔记，我把实践中的问题放在最上面一小节了。

[STM32MP157A和IPCC](https://forsworns.github.io/zh/blogs/20210223/)

## 测试样例 alu_arith

该目录下有三个文件

- `alu_arith.bpf` 是一个 `elf`文件，理论上是由 clang 由 C 代码生成的，但是这里是用 uBPF 里提供的汇编器程序，把下面那个汇编转到 BPF 的指令集上的机器码
- `alu_arith.bpfasm` 是 BPF 指令集上的汇编代码
- `alu_arith.data` 是说明那个汇编，带了些注释

现在我们应该是要向 `/dev/RPMsg0` 或者 `/dev/RPMsg1` 发送这两个文件中的内容，让 Cortex-M4 去执行？我在本地测试这两个样例，在 uBPF 上是都可以用的。

如果测试成功，结果应该是 `0x2a`