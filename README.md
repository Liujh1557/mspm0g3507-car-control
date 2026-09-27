# MSPM0G3507 巡线小车｜双电机速度环与五路灰度控制

基于 TI MSPM0G3507、TB6612 和双路带编码器电机，实现 10 ms 双电机速度环与五路灰度差速巡线。原始工程在同一黑色胶带赛道按“完整跑完且未脱线”测试 15 次，项目作者报告成功 15 次（15/15）；下面附有 21.6 秒实车演示。

<a href="media/line-following-demo.mp4"><img src="media/demo-cover.jpg" alt="小车巡线演示封面" width="320"></a>

[观看 21.6 秒巡线演示](media/line-following-demo.mp4) · [查看测试口径与视频证据](docs/demo-evidence.md) · [查看控制代码](user_driver/motor.c)

视频对应作者提供的原始工程。仓库整理时修正了 PI 数值边界、居中起步和丢线目标速度；这些修订尚未重新上车验证。15/15 是原始工程在上述赛道的作者自测结果，仓库视频不能独立复核全部 15 次。

## 实现与证据

| 问题 | 实现位置 | 当前证据 |
| --- | --- | --- |
| 双路电机驱动与测速 | [`motor.c`](user_driver/motor.c)、[`key.c`](user_driver/key.c) | 代码及 [`empty.syscfg`](empty.syscfg) 引脚/定时器配置 |
| 10 ms 速度环 | [`motor.c`](user_driver/motor.c) 的定时器中断、[`control_math.h`](user_driver/control_math.h) 的有符号增量 PI | [主机单测](Tests/test_control_math.c.txt)覆盖负向调整及上下限 |
| 五路巡线输入 | [`huidu.c`](user_driver/huidu.c) | 原始工程的[实车视频](media/line-following-demo.mp4)展示沿黑线通过直线和弯道；作者报告同一赛道自测 15/15 次成功 |
| 串口观察 | [`main.c`](main.c) 每 500 ms 输出五路输入状态 | UART0 115200 配置见 SysConfig |

```mermaid
flowchart LR
    G[五路数字灰度输入] --> R[左右目标速度选择]
    E[左右编码器边沿] --> V[10 ms 轮速计算]
    R --> PI[双路增量 PI]
    V --> PI
    PI --> P[双路 PWM]
    P --> M[TB6612 与直流电机]
```

## 工程取舍与修订

这是从阶段性 CCS 小车工程整理出的可阅读源码。保留了项目元数据、`.syscfg`、应用代码和调试配置；编译缓存、生成代码、旧模板 README 与未使用的 OLED 驱动未上传。

整理时修正了三处源码级问题：原增量 PI 将负数变化量先转为 `uint16_t`，可能回绕；现在先以有符号浮点计算，再限幅到 0–4000。原居中分支取两轮当前最小目标速度，初始为零时无法起步；现在给双轮设定同一基准目标。五路均未检测到线时，目标速度改为零。**这些修订尚未在车上重新烧录验证，不应当作为已完成的实车性能成果。**

## 验证状态

- `Tests/run_host_tests.ps1`：本机 GCC 控制计算单测通过，覆盖负向更新、0/4000 限幅与零误差保持。
- TI SysConfig 1.26.2 用本机 MSPM0 SDK 2.11.00.07 成功生成配置；原项目元数据指定 SDK 2.10.00.04，因此存在版本不匹配警告。
- TI Arm Clang 4.0.2.LTS 对六个应用 `.c` 文件完成语法检查；项目元数据指定 4.0.4.LTS。**这不是完整 CCS 链接构建，也不是板端验证。**
- 2026-08-01 拍摄的约 21.6 秒[实车视频](media/line-following-demo.mp4)展示沿黑色胶带通过直线与弯道。项目作者确认视频对应其提供的原始小车工程；仓库当前源码又包含本次整理修订，二者差异见[证据说明](docs/demo-evidence.md)。
- 按项目作者说明，在同一条赛道以“完整跑完且未脱线”为成功标准，自测 15 次、成功 15 次（15/15，100%）。仓库仅有一段视频，不能独立复核全部 15 次测试；该结果也不代表其他赛道或整理后的代码修订版。
- 实车速度未测；速度曲线、传感器极性/轮径/编码器线数的标定记录仍缺失。不宣称速度、误差或其他赛道上的成功率。

## 复现

1. 在 Code Composer Studio 导入现有项目，检查 `.cproject` 所需 MSPM0 SDK 2.10.00.04、SysConfig 1.26.2 和 TI Arm Clang 4.0.4.LTS。按你的安装情况配置相同版本；变更工具版本时需重新验证。
2. 用 [`empty.syscfg`](empty.syscfg) 生成 `ti_msp_dl_config.c/.h`，再进行完整构建。生成文件由 SysConfig 管理，不应手工编辑。
3. 核对 [`motor.h`](user_driver/motor.h)、[`huidu.h`](user_driver/huidu.h) 中的实际接线，以及电机电源、逻辑电平、共地和调试探针。附带的 [`MSPM0G3507.ccxml`](targetConfigs/MSPM0G3507.ccxml) 指向 XDS110；烧录前要确认与实际探针一致。
4. 不接车也可以运行 `pwsh -File Tests/run_host_tests.ps1` 验证控制计算边界。

## 当前限制

轮速换算依赖代码中的轮径与编码器计数常数，未附标定报告。巡线决策是规则表，未验证丢线重寻、路口、速度阶跃稳定性，也未提供闭环响应曲线。传感器数字电平代表黑线还是白底需以实物模块设置核对。TI 示例入口的版权声明保留在 `main.c`；本仓库不把外部模板或板级驱动归为原创成果。
