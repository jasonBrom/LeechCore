# LeechCore 日志化 DLL 使用说明

本文介绍如何使用带日志输出功能的 `leechcore.dll`，以及如何通过 GitHub Actions 自动构建和下载该版本以便调试 FPGA DMA 操作。

## 1. GitHub Actions 自动构建

仓库新增了工作流 `.github/workflows/build-logging.yml`，在任意分支发生 `push` 或创建 `pull request` 时都会自动触发。

工作流会在 `windows-latest` 运行器上执行以下任务：

1. 检出源码。
2. 配置 MSBuild 环境。
3. 以 `Release`、`x64` 配置编译 `LeechCore.sln`，生成带日志能力的 `leechcore.dll`。
4. 打包生成的 `files/x64/leechcore.dll`、`files/x64/lib/leechcore.lib` 与 `files/x64/lib/leechcore.pdb` 作为构建工件。

在 GitHub Actions 详情页选择对应的运行记录，展开 **Artifacts** 区域即可下载构建好的 DLL 及符号文件，方便在本地或 CI 环境中直接使用。

## 2. 日志输出总览

新的 DLL 会在加载时自动尝试创建日志文件（默认文件名为 `leechcore.log`）。日志功能可帮助分析通过 `leechcore.dll` 发起的所有读取和写入操作，特别是针对 FPGA DMA 板卡的物理内存访问。

日志内容包含：

* 读取/写入操作开始与结束的时间戳、目标物理地址范围、数据长度以及耗时（毫秒）。
* 对于 `LcReadScatter`/`LcWriteScatter` 调用，记录每个 `MEM_SCATTER` 的地址、长度、是否成功以及地址转换结果。
* 支持线程 ID 标记，便于定位多线程调用场景。

示例片段（开启详细模式时）：

```
[2024-03-01 10:22:33.512][tid:1a4] READ_SCATTER_BEGIN: device='fpga' handle=0x00000123456789AB entries=4 bytes=0x4000 translated=0
[2024-03-01 10:22:33.513][tid:1a4] READ_SCATTER_TRANSLATED: device='fpga' handle=0x00000123456789AB entries=4 bytes=0x4000 translated=1
[2024-03-01 10:22:33.514][tid:1a4]   [0] pa=0x000000100000 cb=0x1000 status=-
[2024-03-01 10:22:33.514][tid:1a4]       original_pa=0x0000000FF000
[2024-03-01 10:22:33.515][tid:1a4] READ_SCATTER_RESULT: device='fpga' handle=0x00000123456789AB entries=4 bytes=0x4000 translated=1
[2024-03-01 10:22:33.516][tid:1a4]   [0] pa=0x000000100000 cb=0x1000 status=ok
[2024-03-01 10:22:33.517][tid:1a4] READ_SCATTER_DONE: device='fpga' handle=0x00000123456789AB entries=4 bytes=0x4000 bytes=0x4000 duration=1.234ms
```

通过“translated”与“original_pa”字段可以清晰看出地址映射关系，从而判断 DMA 实际访问了哪段物理内存。

## 3. 环境变量配置

可以通过以下环境变量调整日志行为：

| 环境变量 | 说明 | 默认值 |
| --- | --- | --- |
| `LEECHCORE_LOG_DISABLE` | 设为非零时关闭日志功能。 | `0`（启用） |
| `LEECHCORE_LOG_PATH` | 指定日志文件的完整路径。 | `<leechcore.dll 所在目录>/leechcore.log` |
| `LEECHCORE_LOG_LEVEL` | 日志级别：`1` 输出摘要；`2` 额外输出每个 `MEM_SCATTER` 的详细信息。 | `1` |
| `LEECHCORE_LOG_FLUSH` | 设为非零时，每条日志都会立即 `fflush`，方便实时查看。 | `0` |

在调用程序启动前设置好上述变量即可。例如在 PowerShell 中：

```powershell
$env:LEECHCORE_LOG_PATH = "C:\\logs\\leechcore_fpga.log"
$env:LEECHCORE_LOG_LEVEL = "2"
$env:LEECHCORE_LOG_FLUSH = "1"
```

或在 Linux 下：

```bash
export LEECHCORE_LOG_PATH=/var/log/leechcore_fpga.log
export LEECHCORE_LOG_LEVEL=2
```

## 4. 调试 FPGA DMA 的建议

1. **保持日志等级为 2**：这样可以获取每个页的映射结果，便于核对 DMA 读写的真实物理地址。
2. **结合 PDB/符号文件**：下载 GitHub Actions 产出的 `leechcore.pdb`，配合调试器能准确定位触发日志的调用栈。
3. **区分远程与本地设备**：日志会自动标注 `device` 名称，如果通过远程代理访问，可快速甄别。
4. **分析耗时字段**：`duration` 字段能协助判断某次操作是否异常耗时，从而定位硬件瓶颈或传输错误重试。

## 5. 常见问题

* **日志文件无法创建**：确认目标目录存在且当前用户拥有写权限，必要时通过 `LEECHCORE_LOG_PATH` 指定到可写路径。
* **文件过大**：可以在调试完成后将 `LEECHCORE_LOG_DISABLE` 设为 1，或将 `LEECHCORE_LOG_LEVEL` 调整回 1 以仅保留摘要。
* **性能影响**：默认情况下日志以追加方式写入且不会在每行强制刷新，开销较低；如需实时查看则可启用 `LEECHCORE_LOG_FLUSH`，但会略微增加 I/O 开销。

通过以上能力，便可以快速定位调用 `leechcore.dll` 的上层程序在 FPGA DMA 板卡上执行的内存操作，极大提升排障与调试效率。
