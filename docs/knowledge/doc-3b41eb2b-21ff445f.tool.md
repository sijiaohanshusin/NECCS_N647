研究论文

第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展

**==> picture [505 x 59] intentionally omitted <==**

## 分布式红外阵列相机的超分辨图像重建研究

1 ， 2* 1 1 1 3 1 ， 2 1** 谢一博 ，徐乃涛 ，周顺 ，姚斯齐 ，余自然 ，程进 ，刘卫国 1 西安工业大学光电工程学院，陕西西安 710021 ；

> 2 无锡微视传感科技有限公司，江苏无锡 214101 ；

> 3 无锡翼盟电子科技有限公司，江苏无锡 214101

摘要 为了解决医疗领域中红外图像分辨率低的问题，搭建了一种结构简单、实时性高的分布式阵列红外成像系统，通 过图像算法处理来提高红外图像的分辨率。利用分布式阵列成像系统得到 4 幅具有像素级位移的图像，以一幅图像为基 准，对其余图像进行图像配准；再采用凸集投影算法进行图像重构，获得一幅较高分辨率的红外图像；最后采用生成对抗 神经网络重建方法获得红外超分辨率图像。实验结果表明：所搭建的分布式阵列相机的红外成像系统可实现实时超分 辨率图像重建，可使红外图像分辨率从 400×300 提升至 3200×2400 （分辨率提高了 8 倍）；与原图像相比，超分辨率重建 后的图像均值提升了 1. 86% ，标准差提升了 8. 67% ，熵值基本不变。所提图像处理算法实现了对红外图像的超分辨率重 建，能够满足医疗领域中的红外超分辨率图像的应用需求。

关键词 红外成像；分布式；图像重构；超分辨率 中图分类号 TP391 文献标志码 A DOI： 10. 3788/LOP202259. 1611004

## - Super Resolution Image Reconstruction of Distributed

## Infrared Array Camera

Xie Yibo1,2*, Xu Naitao1, Zhou Shun1, Yao Siqi1, Yu Ziran3, Cheng Jin1,2, Liu Weiguo1** 1 _School of Photoelectric Engineering, Xi_ ’ _an Technological University, Xi_ ’ _an_ 710021 _, Shaanxi, China_ ; 2 _Wuxi V_ - _Sensor Technology Co., Ltd., Wuxi_ 214101 _, Jiangsu, China_ ;

> 3 _Wuxi Yimeng Electronic Technology Co., Ltd., Wuxi_ 214101 _, Jiangsu, China_

Abstract To address the low resolution problem of infrared images in the medical field, we build a distributed array – based infrared imaging system with a simple structure and a real[-] time performance that achieved an improved image resolution using image algorithm processing. The proposed system is used to obtain four images with pixel[-] level displacement. One of the images is used as a benchmark, and the other three images are registered. Then, the projection of convex set algorithm is used to reconstruct the images and obtain a high[-] resolution infrared image. Finally, the reconstruction method of a generative admissible neural network is employed to obtain the infrared super[-] resolution image. Experimental results show that the infrared imaging system with a distributed array camera can realize real[-] time super[-] resolution image reconstruction, and the infrared image resolution can be improved from 400 × 300 to 3200×2400 (an eightfold increment). Compared with the original image, the mean and standard deviation of the super[-] resolution reconstructed image increase by 1. 86% and 8. 67%, while the entropy value remains basically unchanged. The proposed image processing algorithm realizes the super[-] resolution reconstruction for infrared images, which meets the application requirements of infrared super[-] resolution imaging in the medical field.

Key words infrared imaging; distributed; image reconstruction; super[-] resolution

## 1 引 言

长期以来，红外成像技术在军事、工业、医疗等各 个领域都有广泛的应用，尤其在医疗领域中，生病时的

人体热像与正常生理状态下的人体热像相比是有差别 的［ 1 ］，因此高分辨率的红外图像能够帮助医生对病情 进行准确判断，使得疾病诊断的准确率得到进一步提 高，给社会带来更大的效益。当前市面热成像成品种

收稿日期： 2021[-] 07[-] 26 ；修回日期： 2021[-] 08[-] 10 ；录用日期： 2021[-] 08[-] 17 * ** 通信作者： 13319215096@163. com ； wgliu@163. com

1611004[-] 1

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


类繁多，其中美国 FLIR 公司的产品性能尤为突出，该 公司产品中像素最高的热成像仪型号为 T1010 ［ 2 ］，分 辨率仅为 1024×768 ，且价格昂贵，故市场对低成本、 高分辨的红外成像系统翘首以盼。本文提出一种成本 低廉、结构简单的基于 2×2 分布式的高分辨红外成像 系统，通过超分辨率图像算法，实现人体热分布的图像 质量提升。

目前常用的超分辨率图像重建算法按照技术原理 分类为图像插值算法［ 3 ］、最大后验概率（ MAP ）算法［ 4 ］、 迭代反投影（ IBP ）算法［ 4 ］、凸集投影（ POCS ）算法［ 5 ］及 基于神经网络的重建技术［ 6 ］等。其中凸集投影算法具 有运算速度快以及先验信息包含能力强大的优点。而 在各种神经网络中，生成对抗网络（ SRGAN ）存在两 个不同的网络模型，训练方式采用的是对抗训练方式， 通过不断迭代的方式不断优化图像，进而提高图像的 质量。所以本文采用 POCS 与 SRGAN 相融合的方式 作为超分辨率图像的处理算法，先采用 POCS 算法将 2×2 阵列相机的 4 幅分辨率为 400×300 的图像融合， 得到单幅分辨率为 800×600 的图像，再采用 SRGAN 作为超分辨率重建算法，使红外图像的分辨率从 800×600 提升至 3200×2400 ，解决了医疗领域中红外 图像分辨率较低的问题。

## 2 分布式阵列相机硬件平台搭建

为获取多幅具有像素级偏差的图像，搭建一个具 有 2×2 网格式阵列相机的红外成像系统。多个相机 采用同步触发拍摄的模式获取相同方向视场的图像， 得到某一时刻由于相机空间位置偏差导致像素位置偏 差的一组阵列图像。该系统结构简单、实时性高、性价 比高。

根据分布孔径的多个子孔径等效为一个更大孔径 的设计思想，将红外相机按照分布孔径结构紧密排列 组合在一起，最终设计出分布式［ 7 ］阵列红外成像系统， 其实物图如 图 1 所示。 4 台红外相机分别通过千兆网 线与交换机进行连接，可实现数据的同步传输；交换机 与计算机网口连接，可通过上位机软件实现 4 台红外 相机的同步触发与图像数据的高速同步传输。在设计

**==> picture [227 x 132] intentionally omitted <==**

## 上位机软件时，为保障相机的同步触发，本研究采用多 线程模式控制相机。

以人体作为红外成像系统的拍摄目标。在中红外 波段的电磁波是分析分子结构最有用、信息最丰富的 区域，在该波段下人体的温度信息更准确，故采用红外 相机的光谱范围 8~14 μm ；由于红外相机的分辨率与 其价格呈强负相关，所以根据性价比选择分辨率为 400×300 的红外相机；因为镜头的通光孔径越大，所 覆盖的图像信息越多，图像之间的位移差越小，故选择 红外镜头的视场角 53. 13°×41. 11° ；为保证 4 幅图像之 间有较小的位移且具有信息丰富、相对清晰的人体红 外图像，选择最合适的拍摄距离 2~3 m ，故对应的镜 头焦距为 6. 8 mm 。图 1 中相机的具体详细参数如表 1 所示。

表 1 MV[-] GF120 相机参数 Table 1 MV[-] GF120 camera parameters

|Table 1<br>MV-GF12|0 camera parameters|
|---|---|
|Parameter|Value|
|Spectral range /μm|8-14|
|Resolution|400×300|
|Pixel size /μm|17|
|Frame rate /Hz|50|
|Viewing angle|53. 13°×41. 11°|
|F#|1. 0|
|Focal length /mm|6. 8|



## 3 超分辨率重建算法实现

为实现人体红外图像的超分辨率重建，采用图像 处理的方式，主要包含四个部分：数据预处理、基于小 波变换的相位相关法图像配准、图像重构、图像超分辨 率重建。其中主要研究方法为 POCS 与 SRGAN 的融 合算法，可实现红外图像的超分辨率重建，将红外图像 的分辨率从 400×300 提升至 3200×2400 ，并且该融合 算法具有很好的可移植性和可复用性。图像超分辨率 重建的步骤如图 2 所示。

**==> picture [242 x 157] intentionally omitted <==**

图 1 分布式阵列红外成像系统实物图 Fig. 1 Physical picture of distributed array infrared imaging system

图 2 超分辨率图像重建步骤 Fig. 2 Super[-] resolution image reconstruction steps

1611004[-] 2

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


## 3. 1 图像归一化

由于设备之间存在一定的参数差异， 4 台相机之 间的温度数据存在差异，如图 3 方框中的温度数值，可 以看到图像温度差异高达 1 ℃ ，这些数据的差异会影 响后续图像处理的效果。为解决该问题，根据图像归 一化的基本思想，对数据进行预处理，从而减小设备间 温度差异的影响。

首先，将红外图像以 RAW 格式的 16 bit 数据保存 后遍历求取 4 张红外图像的最大温度数值 _T_ max 和最小 温度数值 _T_ min ；再对每幅红外图像进行遍历，对温度数 据进行归一化处理。归一化处理的公式为

**==> picture [225 x 58] intentionally omitted <==**

式中： img ［ _x_ ］［ _y_ ］为二维红外图像坐标； new ［ _x_ ］［ _y_ ］为 归一化后的图像坐标。经过归一化处理后，得到的

**==> picture [242 x 181] intentionally omitted <==**

图 3 红外相机采集的图像 Fig. 3 Image collected by infrared camera

4 幅图像的温度颜色相近，达到了减小温度差异的效 果，如图 4 所示。

**==> picture [326 x 206] intentionally omitted <==**

图 4 归一化后的图像 Fig. 4 Normalized image

## 3. 2 基于小波变换的相位相关法图像配准

为了后续的图像重构，则需要进行图像配准，使不 同图像中对应于空间同一位置的点一一对应起来。根 据频率域平移理论［ 8 ］，通过基于小波变换的相位相关 法图像配准求得每个图像与基准图像在 _x_ 、 _y_ 方向上平 移的像素距离，然后采用仿射变换对图像进行校准。 因为小波变换具有完善的重构能力，所以先采用小波 分解得到图像的低频信息后，再采用相位相关法进行 相对运动估算。

对 图 4 归一化后的 4 幅红外图像进行二维小波变 换。每个二维小波都是两个一维函数的积。对二维图 像进行小波变换后，得到 4 个尺度函数。

**==> picture [180 x 27] intentionally omitted <==**

**==> picture [182 x 27] intentionally omitted <==**

式中： _φ_ （ _x_ ， _y_ ）为二维尺度函数； _ΨH_ 、 _ΨV_ 及 _ΨD_ 分别度量 沿水平边缘、沿垂直边缘及沿对角线的变化。式（ 3 ） ~ （ 5 ）为可分离的“方向敏感”小波，小波度量图像中灰度 沿不同方向的变化。

图 5 （ a ）为二维离散小波分解的过程图。通过高 通滤波器和低通滤波器将图像分解为 _A_ 、 _H_ 、 _V_ 、 _D_ 高低 频信息。选择 camera 1 的图像作为基准，然后对基准 图像以及其余三张图像进行小波分解，得到主要信息， 即低频小波分量 _A_ 。图 5 （ b ）为对归一化图像进行小波 分解后得到的频率信息。

对图像进行小波分解得到低频分量 _A_ 后，再对待 配准图像进行相对运动估算。首先，对图像 （ _f x_ ， _y_ ）进

1611004[-] 3

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


**==> picture [376 x 223] intentionally omitted <==**

图 5 二维小波变换示意图及结果图。（ a ）二维离散小波变换；（ b ）小波分解结果 Fig. 5 Schematic diagram and result of two[-] dimensional wavelet transform. (a) Two[-] dimensional discrete wavelet transform; (b) result of wavelet decomposition

行频率域的傅里叶变换：

**==> picture [219 x 26] intentionally omitted <==**

式中： _M_ 和 _N_ 为图像的尺寸大小； _f_ 是空间域值； _F_ 是频 域值。

假设 _f_ （ 1 _x_ ， _y_ ）是基准图像， _f_ （ 2 _x_ ， _y_ ）是 _f_ （ 1 _x_ ， _y_ ）在 _x_ 轴 上移动 _x_ 0 ，在 _y_ 轴上移动 _y_ 0 的结果，即

**==> picture [193 x 11] intentionally omitted <==**

将 _f_ （ 1 _x_ ， _y_ ）代入傅里叶变换公式可得 _F_ （ 1 _u_ ， _v_ ）；再 将式（ 7 ）代入傅里叶变换公式化简，可得

**==> picture [224 x 27] intentionally omitted <==**

**==> picture [191 x 26] intentionally omitted <==**

式中： _F_ *1[是] _[F]_ 1[的共轭复数。] 傅里叶逆变换公式为

**==> picture [205 x 27] intentionally omitted <==**

将 _C_ （ _u_ ， _v_ ）代入式（ 10 ）得到互功率谱的逆变换 −1 _C_ （ _u_ ， _v_ ），再将零频率分量移到频谱的中心，便于观察 −1 傅里叶变换，最后求出 _C_ （ _u_ ， _v_ ）最高点的坐标值（ _x_ 0 ， ［ 9 ］ _y_ 0 ），即为两幅图像之间的平移量 。

采用相位相关法进行相对运动估算，最终得到两 幅图像互功率谱逆变换后的三维频谱图，如图 6 所示， 方框中为 _f_ （ 1 _x_ ， _y_ ）与 _f_ （ 2 _x_ ， _y_ ）分别在 _x_ 方向和 _y_ 方向的平 移量。

经过上述的相对运动估算后得到两个图像之间的 平移量，再对待配准图像进行仿射变换平移，用一个矩 阵模型描述两幅图像的对应关系，平移矩阵为

**==> picture [198 x 175] intentionally omitted <==**

**==> picture [213 x 76] intentionally omitted <==**

式中： _tx_ 与 _ty_ 分别为 _x_ 与 _y_ 方向上的平移距离。根据上 述过程得到 _x_ 0 和 _y_ 0 后，将待配准的图像在 _x_ 和 _y_ 坐标上 分别移动 _x_ 0 和 _y_ 0 个像素数，最终完成图像的配准。假 设 _x_ 0 和 _y_ 0 均为 1 ，则将图像在 _x_ ， _y_ 方向上分别移动一个 像素数，如图 7 所示。

## 3. 3 凸集投影算法

通过图像配准将 4 幅图像在空间位置上的点相互 对应后，再采用凸集投影（ POCS ）算法作为本文的图 像重构算法，该算法具有运算速度快以及先验信息包 含能力强大的优点，所以被很好地应用于图像超分辨 率重建领域。凸集投影算法是为了找到满足所有凸约

1611004[-] 4

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


**==> picture [342 x 150] intentionally omitted <==**

图 7 图像校准平移对比图

Fig. 7 Comparison diagram of image calibration and translation

束集的解，从给定向量空间中的任何点开始进行最优 ［ 10 ］ 解搜索的过程 。

以 camera 1 得到的图像作为基准图像，对其余三 幅图像进行校准平移后，再对 4 幅图像进行图像重构， 重构前的图像（ SRC ）的分辨率为 400×300 ，重构后的

红外图像（ REC ）的分辨率是原红外图像的 2 倍，分辨 率提升至 800×600 。图 8 为对图像进行重构前后的对 比，对重构前后的图像局部信息进行放大，可以看出原 图像的信息较模糊，而重构后图像的细节信息更加明 显，故 POCS 的重构效果较好。

**==> picture [354 x 130] intentionally omitted <==**

图 8 初始图像和重构图像的对比。（ a ）初始图像[-] SRC ；（ b ）重构图像[-] REC Fig. 8 Comparison of the initial image and the reconstructed image. (a) Initial image[-] SRC; （ b ） reconstructed image[-] REC

## 3. 4 SRGAN

采用 POCS 算法将图像的分辨率提升 2 倍后，采用 生成对抗网络（ SRGAN ）进一步提高红外图像的分辨率。 图 9 为 SRGAN 的模型结构图， SRGAN 包含两个

深度学习的模型结构，即生成网络和判别网络。生成 网络部分包含多个残差块，每个残差块包含两个 3×3 的卷积层，并且每个卷积层后会存在一个批规范层 （ BN ）和一个激活函数 PReLU ，之后接上两个亚像素

**==> picture [454 x 169] intentionally omitted <==**

图 9 SRGAN 的两个模型结构（生成网络 G 和判别网络 D ） Fig. 9 Two model structures of SRGAN (generation network G and discriminant network D)

1611004[-] 5

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


卷积层。判别网络部分共包含 8 个卷积层，选取 Leaky ReLU 作为激活函数，最后通过两个全连接层和 Sigmoid 激活函数得到预测为自然图像的概率。两个 模型结构不断进行博弈，进而使 G 生成逼真的图像，而 D 具有非常强的判断图像真伪的能力［ 11 ］。

SRGAN 的损失函数由内容损失和对抗损失两部 分组成［ 11[-] 12 ］，表达式为

**==> picture [183 x 32] intentionally omitted <==**

其中，内容损失选择的是基于训练好的以 ReLU 为激活函数的 VGG 模型的损失函数，表达式为

**==> picture [245 x 27] intentionally omitted <==**

**==> picture [28 x 10] intentionally omitted <==**

式中： _i_ 表示 VGG19 网络中第 _i_ 个最大池化层； _j_ 表示第 _j_ 个卷积层； _W i_ ， _j_ 和 _Hi_ ， _j_ 是描述 VGG 网络内各个特征图 的尺寸。对抗损失函数的表达式为

**==> picture [193 x 25] intentionally omitted <==**

式中： _D θ D_ [ _G θ G_ ( _I_ LR )] 是重建图像； _G θ G_ ( _I_ LR ) 是自然高分 辨率图像的概率。

SRGAN 存在两个不同的网络，并且训练方式采 用的是对抗训练方式，这种方式有利于提升图像质量。 生成对抗网络在不断迭代的过程中不断地对图像进行 改进优化，最终所得的超分辨率重建图像的效果会更 好［ 13[-] 14 ］。如 图 10 所示， SRGAN 重建后的图像细节信 息更平滑， SRGAN 有效地解决重构图像边缘存在块 状的问题。

**==> picture [312 x 116] intentionally omitted <==**

图 10 重构图像和 SRGAN 重建图像对比。（ a ）重构图像[-] REC ；（ b ） SRGAN 重建图像 Fig. 10 Comparison of reconstructed image and reconstructed image obtained by SRGAN. （ a ） Reconstructed image[-] REC ； （ b ） reconstructed image obtained by SRGAN

## 4 实验数据分析

为了进一步证明所提算法的优势，选用残差网络

［ 15[-] 16 ］ - （ EDSR ） 作为对比算法。 图 11 为初始图像 SRC 、 POCS 重构图像[-] REC 、 EDSR 重建图像以及 SRGAN 重建图像的局部放大对比结果，红外图像的分辨率有

**==> picture [307 x 222] intentionally omitted <==**

图 11 图像对比结果。（ a ）初始图像[-] SRC ；（ b ）重构图像[-] REC ；（ c ） EDSR 重建图像；（ d ） SRGAN 重建图像 Fig. 11 Comparison results of the images. (a) Initial image[-] SRC; (b) reconstructed image[-] REC; (c) reconstructed image obtained by EDSR; (d) reconstructed image obtained by SRGAN

1611004[-] 6

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


所提高，并且图像的细节信息也更加明显。由于理想 图像难以获得，所以采用无参考图像方法中的熵值、均 值以及标准差对原图像、重构图像、 EDSR 重建图像以 及 SRGAN 重建图像进行质量评价［ 12 ］，分析结果如表 2

所示。其中图像的熵值、平均值、标准差分别代表了图 像携带信息的多少、图像的平均亮度、图像像素灰度值 的离散程度。

## 表 2 各图像质量评价指标对比

Table 2 Comparison of image quality evaluation indexes

|Image|Entropy（H）|Mean（u）|Standard deviation（std）|Resolution /（pixel×pixel）|
|---|---|---|---|---|
|Initial image|5. 3373|30. 3486|29. 9060|400×300|
|Reconstructed image|5. 3592|30. 6888|32. 4704|800×600|
|EDSR reconstructed image|5. 3340|30. 5813|32. 3854|3200×2400|
|SRGAN reconstructed image|5. 3730|30. 9126|32. 4996|3200×2400|



从 表 2 可直观看出：分辨率为 400×300 的初始图 像的熵值较高；重构后图像分辨率为 800×600 （ 2 倍）， 其熵值提高了 0. 022 （ 0. 41% ），均值提高了 0. 340 （ 1. 12% ），标准差提高了 2. 564 （ 8. 57% ）；与原图像相 比， EDSR 重建后的图像熵值和均值基本不变，标准差 提高了 2. 480 （ 8. 29% ）， SRGAN 重建后的图像分辨率 为 3200×2400 ，其熵值提高了 0. 036 （ 0. 67% ），均值提 高了 0. 564 （ 1. 86% ），标准差提高了 2. 594 （ 8. 67% ）。 分析实验结果得出结论， POCS 与 SRGAN 融合的算 法得到的超分辨率图像质量较好，分辨率相比原图像 提升了 8 倍。

**==> picture [78 x 13] intentionally omitted <==**

基于分布孔径思想，搭建了分布式阵列红外成像 系统作为硬件平台，然后对图像进行超分辨率重建算 法处理；对 4 幅红外图像进行小波分解，得到图像的低 频分量，经过相位相关的配准后得到图像之间的位移 参数量，并对 3 幅待配准图像进行校准；校准后对 4 幅 分辨率为 400×300 的红外图像进行 POCS 图像重构， 合成一幅分辨率为 800×600 的较高分辨率图像；最后 采用 SRGAN 进行超分辨率重建，得到 3200×2400 超 分辨率人体红外图像。从最终的重建结果来看，所提 方法得到的图像具有更清晰的人体细节信息，所提方 法解决了图像的块状问题，较好地解决医疗领域中的 红外图像分辨率低、放大后图像块状结构明显的问题。 所提方法对人体成像的超分辨率图像重建研究不仅在 医疗领域中有较好的应用，而且在公共安全领域中也 有巨大的价值。

**==> picture [73 x 10] intentionally omitted <==**

- [1] 张敏 , 张斌青 , 郭会利 , 等 . 功能影像学骨与关节疾病诊 断中的应用 [C]∥ 第十九届全国中西医结合骨伤科学术 研讨会 , 8 月 24 日到 26 日 , 2012, 洛阳 , 河南 . 北京 : 中 国中西医结合学会 , 2012: 30 - 34.

   - Zhang M, Zhang B Q, Guo H L, et al. The application of functional imaging in the diagnosis of bone and joint diseases[C]∥The 19th National Symposium of Integrated Chinese and Western Medicine Orthopaedics, August 24 -

26, 2012, Luoyang, Henan. Beijing: Chinese Association of Integrative Medicine, 2012: 30 - 34.

- [2] Patrick M R, Harris A J L, Ripepe M, et al. Strombolian explosive styles and source conditions: insights from thermal (FLIR) video[J]. Bulletin of Volcanology, 2007, 69(7): 769 - 784.

- [3] Yang J J, Zhang X, Peng W, et al. A novel regularized K - SVD dictionary learning based medical image super - resolution algorithm[J]. Multimedia Tools and Applications, 2016, 75(21): 13107 - 13120.

- [4] 李丹 . 多视图高清图像合成技术研究 [D]. 北京 : 北方工 业大学 , 2017. Li D. High definition image synthesis technology research of multi - view image[D]. Beijing: North China University of Technology, 2017.

- [5] 杨大伟 , 李丹 , 李健 . 基于 POCS 的超分辨率重建算法 研究 [J]. 微型机与应用 , 2014, 33(8): 39 - 41, 44. Yang D W, Li D, Li J. Research of reconstruction algorithm based on POCS super - resolution[J]. Microcomputer & Its Applications, 2014, 33(8): 39 - 41, 44.

- [6] 汪廷 . 红外图像与可见光图像融合研究与应用 [D]. 西 安 : 西安理工大学 , 2019. Wang T. Research and application of infrared image and visible image fusion[D]. Xi ’ an: Xi ’ an University of Technology, 2019.

- [7] Kong E M C, Miller D W, Sedwick R J. Exploiting orbital dynamics for aperture synthesis using distributed satellite systems: applications to a visible earth imager system[J]. The Journal of the Astronautical Sciences, 1999, 47(1/2): 53 - 75.

- [8] Zitová B, Flusser J. Image registration methods: a survey [J]. Image and Vision Computing, 2003, 21(11): 977 - 1000.

- [9] 甘亚莉 , 涂丹 , 李国辉 . 基于相位相关的小波域图像配 准方法研究 [J]. 计算机工程与设计 , 2006, 27(22): 4271 - 4273.

   - Gan Y L, Tu D, Li G H. Image registration approach based on phase correlation in wavelet domain[J]. Computer Engineering and Design, 2006, 27(22): 4271 - 4273.

- [10] 谢甜 . 使用改进的 POCS 算法的超分辨率图像复原 [J]. 电子设计工程 , 2013, 21(18): 142 - 144.

   - Xie T. Super - resolution image restoration via improved POCS algorithm[J]. Electronic Design Engineering, 2013, 21(18): 142 - 144.

1611004[-] 7

**==> picture [506 x 16] intentionally omitted <==**

**----- Start of picture text -----**<br>
研究论文 第 59 卷第 16 期 /2022 年 8 月 / 激光与光电子学进展<br>**----- End of picture text -----**<br>


- [11] 李斌 , 马璐 . 密集连接的生成对抗网络图像超分辨率重 建 [J]. 激光与光电子学进展 , 2020, 57(22): 221011. Li B, Ma L. Super - resolution reconstruction of densely connected generative adversarial network images[J]. Laser & Optoelectronics Progress, 2020, 57(22): 221011.

- [12] 程换新 , 刘文翰 . 基于生成对抗网络的图像超分辨率研 究 [J]. 电子测量技术 , 2020, 43(14): 132 - 135. Cheng H X, Liu W H. Images super - resolution using improved generative adversarial networks[J]. Electronic Measurement Technology, 2020, 43(14): 132 - 135.

- [13] 查体博 , 罗林 , 杨凯 , 等 . 基于改进超分辨率生成对抗 网络的图像重建算法 [J]. 激光与光电子学进展 , 2021, 58(8): 0810005.

   - Zha T B, Luo L, Yang K, et al. Image reconstruction algorithm based on improved super - resolution generative adversarial network[J]. Laser & Optoelectronics Progress,

2021, 58(8): 0810005.

- [14] Xu F, Hu C, Li J, et al. Special focus on deep learning in remote sensing image processing[J]. Science China Information Sciences, 2020, 63(4): 140300.

- [15] 胡芬 , 林洋 , 侯梦迪 , 等 . 基于深度学习的细胞骨架图 像超分辨重建 [J]. 光学学报 , 2020, 40(24): 2410001. Hu F, Lin Y, Hou M D, et al. Super - resolution reconstruction of cytoskeleton image based on deep learning[J]. Acta Optica Sinica, 2020, 40(24): 2410001.

- [16] 张晓菲 . 红外成像系统及其超分辨率重建技术的研究 [D]. 长春 : 中国科学院长春光学精密机械与物理研究 所 , 2020.

   - Zhang X F. Study on the imaging and super - resolution reconstruction of the infrared optical system[D]. Changchun: Changchun Institute of Optics, Fine Mechanics and Physics, Chinese Academy of Sciences, 2020.

1611004[-] 8
