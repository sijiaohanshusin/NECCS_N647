

**==> picture [44 x 44] intentionally omitted <==**

**----- Start of picture text -----**<br>
<br>**----- End of picture text -----**<br>


**==> picture [98 x 44] intentionally omitted <==**

**----- Start of picture text -----**<br>
 <br>**----- End of picture text -----**<br>


声源定位与声学成像 算法框架设计报告 A C O U S T I C  I M A G I N G  A L G O R I T H M  F R A M E W O R K

目标平台 主频  ⚡ STM32H743IIT6 Cortex-M7 @ 480MHz 阵列规模 实时约束   16路 / 2D平面阵列 ≤ 5ms / 帧 (≥30 FPS) 采样率 / FFT 核心算法   48kHz / 256点 RFFT SRP-PHAT 优化版 SRP-PHAT GCC-PHAT CMSIS-DSP Coarse-to-Fine TDOA LUT

声源定位与声学成像算法框架

2 / 10

##  目录与项目概述

##  报告目录

1. 目录与项目概述目录与项目概述录与项目概述项目概述目概述概述述

**==> picture [480 x 119] intentionally omitted <==**

**----- Start of picture text -----**<br>
1. 目录与项目概述目录与项目概述录与项目概述项目概述目概述概述述 P2<br>2. 算法路线选型分析 (问题A) P3–P4<br>3. SRP-PHAT 高效数学降维 (问题B) P5–P6<br>4. 空间流形矩阵预计算 (问题C) P7–P8<br>5. CMSIS-DSP 巧妙运用 (问题D) P9<br>6. 系统流水线与性能总结系统流水线与性能总结统流水线与性能总结线与性能总结总结 P10<br>**----- End of picture text -----**<br>


6. 系统流水线与性能总结系统流水线与性能总结统流水线与性能总结线与性能总结总结

## 阵列物理拓扑

**==> picture [524 x 320] intentionally omitted <==**

**----- Start of picture text -----**<br>
X (mm)<br>12<br>15<br>7<br>4<br>10 2 9<br>1<br>5<br>14<br>6<br>13<br>3<br>8<br>11<br>16<br>0 50 100<br>16路麦克风2D平面阵列拓扑 · 几何中心 (50, 50) mm · 非均匀螺旋分布<br>Y (mm)<br>**----- End of picture text -----**<br>


## 关键系统参数

**==> picture [523 x 27] intentionally omitted <==**

**----- Start of picture text -----**<br>
参数 值 说明<br>**----- End of picture text -----**<br>


|MCU|STM32H743IIT6|Cortex-M7,单精度FPU|
|---|---|---|
|主频|480 MHz|峰值 ~240 MFLOPS (SP)|
|麦克风数|16|麦克风对数C(16,2) = 120|
|采样率|48 kHz|奈奎斯特频率24 kHz|
|FFT长度|256点|频率分辨率187.5 Hz|
|帧时长|5.33 ms|256 / 48000|
|有效频点|128 bins|RFFT输出N/2个复数|
|声速|343 m/s|标准条件(20°C)|
|阵列孔径|~75 mm|最大对角距离|



声源定位与声学成像算法框架

3 / 10

>  问题 A：算法路线选型分析 本节对三种主流声源定位算法——SRP-PHAT、MUSIC 和频域 DAS——在 16 阵元 + STM32H7 嵌入式平台上的适用性进行深度对 比。

## 三大算法核心原理概述

① 频域 DAS (Delay-and-Sum Beamforming)：最经典的波束形成方法。对每个候选方向，将各通道信号按预计算的时延进行对 齐后求和，输出功率最大的方向即为声源方向。数学表达：

|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|式(1)：DAS波束形成输出功率<br>PDAS(q) =<br>M<br>∑<br>m=1<br>wm⋅Xm(f) ⋅ej2πfτm(q)<br>2<br>|
|---|---|---|---|---|---|
|②MUSIC (Multiple Signal Classifcation)：基于协方差矩阵特征分解的超分辨算法。将信号空间分解为信号子空间和噪声子空<br>间，通过搜索与噪声子空间正交的指向矢量来定位声源：<br>||||||
|PMUSIC(q) =||式(2)：MUSIC空间谱，<br> 为噪声子空间<br><br>1<br>aH(q)EnEH<br>n a(q)<br>En||||
|③SRP-PHAT (Steered Response Power — Phase Transform)：将GCC-PHAT互相关结果在空间网格上进行累积。PHAT加权<br>通过谱白化消除幅度影响，仅保留相位信息，使得在混响环境下仍能保持尖锐的相关峰：<br>||||||
|PSRP(q) =<br>M<br>∑<br>k=1<br>M<br>∑<br>l=k+1<br>∑<br>f<br>Re[||||式(3)：频域SRP-PHAT功率谱(核心公式)<br><br> Xk(f)X∗<br>l (f)<br>|Xk(f)X ∗<br>l (f)| ⋅ej2πfΔτkl(q)]||
|三维对比分析<br>评估维度|||||SRP-PHAT|
||频域DAS<br>|||MUSIC<br>||
|空间分辨率|低<br>受阵列孔径限制，主瓣宽<br>|||高<br>超分辨，可突破瑞利极限<br>|中高<br>优于DAS，接近MUSIC|
|混响鲁棒性|差<br>旁瓣抬升严重<br>|||差<br>协方差矩阵被混响污染<br>|优秀<br>PHAT白化抑制混响|
|计算复杂度|O(MFN)<br>最低，适合实时<br>|||O(M³+M²N)<br>特征分解极耗算力<br>|O(QFN)<br>Q=120对，可优化|
|多声源能力|弱<br>旁瓣干扰严重<br>|||强<br>需准确估计声源数<br>|中等<br>可分辨空间上分离的源|
|STM32H7可行性|完全可行<br>|||极困难<br>16×16特征分解~2ms<br>|可行(优化后)|



## ⭐ 选型结论

综合考虑国赛室内混响环境、STM32H7 算力约束和成像质量需求，强烈推荐采用频域 SRP-PHAT 的优化版本。它在混响鲁 棒性上远超 DAS 和 MUSIC，且通过粗细结合搜索+预计算 TDOA 表的方案，可在 5ms 内完成一帧定位。

声源定位与声学成像算法框架

4 / 10

## MUSIC 在 STM32H7 上的不可行性分析 MUSIC 算法的核心瓶颈在于协方差矩阵的特征分解。对于 16 路麦克风，需要对 16×16 的复数协方差矩阵执行特征值分解 (EVD)。 即使使用优化的 Jacobi 迭代法：

**==> picture [524 x 105] intentionally omitted <==**

**----- Start of picture text -----**<br>
L<br>Rxx = [1] ∑ X(fl)X [H] (fl) = EsΛsE [H] s [+][ σ][2][E][n][E][H] n<br>L<br>l=1<br>式(4)：协方差矩阵特征分解<br>**----- End of picture text -----**<br>


**==> picture [523 x 27] intentionally omitted <==**

**----- Start of picture text -----**<br>
MUSIC 计算步骤 运算量 M=16 实际值 耗时估算 @480MHz<br>**----- End of picture text -----**<br>


|协方差矩阵估计|O(M 2|⋅F)|256 × 256 = 32.8K复数MAC|~0.3 ms|
|---|---|---|---|---|
|特征值分解(EVD)|迭代<br>O(M 3)||~50次Jacobi旋转× 4096|~1.5–2.0 ms|
|空间谱搜索(每网格点)|<br>O(M 2)|矩阵乘|256 × N网格点|~0.5 ms (8×8网格)|
|总计||||~2.3–2.8 ms|



⚠ 关键问题：MUSIC 的 EVD 单步就消耗近 2ms，且在混响环境下协方差矩阵估计需要更多帧累积 (≥10帧) 才能稳定，这直接 导致成像延迟 ≥50ms，无法满足 30FPS 实时要求。此外，MUSIC 需要预知声源数目，在竞赛场景下不实际。

## 频域 DAS 的局限性 频域 DAS 虽然计算量最小，但其空间分辨率受到阵列孔径的物理限制。对于本阵列 (~75mm 孔径)，在 1kHz 频率下的 3dB 波束宽 度约为： θ3dB ≈[λ][343][/][1000] ≈4.57 rad ≈262° D[=] 0.075 式(5)：低频波束宽度 (1kHz)——分辨率极差

## 一 即使在 4kHz 下，波束宽度仍约 65°，这意味着 DAS 在中低频段几乎无法区分不同方向的声源。在国赛的室内环境中，混响会进 步 模糊 DAS 的成像结果。 SRP-PHAT 的决定性优势 为什么 SRP-PHAT 是最优选择？ PHAT 白化：通过 1/|XkXl[∗][|] 归一化，消除频谱幅度的影响，等效地对所有频率分量赋予相同权重。这使得在混响环境 下，直达声路径的相位信息被最大化保留。 120对冗余：16路麦克风产生 C(16,2)=120 个互相关对，远多于最少所需的 2 对。大量冗余通过空间累积有效抑制了虚假 峰值。

- 频域直接累积：关键优化——无需执行 120 次 IFFT。直接在频域用预计算的相位矢量进行旋转累积，大幅降低计算量。 粗细结合搜索：通过两级网格搜索将空间扫描点从 3600+ 降至 ~112，使得在 H7 上实现 ≥30FPS 完全可行。 无需先验信息：不需要预知声源数目，不需要长时间的协方差矩阵累积，单帧即可成像。

## 算力对比总结

**==> picture [523 x 26] intentionally omitted <==**

**----- Start of picture text -----**<br>
算法 单帧算力 (MFLOPS) @480MHz 耗时 混响鲁棒性 结论<br>**----- End of picture text -----**<br>


|频域DAS|~2.1|~0.5 ms|差|快但无用|
|---|---|---|---|---|
|MUSIC|~12.5|~2.8 ms|差(需累积)|不可行|
|SRP-PHAT (朴素)|~450 (60×60网格)|~95 ms|优秀|太慢|
|SRP-PHAT (优化版)|~14.5|~3.0 ms✓|优秀|✓ 推荐|



注：优化版 SRP-PHAT 采用 8×8 粗搜索 + 3×(4×4) 细搜索 = 112 网格点，频域直接累积 (跳过 IFFT)，预计算 TDOA LUT。详见问题 B 的完整方 案。

声源定位与声学成像算法框架

5 / 10

##  问题 B：SRP-PHAT 高效数学降维

本节详细推导频域 SRP-PHAT 的 GCC-PHAT 计算公式，并提供跳过 IFFT 的频域直接累积方案与粗细结合搜索策略。

## B.1 GCC-PHAT 交叉功率谱公式推导

**==> picture [524 x 647] intentionally omitted <==**

**----- Start of picture text -----**<br>
对于麦克风对 (i, j)，设其频域信号为 Xi(fk) 和 Xj(fk)（由 256 点 RFFT 得到，k = 0, 1, … , 127），GCC-PHAT 的计算分为以<br>下三步：<br>1 交叉功率谱 (Cross-Power Spectrum)<br>Gij(fk) = Xi(fk) ⋅ Xj [∗][(][f][k][)]<br>式(6)：复数共轭乘法，输出为复数<br>其中 Xj [∗][(][f][k][)] 为 Xj(fk) 的复共轭。在 CMSIS-DSP 中，频域数据以交织格式 [Re0, Im0, Re1, Im1, …] 存储。<br>2 PHAT 相位加权 (Phase Transform)<br>˜ Gij(fk) Xi(fk) ⋅ Xj [∗][(][f][k][)]<br>Gij(fk) =<br>|Gij(fk)| + ε [=] |Xi(fk) ⋅ Xj [∗][(][f][k][)| +][ ε]<br>式(7)：PHAT 白化——仅保留相位信息，ε ≈10 [−10] 防除零<br>白化后 |G [˜] ij(fk)| ≈1，即所有频率分量权重相等。这是 SRP-PHAT 抗混响的核心机制。<br>3 频域 SRP 累积 (跳过 IFFT 的关键优化)<br>M M K−1<br>P (q) = ∑ ∑ ∑ Re[G [˜] ij(fk) ⋅ e [j][2][πf][k][⋅Δ][τ][ij][(][q][)] ]<br>i=1 j=i+1 k=0<br>式(8)：频域直接累积 SRP 功率——无需 IFFT<br>其中：<br>K = 128 为有效频点数 (RFFT 正频部分)<br>fk = k ⋅Δf = k ⋅187.5 Hz 为第 k 个频率 bin<br>Δτij(q) 为从候选声源位置 q 到麦克风对 (i, j) 的时延差 (TDOA)<br>e [j][2][πf][k][Δ][τ][ij][(][q][)] 为预计算的相位旋转因子 (Steering Phase)<br>**----- End of picture text -----**<br>


🔑 核心优化：传统方法需要对 120 对麦克风分别做 IFFT (120 × 256点 IFFT = 巨大开销)。而式(8)直接在频域用预计算的相位 因子旋转后实部累加，将 120 次 IFFT 替换为 120 × 128 次复数乘法 + 实部取出，计算量降低约 10 倍。

## B.2 展开式(8)的实数运算细节

˜ 将复数乘法展开为实数运算，令 Gij(fk) = a + jb，预计算相位 e[jϕ] = cos ϕ + j sin ϕ：

Re[(a + jb)(cos ϕ + j sin ϕ)] = a cos ϕ − b sin ϕ 式(9)：每个(对, 频点, 网格点)仅需 2次乘法 + 1次减法 SRP 功率累积：功率累积：率累积：累积：积：： 127 P (q) = ∑ ∑ [aij,k cos(ϕij,k(q)) − bij,k sin(ϕij,k(q))] (i,j)∈P k=0

**==> picture [166 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
因此，对于单个网格点的 SRP 功率累积：功率累积：率累积：累积：积：：<br>**----- End of picture text -----**<br>


式(10)：完全展开的实数运算形式，P 为 120 个麦克风对集合

声源定位与声学成像算法框架

6 / 10

## B.3 粗细结合搜索策略 (Coarse-to-Fine) 朴素 SRP-PHAT 在 60×60 = 3600 个网格点上搜索，对 H7 来说计算量过大。粗细结合策略将搜索分为两级：

**==> picture [524 x 91] intentionally omitted <==**

**----- Start of picture text -----**<br>
第一级: 粗搜索 提取 Top-3 峰值 第二级: 细搜索<br>8×8 = 64 个网格点 取最大的 3 个粗格 3 × (4×4) = 48 个网格点<br>**----- End of picture text -----**<br>


## 搜索空间设计 假设声源在阵列前方 0.3m ~ 3.0m 范围，扫描平面的角度范围为 ±60°（方位角和俯仰角），采用球坐标 (θ, φ) 参数化： 粗搜索：θ ∈[−60°, 60°]，步长 15°；φ ∈[−60°, 60°]，步长 15°。共 9 × 9 = 81 点（或简化为 8×8 = 64 点） 细搜索：在 Top-3 粗峰值周围 ±7.5° 范围内，步长 3.75°。每个峰值区域 4×4 = 16 点，共 48 点 总计：64 + 48 = 112 个网格点（相比 60×60 = 3600 点，减少 97%）

## 伪代码实现

**==> picture [189 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
/ =======  粗细结合  SRP-PHAT  伪代码  =======<br>**----- End of picture text -----**<br>


**==> picture [252 x 8] intentionally omitted <==**

**----- Start of picture text -----**<br>
/  输入 : GCC_PHAT[120][128] — 120 对的白化交叉功率谱  ( 复数 )<br>**----- End of picture text -----**<br>


**==> picture [304 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
/  输入 : SteerPhase_Coarse[64][120][128] —  粗网格预计算相位  (cos,sin)<br>**----- End of picture text -----**<br>


**==> picture [304 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
/  输入 : SteerPhase_Fine[48][120][128]   —  细网格预计算相位  (cos,sin)<br>**----- End of picture text -----**<br>


**==> picture [154 x 9] intentionally omitted <==**

**----- Start of picture text -----**<br>
/  输出 : best_pos —  声源方向  (θ, φ)<br>**----- End of picture text -----**<br>


float SRP_Coarse[64];

/ //= 第一级 : 粗搜索 (8×8 = 64 点 ) //= for (n = 0; n < 64; n/+) { SRP_Coarse[n] = 0.0f; for (p = 0; p < 120; p/+) { / 遍历 120 对 float acc = 0.0f; for (k = 0; k < 128; k/+) { / 遍历 128 频点 float a = GCC_re[p][k]; / PHAT 白化后实部 float b = GCC_im[p][k]; / PHAT 白化后虚部 float c = Steer_cos[n][p][k]; / 预计算 cos(φ) float s = Steer_sin[n][p][k]; / 预计算 sin(φ) acc += a * c - b * s; / Re{G·e^(jφ)} } SRP_Coarse[n] += acc; } } / //= 提取 Top-3 峰值 //= top3_idx = find_top3(SRP_Coarse, 64); / //= 第二级 : 细搜索 (3 × 4×4 = 48 点 ) //= float SRP_Fine[48]; for (n = 0; n < 48; n/+) { SRP_Fine[n] = 0.0f; for (p = 0; p < 120; p/+) { float acc = 0.0f; for (k = 0; k < 128; k/+) { acc += GCC_re[p][k] * FineCos[n][p][k] - GCC_im[p][k] * FineSin[n][p][k]; } SRP_Fine[n] += acc; } } best_fine_idx = argmax(SRP_Fine, 48); best_pos = FineGrid[best_fine_idx]; / 最终声源方向

## B.4 算力估算

**==> picture [523 x 27] intentionally omitted <==**

**----- Start of picture text -----**<br>
阶段 网格点 MAC 运算量 @480MHz 耗时<br>**----- End of picture text -----**<br>


|GCC-PHAT计算(120对)|—|120 × 128 × 6 = 92.2K|~0.2 ms|
|---|---|---|---|
|粗搜索(8×8)|64|64 × 120 × 128 × 3 = 2.95M|~1.2 ms|
|细搜索(3×4×4)|48|48 × 120 × 128 × 3 = 2.21M|~0.9 ms|
|Top-3提取+最终argmax|—|微量|< 0.01 ms|
|总计|112|~5.25M MAC|~2.3 ms✓|



- ✅ 结论：粗细结合的频域 SRP-PHAT 总耗时约 2.3ms，远小于 5ms 的帧周期限制。剩余约 2.7ms 可用于数据搬运、热力图渲 染等后处理任务。成像帧率可稳定在 ≥ 60 FPS（远超 30FPS 要求）。

## 一 B.5 进 步优化空间

- 频率子带裁剪：仅使用 500Hz~8kHz 范围的频点 (约 bin 3~43)，将频点数从 128 降至 ~40，粗搜索降至 ~0.4ms 内循环向量化：将 128 个频点的 a ⋅ c − b ⋅ s 用 `arm_dot_prod_f32` 展开为两次点积之差，充分利用 FPU 流水线 动态 Top-N：根据粗搜索峰值分布自适应调整细搜索区域数量 (1~5 个) 帧间预测：利用前一帧的定位结果缩小当前帧的粗搜索范围，在跟踪模式下可跳过粗搜索

声源定位与声学成像算法框架

7 / 10

##  问题 C：空间流形矩阵预计算 利用麦克风坐标固定的特性，预先计算所有网格点的 TDOA 延迟表和指向相位矢量，固化到 Flash/SDRAM 中，运行时零延迟查表。

## C.1 TDOA 延迟公式推导

设麦克风 i 的位置为 ri = (xi, yi, 0)（阵列在 z = 0 平面），候选声源位置为 q = (qx, qy, qz)。声源到麦克风 i 的传播距离为： di(q) = ∥q − ri∥=[√] (qx − xi)[2] + (qy − yi)[2] + qz[2]

**==> picture [117 x 9] intentionally omitted <==**

## 麦克风对 (i, j) 的 TDOA (到达时间差) 为：

## Δτij(q) =[d][i][(][q][)][ −][d][j][(][q][)] =[∥][q][ −][r][i][∥][−][∥][q][ −][r][j][∥] c c 式(12)：TDOA 核心公式，c = 343 m/s

远场近似 (Far-field Approximation) 当声源距离 ≫ 阵列孔径时（如 d > 0.5m vs 孔径 75mm），可采用平面波近似。设声源方向为单位向量 u^ = (sin θ cos φ,  sin θ sin φ,  cos θ)，则： Δτ u =[(][x][i][ −][x][j][)][ sin][ θ][ cos][ φ][ + ][(][y][i][ −][y][j][)][ sin][ θ][ sin][ φ] ij(^) ≈[(][r][i][ −][r][j][)][ ⋅^][u] c c 式(13)：远场平面波 TDOA 近似——仅依赖方向角，计算更简单

💡 建议：国赛场景声源距离通常 0.5m~3m，远场近似的误差在 ±0.1°以内，推荐使用远场模型以简化预计算。若需近场精 度，使用式(12)的球面波模型。

**==> picture [524 x 135] intentionally omitted <==**

预计算并存储其 cos 和 sin 值：

## SteerCos[n][p][k] = cos(ϕ[(] p[n] ,k[)][),] SteerSin[n][p][k] = sin(ϕ[(] p[n] ,k[)][)] 式(15)：预计算 cos/sin 查找表

## C.3 内存需求分析

**==> picture [523 x 27] intentionally omitted <==**

**----- Start of picture text -----**<br>
数据结构 维度 元素数 float32 大小 存储位置<br>**----- End of picture text -----**<br>


|SteerCos (粗)|[64][120][128]|983,040|3.75 MB|SDRAM|
|---|---|---|---|---|
|SteerSin (粗)|[64][120][128]|983,040|3.75 MB|SDRAM|
|SteerCos (细)|[48][120][128]|737,280|2.81 MB|SDRAM|
|SteerSin (细)|[48][120][128]|737,280|2.81 MB|SDRAM|
|总计||3,440,640|~13.1 MB|SDRAM|



📌 内存策略：13.1MB 超出 512KB AXI SRAM，必须使用外部 SDRAM。但 SDRAM 的读带宽在 H7 上约 100~200 MB/s，读取 13.1MB 需要 ~70ms，远超帧周期。因此需要分块 DMA 预取策略——下一节给出优化方案。

声源定位与声学成像算法框架

8 / 10

## C.4 内存优化：TDOA 索引法 (推荐方案) 直接存储 cos/sin 表需要 13.1MB，不可接受。核心优化思路：只存储 TDOA 延迟值，运行时实时计算 cos/sin。

**==> picture [524 x 77] intentionally omitted <==**

**----- Start of picture text -----**<br>
(n)<br>TDOA\_LUT[n][p] = Δτp (float32,  单位 :  秒 )<br>式(16)：仅存储 TDOA 值，维度 [网格点数][麦克风对数]<br>**----- End of picture text -----**<br>


**==> picture [523 x 25] intentionally omitted <==**

**----- Start of picture text -----**<br>
方案 维度 存储大小 存储位置 运行时开销<br>**----- End of picture text -----**<br>


|方案A:全量cos/sin|[112][120][128]×2|13.1 MB|SDRAM (慢)|零(纯查表)|
|---|---|---|---|---|
|方案B: TDOA索引法 ✓|[112][120]|52.5 KB|AXI SRAM|实时计算cos/sin|
|方案C:量化int16|[112][120][128]×2|6.6 MB|SDRAM|反量化|



✅ 推荐方案 B：TDOA 索引法仅需 52.5 KB，完全放入 AXI SRAM（零等待访问）。运行时在内循环中对每个频点 k，计算 ϕ = 2πkΔf ⋅TDOA[n][p]，然后用快速 sin/cos 查表或 CORDIC 求值。Cortex-M7 的 FPU 执行 `sinf()/cosf()` 约需 20~30 周期，128 频点的额外开销约 120×128×30 = 460K 周期 ≈ 1ms，总耗时仍在 5ms 以内。

## C.5 PC 端预计算 Python 脚本

**==> picture [523 x 586] intentionally omitted <==**

**----- Start of picture text -----**<br>
!/usr/bin/env python3<br>""" 预计算  TDOA LUT  并导出为  C  数组头文件 """<br>import numpy as np<br>from itertools import combinations<br># ##= 麦克风坐标  (mm  →  m) ##=<br>mic_pos = np.array([<br>    [65.00,50.00], [36.45,62.41], [51.85,28.87], [64.43,68.82],<br>    [24.42,45.47], [73.68,34.94], [42.21,78.97], [35.33,21.76],<br>    [81.51,61.51], [17.48,63.42], [65.57,16.72], [61.44,86.49],<br>    [15.66,30.10], [90.12,41.18], [25.60,84.71], [44.38, 6.63]<br>]) / 1000.0 #  转换为米<br>c = 343.0 #  声速  m/s<br>pairs = list(combinations(range(16), 2))  # 120  对<br># ##= 构建粗搜索网格  ( 远场 : θ, φ 各  8  级 ) ##=<br>theta_coarse = np.linspace(-60, 60, 8)  #  方位角  ( 度 )<br>phi_coarse   = np.linspace(-60, 60, 8)  #  俯仰角  ( 度 )<br>def compute_tdoa_farfield(theta_deg, phi_deg, pair):<br>""" 远场  TDOA = (r_i - r_j) · u_hat / c"""<br>    th = np.radians(theta_deg)<br>    ph = np.radians(phi_deg)<br>    u_hat = np.array([np.sin(th)*np.cos(ph), np.sin(th)*np.sin(ph)])<br>    i, j = pair<br>    dr = mic_pos[i] - mic_pos[j]  # 2D  向量差<br>return np.dot(dr, u_hat) / c<br># ##= 计算粗网格  TDOA LUT ##=<br>N_coarse = len(theta_coarse) * len(phi_coarse)  # 64<br>tdoa_lut = np.zeros((N_coarse, 120), dtype=np.float32)<br>grid_idx = 0<br>for th in theta_coarse:<br>for ph in phi_coarse:<br>for p, pair in enumerate(pairs):<br>            tdoa_lut[grid_idx, p] = compute_tdoa_farfield(th, ph, pair)<br>        grid_idx += 1<br># ##= 导出为  C  头文件 ##=<br>with open("tdoa_lut_coarse.h", "w") as f:<br>    f.write(" /  自动生成  -  粗搜索  TDOA LUT [64][120]\n")<br>    f.write("const float TDOA_Coarse[64][120] = {\n")<br>for n in range(N_coarse):<br>        vals = ", ".join(f"{v:.9e}f" for v in tdoa_lut[n])<br>        f.write(f"  {{{vals}}},\n")<br>    f.write("};\n")<br>**----- End of picture text -----**<br>


## C.6 Flash 固化策略

- 粗网格 TDOA LUT：64 × 120 × 4B = 30 KB → 存入 STM32H7 内部 Flash (2MB 容量) 细网格 TDOA LUT：需要在粗搜索后动态确定位置，有两种策略： 策略A (预计算全量)：预计算所有可能的细网格区域，存入 Flash。对 64 个粗格各 16 个细点 = 1024 × 120 × 4B = 480 KB，可 存入 Flash。 策略B (运行时计算)：仅存粗网格 TDOA，细网格 TDOA 在确定 Top-3 后实时计算。48 × 120 = 5760 次远场 TDOA 计算 (式 13)，每次约 10 周期 → ~0.12ms，可接受。

📌 推荐：采用 策略A (Flash 全量预计算)。480KB 对 2MB Flash 毫无压力，且运行时完全零计算开销。将 TDOA LUT 通过 `__attribute__((section(".flash_data")))` 放入 Flash 的指定段。

声源定位与声学成像算法框架

9 / 10

##  问题 D：CMSIS-DSP 的巧妙运用 本节说明如何将 GCC-PHAT 与 SRP 累积的每一步映射到 CMSIS-DSP 函数，最大化利用 Cortex-M7 的单精度 FPU 流水线。

## D.1 关键 CMSIS-DSP 函数映射

**==> picture [523 x 24] intentionally omitted <==**

**----- Start of picture text -----**<br>
CMSIS-DSP 函数 算法步骤 用途说明<br>**----- End of picture text -----**<br>


|arm_cmplx_mult_cmplx_f32|交叉功率谱(式6)|。需先对<br> 共轭。<br>Gij(k) =Xi(k) ⋅X ∗<br>j (k)<br>Xj|
|---|---|---|
|arm_cmplx_mag_f32|PHAT归一化分母|计算<br>，用于白化除法。<br>|Gij(k)| = √<br>Re2 + Im2|
|arm_cmplx_conj_f32|复共轭|对<br> 取共轭：<br>。<br>Xj<br>Im →−Im|
|arm_dot_prod_f32|SRP频域累积|将式(10)分解为两个点积之差：<br>。<br>∑akck−∑bksk|
|arm_rfft_fast_f32|实数FFT (已完成)|256点RFFT，输出交织格式[Re,Im,Re,Im,...]。|
|arm_max_f32|峰值检测|在SRP数组中找最大值及其索引。|



## D.2 GCC-PHAT 核心循环 C 代码实现

/ GCC-PHAT 计算 : 120 对麦克风的白化交叉功率谱 / 输入 : Mic_Freq_Buffer[16][256] — 交织复数 [Re0,Im0,Re1,Im1, .] / 输出 : GCC_PHAT[120][256] — 白化后的交叉功率谱 ( 交织复数 ) #include "arm_math.h" #define N_FFT 256 / FFT 长度 #define N_BINS 128 / 有效频点 (N_FFT/2) #define N_MICS 16 #define N_PAIRS 120 #define EPSILON 1e-10f / 预构建的麦克风对索引表 ( 在 Flash 中 ) static const uint8_t pair_i[N_PAIRS]; / pair_i[p], pair_j[p] static const uint8_t pair_j[N_PAIRS]; / 工作缓冲区 (DTCM 或 AXI SRAM) float32_t conj_buf[N_FFT]; / 共轭缓冲 float32_t cross_buf[N_FFT]; / 交叉功率谱 float32_t mag_buf[N_BINS]; / 幅度 float32_t GCC_Re[N_PAIRS][N_BINS]; / PHAT 后实部 float32_t GCC_Im[N_PAIRS][N_BINS]; / PHAT 后虚部 void compute_gcc_phat_all(float32_t Mic_Freq[][N_FFT]) { for (uint32_t p = 0; p < N_PAIRS; p/+) { uint8_t mi = pair_i[p], mj = pair_j[p]; / Step 1: 对 Xj 取复共轭 arm_cmplx_conj_f32(Mic_Freq[mj], conj_buf, N_BINS); / Step 2: 交叉功率谱 Xi · Xj* arm_cmplx_mult_cmplx_f32(Mic_Freq[mi], conj_buf, cross_buf, N_BINS); / Step 3: 计算幅度 |Xi · Xj*| arm_cmplx_mag_f32(cross_buf, mag_buf, N_BINS); / Step 4: PHAT 白化 — 逐频点除以幅度 for (uint32_t k = 0; k < N_BINS; k/+) { float32_t inv_mag = 1.0f / (mag_buf[k] + EPSILON); GCC_Re[p][k] = cross_buf[2*k]     * inv_mag; GCC_Im[p][k] = cross_buf[2*k + 1] * inv_mag; } } }

D.3 SRP 频域累积的向量化技巧 式(10)的核心内循环 ∑k(akck − bksk) 可分解为两次 `arm_dot_prod_f32` ：

/ 单个网格点的 SRP 功率计算 ( 向量化版本 ) float32_t compute_srp_single_grid( uint32_t grid_idx, float32_t cos_lut[][N_BINS], / [N_PAIRS][N_BINS] 运行时计算或查表 float32_t sin_lut[][N_BINS]) { float32_t srp_power = 0.0f; for (uint32_t p = 0; p < N_PAIRS; p/+) { float32_t dot_ac, dot_bs; / Σ(a_k · cos_k) — 利用 FPU 流水线的 128 次 MAC arm_dot_prod_f32(GCC_Re[p], cos_lut[p], N_BINS, &dot_ac); / Σ(b_k · sin_k) arm_dot_prod_f32(GCC_Im[p], sin_lut[p], N_BINS, &dot_bs); srp_power += dot_ac - dot_bs; / Re{G·e^(jφ)} } return srp_power; }

🚀 FPU 流水线优化： `arm_dot_prod_f32` 内部展开为 4 路并行 MAC (乘累加)，每次循环处理 4 个 float32。128 个频点仅需 32 次循环迭代，充分利用 Cortex-M7 的 FPU 单周期乘法和双发射流水线。相比逐元素的标量循环，吞吐量提升约 3~4 倍。

声源定位与声学成像算法框架

10 / 10

##  系统流水线与性能总结 端到端处理流程

**==> picture [320 x 356] intentionally omitted <==**

**----- Start of picture text -----**<br>
DMA 双缓冲接收 (与计算并行)<br>1<br>16路 I2S/SAI → DMA 交织传输 → Ping-Pong 缓冲区，非缓存区域 (MPU 配置)<br>0 ms (与上一帧计算重叠)<br>↓<br>时域预处理 (已完成)域预处理 (已完成)预处理 (已完成)处理 (已完成)理 (已完成) (已完成)已完成)成))<br>2<br>解交织织 → 加汉宁窗汉宁窗窗 → 256点 arm_rfft_fast_f32 × 16路 256点 arm_rfft_fast_f32 × 16路点 arm_rfft_fast_f32 × 16路 arm_rfft_fast_f32 × 16路路 → Mic_Freq_Buffer[16][256]<br>~0.4 ms (16 × 256点 RFFT)<br>↓<br>GCC-PHAT 计算 (120 对)<br>3<br>共轭 → 复数乘法 → 幅度计算 → PHAT 白化 → GCC_Re/Im[120][128]<br>~0.8 ms<br>↓<br>粗搜索 (8×8 = 64 点)<br>4<br>TDOA LUT 查表查表表 → 实时 cos/sin → arm_dot_prod_f32 向量累积 cos/sin → arm_dot_prod_f32 向量累积→ arm_dot_prod_f32 向量累积 arm_dot_prod_f32 向量累积向量累积量累积累积积 → Top-3 峰 Top-3 峰峰值提取<br>~1.2 ms<br>↓<br>细搜索 (3×4×4 = 48 点)<br>5<br>在 Top-3 粗格周围细化 → 精确声源方向 (θ, φ)<br>~0.9 ms<br>↓<br>**----- End of picture text -----**<br>


时域预处理 (已完成)域预处理 (已完成)预处理 (已完成)处理 (已完成)理 (已完成) (已完成)已完成)成)) 2 解交织织 → 加汉宁窗汉宁窗窗 → 256点 arm_rfft_fast_f32 × 16路 256点 arm_rfft_fast_f32 × 16路点 arm_rfft_fast_f32 × 16路 arm_rfft_fast_f32 × 16路路 → Mic_Freq_Buffer[16][256]

TDOA LUT 查表查表表 → 实时 cos/sin → arm_dot_prod_f32 向量累积 cos/sin → arm_dot_prod_f32 向量累积→ arm_dot_prod_f32 向量累积 arm_dot_prod_f32 向量累积向量累积量累积累积积 → Top-3 峰 Top-3 峰峰值提取 ~1.2 ms

后处理 & 输出 6 声源坐标映射 → 热力图更新 → 串口/LCD 输出 ~0.2 ms

## 性能预算表

**==> picture [523 x 27] intentionally omitted <==**

**----- Start of picture text -----**<br>
处理阶段 MAC 运算量 耗时 (ms) 占帧预算<br>**----- End of picture text -----**<br>


|RFFT × 16|~200K|0.4|8%|
|---|---|---|---|
|GCC-PHAT × 120|~400K|0.8|16%|
|粗搜索(64点)|~2.95M|1.2|24%|
|细搜索(48点)|~2.21M|0.9|18%|
|后处理|微量|0.2|4%|
|总计|~5.76M|3.5 ms|70%|



✅ 实时性验证：总处理耗时 ~3.5ms，帧周期 5.33ms (256/48kHz)，余量 1.83ms (35%)。理论成像帧率可达 ~285 FPS（实 际受限于 ADC 帧率 ~188 FPS）。远超国赛要求的 30FPS。

内存布局总览 ⚡ DTCM (128 KB)  AXI SRAM (512 KB)  Flash (2 MB) Mic_Freq_Buffer[16][256] = 16 GCC_Re[120][128] = 60 KB TDOA_Coarse[64][120] = 30 KB KB GCC_Im[120][128] = 60 KB TDOA_AllFine[1024][120] = 480 conj_buf / cross_buf = 2 KB TDOA_LUT[112][120] = 52.5 KB KB mag_buf = 0.5 KB cos/sin 临时缓冲 [120][128] = 120 pair_i/j[120] = 240 B SRP_Coarse/Fine = 0.5 KB KB 程序代码 = ~200 KB 已用: ~19 KB / 128 KB 已用: ~293 KB / 512 KB 已用: ~710 KB / 2048 KB

📋 关键建议：① 使用 DWT_CYCCNT 寄存器精确测量每个阶段的实际周期数，验证理论估算。② 频率子带裁剪 (500Hz~8kHz) 可将粗搜索耗时再降 60%。③ 帧间跟踪模式下可跳过粗搜索，仅做细搜索，耗时降至 ~2ms。④ 内存布局需通过链接脚本精 确控制各段的存放区域。
