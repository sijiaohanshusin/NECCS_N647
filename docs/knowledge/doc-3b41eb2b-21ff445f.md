<!-- page: 1 -->
第59 卷第16 期/2022 年8 月/激光与光电子学进展 研究论文

分布式红外阵列相机的超分辨图像重建研究

谢一博 1，2*，徐乃涛 1，周顺 1，姚斯齐 1，余自然 3，程进 1，2，刘卫国 1**
1西安工业大学光电工程学院，陕西西安710021；
2无锡微视传感科技有限公司，江苏无锡214101；
3无锡翼盟电子科技有限公司，江苏无锡214101

摘要 为了解决医疗领域中红外图像分辨率低的问题，搭建了一种结构简单、实时性高的分布式阵列红外成像系统，通
过图像算法处理来提高红外图像的分辨率。利用分布式阵列成像系统得到4 幅具有像素级位移的图像，以一幅图像为基
准，对其余图像进行图像配准；再采用凸集投影算法进行图像重构，获得一幅较高分辨率的红外图像；最后采用生成对抗
神经网络重建方法获得红外超分辨率图像。实验结果表明：所搭建的分布式阵列相机的红外成像系统可实现实时超分
辨率图像重建，可使红外图像分辨率从400×300 提升至3200×2400（分辨率提高了8 倍）；与原图像相比，超分辨率重建
后的图像均值提升了1. 86%，标准差提升了8. 67%，熵值基本不变。所提图像处理算法实现了对红外图像的超分辨率重
建，能够满足医疗领域中的红外超分辨率图像的应用需求。
关键词 红外成像；分布式；图像重构；超分辨率
中图分类号 TP391 文献标志码 A DOI：10. 3788/LOP202259. 1611004
Super-Resolution Image Reconstruction of Distributed
Infrared Array Camera
Xie Yibo 1,2*, Xu Naitao 1, Zhou Shun 1, Yao Siqi 1, Yu Ziran 3, Cheng Jin 1,2, Liu Weiguo 1**
1School of Photoelectric Engineering, Xi’an Technological University, Xi’an 710021, Shaanxi, China;
2Wuxi V-Sensor Technology Co., Ltd., Wuxi 214101, Jiangsu, China;
3Wuxi Yimeng Electronic Technology Co., Ltd., Wuxi 214101, Jiangsu, China

Abstract To address the low resolution problem of infrared images in the medical field, we build a distributed array –
based infrared imaging system with a simple structure and a real-time performance that achieved an improved image
resolution using image algorithm processing. The proposed system is used to obtain four images with pixel-level
displacement. One of the images is used as a benchmark, and the other three images are registered. Then, the projection
of convex set algorithm is used to reconstruct the images and obtain a high-resolution infrared image. Finally, the
reconstruction method of a generative admissible neural network is employed to obtain the infrared super-resolution image.
Experimental results show that the infrared imaging system with a distributed array camera can realize real-time super-
resolution image reconstruction, and the infrared image resolution can be improved from 400 × 300 to 3200×2400 (an
eightfold increment). Compared with the original image, the mean and standard deviation of the super-resolution
reconstructed image increase by 1. 86% and 8. 67%, while the entropy value remains basically unchanged. The proposed
image processing algorithm realizes the super-resolution reconstruction for infrared images, which meets the application
requirements of infrared super-resolution imaging in the medical field.
Key words infrared imaging; distributed; image reconstruction; super-resolution

人体热像与正常生理状态下的人体热像相比是有差别
1 引 言 的［1］，因此高分辨率的红外图像能够帮助医生对病情
长期以来，红外成像技术在军事、工业、医疗等各 进行准确判断，使得疾病诊断的准确率得到进一步提
个领域都有广泛的应用，尤其在医疗领域中，生病时的 高，给社会带来更大的效益。当前市面热成像成品种

收稿日期：2021-07-26；修回日期：2021-08-10；录用日期：2021-08-17
通信作者： *13319215096@163. com； **wgliu@163. com
1611004-1

<!-- page: 2 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展
类繁多，其中美国FLIR 公司的产品性能尤为突出，该 上位机软件时，为保障相机的同步触发，本研究采用多
公司产品中像素最高的热成像仪型号为T1010［2］，分 线程模式控制相机。
辨率仅为1024×768，且价格昂贵，故市场对低成本、 以人体作为红外成像系统的拍摄目标。在中红外
高分辨的红外成像系统翘首以盼。本文提出一种成本 波段的电磁波是分析分子结构最有用、信息最丰富的
低廉、结构简单的基于2×2 分布式的高分辨红外成像 区域，在该波段下人体的温度信息更准确，故采用红外
系统，通过超分辨率图像算法，实现人体热分布的图像 相机的光谱范围8~14 μm；由于红外相机的分辨率与
质量提升。 其价格呈强负相关，所以根据性价比选择分辨率为
目前常用的超分辨率图像重建算法按照技术原理 400×300 的红外相机；因为镜头的通光孔径越大，所
分类为图像插值算法［3］、最大后验概率（MAP）算法［4］、 覆盖的图像信息越多，图像之间的位移差越小，故选择
迭代反投影（IBP）算法［4］、凸集投影（POCS）算法［5］及 红外镜头的视场角53. 13°×41. 11°；为保证4 幅图像之
基于神经网络的重建技术［6］等。其中凸集投影算法具 间有较小的位移且具有信息丰富、相对清晰的人体红
有运算速度快以及先验信息包含能力强大的优点。而 外图像，选择最合适的拍摄距离2~3 m，故对应的镜
在各种神经网络中，生成对抗网络（SRGAN）存在两 头焦距为6. 8 mm。图1 中相机的具体详细参数如表1
个不同的网络模型，训练方式采用的是对抗训练方式， 所示。
通过不断迭代的方式不断优化图像，进而提高图像的 表1 MV-GF120 相机参数
质量。所以本文采用POCS 与SRGAN 相融合的方式 Table 1 MV-GF120 camera parameters
作为超分辨率图像的处理算法，先采用POCS 算法将 Parameter Value
2×2 阵列相机的4 幅分辨率为400×300 的图像融合，
得到单幅分辨率为800×600 的图像，再采用SRGAN Spectral range /μm 8-14
作为超分辨率重建算法，使红外图像的分辨率从 Resolution 400×300
Pixel size /μm 17800×600 提升至3200×2400，解决了医疗领域中红外
Frame rate /Hz 50图像分辨率较低的问题。
Viewing angle 53. 13°×41. 11°
2 分布式阵列相机硬件平台搭建 F# 1. 0
Focal length /mm 6. 8
为获取多幅具有像素级偏差的图像，搭建一个具
有2×2 网格式阵列相机的红外成像系统。多个相机
采用同步触发拍摄的模式获取相同方向视场的图像， 3 超分辨率重建算法实现
得到某一时刻由于相机空间位置偏差导致像素位置偏 为实现人体红外图像的超分辨率重建，采用图像
差的一组阵列图像。该系统结构简单、实时性高、性价 处理的方式，主要包含四个部分：数据预处理、基于小
比高。 波变换的相位相关法图像配准、图像重构、图像超分辨
根据分布孔径的多个子孔径等效为一个更大孔径 率重建。其中主要研究方法为POCS 与SRGAN 的融
的设计思想，将红外相机按照分布孔径结构紧密排列 合算法，可实现红外图像的超分辨率重建，将红外图像
组合在一起，最终设计出分布式［7］阵列红外成像系统， 的分辨率从400×300 提升至3200×2400，并且该融合
其实物图如图1 所示。4 台红外相机分别通过千兆网 算法具有很好的可移植性和可复用性。图像超分辨率
线与交换机进行连接，可实现数据的同步传输；交换机 重建的步骤如图2 所示。
与计算机网口连接，可通过上位机软件实现4 台红外
相机的同步触发与图像数据的高速同步传输。在设计

图1 分布式阵列红外成像系统实物图 图2 超分辨率图像重建步骤
Fig. 1 Physical picture of distributed array infrared imaging system Fig. 2 Super-resolution image reconstruction steps

1611004-2

<!-- page: 3 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展
3. 1 图像归一化
由于设备之间存在一定的参数差异，4 台相机之
间的温度数据存在差异，如图3 方框中的温度数值，可
以看到图像温度差异高达1 ℃，这些数据的差异会影
响后续图像处理的效果。为解决该问题，根据图像归
一化的基本思想，对数据进行预处理，从而减小设备间
温度差异的影响。
首先，将红外图像以RAW 格式的16 bit 数据保存
后遍历求取4 张红外图像的最大温度数值Tmax和最小
温度数值Tmin；再对每幅红外图像进行遍历，对温度数
据进行归一化处理。归一化处理的公式为
300 400
new [ x ] [ y ]= x∑= 0 y∑= 0
300 400 ( img [ x ] [ y ]- T min )×( 2 16 - 1 ) 图3 红外相机采集的图像
， （1） Fig. 3 Image collected by infrared camera x∑= 0 y∑= 0 T max - T min
式中：img［x］［y］为二维红外图像坐标；new［x］［y］为 4 幅图像的温度颜色相近，达到了减小温度差异的效
归一化后的图像坐标。经过归一化处理后，得到的 果，如图4 所示。

图4 归一化后的图像
Fig. 4 Normalized image
3. 2 基于小波变换的相位相关法图像配准 Ψ V ( x，y )= φ( x )Ψ ( y )， （4）
为了后续的图像重构，则需要进行图像配准，使不 Ψ D ( x，y )= Ψ ( x )Ψ ( y )， （5）
同图像中对应于空间同一位置的点一一对应起来。根 式中：φ（x，y）为二维尺度函数；Ψ H、Ψ V及Ψ D分别度量
据频率域平移理论［8］，通过基于小波变换的相位相关 沿水平边缘、沿垂直边缘及沿对角线的变化。式（3）~
法图像配准求得每个图像与基准图像在x、y 方向上平 （5）为可分离的“方向敏感”小波，小波度量图像中灰度
移的像素距离，然后采用仿射变换对图像进行校准。 沿不同方向的变化。
因为小波变换具有完善的重构能力，所以先采用小波 图5（a）为二维离散小波分解的过程图。通过高
分解得到图像的低频信息后，再采用相位相关法进行 通滤波器和低通滤波器将图像分解为A、H、V、D 高低
相对运动估算。 频信息。选择camera 1 的图像作为基准，然后对基准
对图4 归一化后的4 幅红外图像进行二维小波变 图像以及其余三张图像进行小波分解，得到主要信息，
换。每个二维小波都是两个一维函数的积。对二维图 即低频小波分量A。图5（b）为对归一化图像进行小波
像进行小波变换后，得到4 个尺度函数。 分解后得到的频率信息。
φ( x，y )= φ( x ) φ( y )， （2） 对图像进行小波分解得到低频分量A 后，再对待
Ψ H ( x，y )= Ψ ( x ) φ( y )， （3） 配准图像进行相对运动估算。首先，对图像f（x，y）进

1611004-3

<!-- page: 4 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展

图5 二维小波变换示意图及结果图。（a）二维离散小波变换；（b）小波分解结果
Fig. 5 Schematic diagram and result of two-dimensional wavelet transform. (a) Two-dimensional discrete wavelet transform; (b) result
of wavelet decomposition

行频率域的傅里叶变换：
1 M - 1 N - 1 -j2π( xuM + yvN F ( u，v )= f ( x，y )e )，（6） MN ∑i = 0 ∑j = 0
式中：M 和N 为图像的尺寸大小；f 是空间域值；F 是频
域值。
假设f1（x，y）是基准图像，f2（x，y）是f1（x，y）在x 轴
上移动x0，在y 轴上移动y0的结果，即
f2 ( x，y )= f1 ( x - x0，y - y0 )。 （7）
将f1（x，y）代入傅里叶变换公式可得F1（u，v）；再
将式（7）代入傅里叶变换公式化简，可得
F 2 ( x，y )= e-j2π( ux0 + vy0 ) ⋅F 1 ( u，v )。 （8）
两图像之间的互功率谱为
* F 2 F 1
C ( u，v )= = e j2π( ux0 + vy0 )， （9） 图6 三维频谱 * |F 2 F 1| Fig. 6 Three-dimensional frequency spectrum
式中：F * 1是F1的共轭复数。
傅里叶逆变换公式为 x' 1 0 tx x
y xu yv ty M ， （11） y' 0 1 M + j2π( f ( x，y )= F ( u，v )e N ) 。 （10） 0 0 1 1 1 ( ) =( ) ( ) u∑=- 01 Nv∑=- 01
式中：tx与ty分别为x 与y 方向上的平移距离。根据上 将C（u，v）代入式（10）得到互功率谱的逆变换
述过程得到x0和y0后，将待配准的图像在x 和y 坐标上C −1（u，v），再将零频率分量移到频谱的中心，便于观察
分别移动x0和y0个像素数，最终完成图像的配准。假傅里叶变换，最后求出C −1（u，v）最高点的坐标值（x0，
y0），即为两幅图像之间的平移量［9］。 设x0和y0均为1，则将图像在x，y 方向上分别移动一个
采用相位相关法进行相对运动估算，最终得到两 像素数，如图7 所示。
3. 3 凸集投影算法幅图像互功率谱逆变换后的三维频谱图，如图6 所示，
方框中为f1（x，y）与f2（x，y）分别在x 方向和y 方向的平 通过图像配准将4 幅图像在空间位置上的点相互
移量。 对应后，再采用凸集投影（POCS）算法作为本文的图
经过上述的相对运动估算后得到两个图像之间的 像重构算法，该算法具有运算速度快以及先验信息包
平移量，再对待配准图像进行仿射变换平移，用一个矩 含能力强大的优点，所以被很好地应用于图像超分辨
阵模型描述两幅图像的对应关系，平移矩阵为 率重建领域。凸集投影算法是为了找到满足所有凸约

1611004-4

<!-- page: 5 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展

图7 图像校准平移对比图
Fig. 7 Comparison diagram of image calibration and translation
束集的解，从给定向量空间中的任何点开始进行最优 红外图像（REC）的分辨率是原红外图像的2 倍，分辨
解搜索的过程［10］。 率提升至800×600。图8 为对图像进行重构前后的对
以camera 1 得到的图像作为基准图像，对其余三 比，对重构前后的图像局部信息进行放大，可以看出原
幅图像进行校准平移后，再对4 幅图像进行图像重构， 图像的信息较模糊，而重构后图像的细节信息更加明
重构前的图像（SRC）的分辨率为400×300，重构后的 显，故POCS 的重构效果较好。

图8 初始图像和重构图像的对比。（a）初始图像-SRC；（b）重构图像-REC
Fig. 8 Comparison of the initial image and the reconstructed image. (a) Initial image-SRC;（b）reconstructed image-REC
3. 4 SRGAN 深度学习的模型结构，即生成网络和判别网络。生成
采用POCS 算法将图像的分辨率提升2 倍后，采用 网络部分包含多个残差块，每个残差块包含两个3×3
生成对抗网络（SRGAN）进一步提高红外图像的分辨率。 的卷积层，并且每个卷积层后会存在一个批规范层
图9 为SRGAN 的模型结构图，SRGAN 包含两个 （BN）和一个激活函数PReLU，之后接上两个亚像素

图9 SRGAN 的两个模型结构（生成网络G 和判别网络D）
Fig. 9 Two model structures of SRGAN (generation network G and discriminant network D)

1611004-5

<!-- page: 6 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展
卷积层。判别网络部分共包含8 个卷积层，选取 式中：i 表示VGG19 网络中第i 个最大池化层；j表示第
Leaky ReLU 作为激活函数，最后通过两个全连接层和 j 个卷积层；Wi，j和Hi，j是描述VGG 网络内各个特征图
Sigmoid 激活函数得到预测为自然图像的概率。两个 的尺寸。对抗损失函数的表达式为
模型结构不断进行博弈，进而使G 生成逼真的图像，而 SR N' LR ， （14） l Gen - log D θD[ G θG ( I ) ]D 具有非常强的判断图像真伪的能力［11］。 =∑n = 1
SRGAN 的损失函数由内容损失和对抗损失两部 LR LR I 式中：D I ) 是自然高分 θD[G θG ( )]是重建图像；G θG (分组成［11-12］，表达式为
SR SR SR 辨率图像的概率。 X + - - - 10----3l---Gen 。 （12） l = - l
- SRGAN 存在两个不同的网络，并且训练方式采 - content-------- - loss----- adversarial---------- - --loss
perceptual - loss 用的是对抗训练方式，这种方式有利于提升图像质量。
其中，内容损失选择的是基于训练好的以ReLU 生成对抗网络在不断迭代的过程中不断地对图像进行
为激活函数的VGG 模型的损失函数，表达式为 改进优化，最终所得的超分辨率重建图像的效果会更
W i，j H i，j 2
SR 1 HR LR 好［13-14］。如图10 所示，SRGAN 重建后的图像细节信 ，lVGG/i.j= φ i，j ( I )x，y-φ i，j[ G θG ( I ) ] x，y } W i，j H i，j ∑x=1 ∑y=1 { 息更平滑，SRGAN 有效地解决重构图像边缘存在块
（13） 状的问题。

图10 重构图像和SRGAN 重建图像对比。（a）重构图像-REC；（b）SRGAN 重建图像
Fig. 10 Comparison of reconstructed image and reconstructed image obtained by SRGAN.（a）Reconstructed image-REC；
（b）reconstructed image obtained by SRGAN
（EDSR）［15-16］作为对比算法。图11 为初始图像-SRC、
4 实验数据分析 POCS 重构图像-REC、EDSR 重建图像以及SRGAN
为了进一步证明所提算法的优势，选用残差网络 重建图像的局部放大对比结果，红外图像的分辨率有

图11 图像对比结果。（a）初始图像-SRC；（b）重构图像-REC；（c）EDSR 重建图像；（d）SRGAN 重建图像
Fig. 11 Comparison results of the images. (a) Initial image-SRC; (b) reconstructed image-REC; (c) reconstructed image obtained by
EDSR; (d) reconstructed image obtained by SRGAN

1611004-6

<!-- page: 7 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展

所提高，并且图像的细节信息也更加明显。由于理想 所示。其中图像的熵值、平均值、标准差分别代表了图
图像难以获得，所以采用无参考图像方法中的熵值、均 像携带信息的多少、图像的平均亮度、图像像素灰度值
值以及标准差对原图像、重构图像、EDSR 重建图像以 的离散程度。
及SRGAN 重建图像进行质量评价［12］，分析结果如表2
表2 各图像质量评价指标对比
Table 2 Comparison of image quality evaluation indexes
Image Entropy（H） Mean（u） Standard deviation（std） Resolution /（pixel×pixel）
Initial image 5. 3373 30. 3486 29. 9060 400×300
Reconstructed image 5. 3592 30. 6888 32. 4704 800×600
EDSR reconstructed image 5. 3340 30. 5813 32. 3854 3200×2400
SRGAN reconstructed image 5. 3730 30. 9126 32. 4996 3200×2400
从表2 可直观看出：分辨率为400×300 的初始图 26, 2012, Luoyang, Henan. Beijing: Chinese Association
像的熵值较高；重构后图像分辨率为800×600（2 倍）， of Integrative Medicine, 2012: 30-34.
[2] Patrick M R, Harris A J L, Ripepe M, et al.其熵值提高了0. 022（0. 41%），均值提高了0. 340 Strombolian explosive styles and source conditions:
（1. 12%），标准差提高了2. 564（8. 57%）；与原图像相 insights from thermal (FLIR) video[J]. Bulletin of
比，EDSR 重建后的图像熵值和均值基本不变，标准差 Volcanology, 2007, 69(7): 769-784.
提高了2. 480（8. 29%），SRGAN 重建后的图像分辨率 [3] Yang J J, Zhang X, Peng W, et al. A novel regularized
为3200×2400，其熵值提高了0. 036（0. 67%），均值提 K-SVD dictionary learning based medical image super-
高了0. 564（1. 86%），标准差提高了2. 594（8. 67%）。 resolution algorithm[J]. Multimedia Tools and Applications,
2016, 75(21): 13107-13120.分析实验结果得出结论，POCS 与SRGAN 融合的算 [4] 李丹. 多视图高清图像合成技术研究[D]. 北京: 北方工
法得到的超分辨率图像质量较好，分辨率相比原图像 业大学, 2017.
提升了8 倍。 Li D. High definition image synthesis technology research
5 结 论 ofof Technology,multi-view image[D].2017. Beijing: North China University
基于分布孔径思想，搭建了分布式阵列红外成像 [5] 杨大伟, 李丹, 李健. 基于POCS 的超分辨率重建算法
研究[J]. 微型机与应用, 2014, 33(8): 39-41, 44.系统作为硬件平台，然后对图像进行超分辨率重建算
Yang D W, Li D, Li J. Research of reconstruction
法处理；对4 幅红外图像进行小波分解，得到图像的低 algorithm based on POCS super-resolution[J]. Microcomputer
频分量，经过相位相关的配准后得到图像之间的位移 & Its Applications, 2014, 33(8): 39-41, 44.
参数量，并对3 幅待配准图像进行校准；校准后对4 幅 [6] 汪廷. 红外图像与可见光图像融合研究与应用[D]. 西
分辨率为400×300 的红外图像进行POCS 图像重构， 安: 西安理工大学, 2019.
合成一幅分辨率为800×600 的较高分辨率图像；最后 Wang T. Research and application of infrared image and
visible image fusion[D]. Xi’an: Xi’an University of采用SRGAN 进行超分辨率重建，得到3200×2400 超 Technology, 2019.
分辨率人体红外图像。从最终的重建结果来看，所提 [7] Kong E M C, Miller D W, Sedwick R J. Exploiting
方法得到的图像具有更清晰的人体细节信息，所提方 orbital dynamics for aperture synthesis using distributed
法解决了图像的块状问题，较好地解决医疗领域中的 satellite systems: applications to a visible earth imager
红外图像分辨率低、放大后图像块状结构明显的问题。 system[J]. The Journal of the Astronautical Sciences,
所提方法对人体成像的超分辨率图像重建研究不仅在 1999, 47(1/2): 53-75.
[8] Zitová B, Flusser J. Image registration methods: a survey医疗领域中有较好的应用，而且在公共安全领域中也
[J]. Image and Vision Computing, 2003, 21(11): 977-1000.
有巨大的价值。 [9] 甘亚莉, 涂丹, 李国辉. 基于相位相关的小波域图像配
参 考 文 献 准方法研究[J]. 计算机工程与设计, 2006, 27(22): 4271-
4273.
[1] 张敏, 张斌青, 郭会利, 等. 功能影像学骨与关节疾病诊 Gan Y L, Tu D, Li G H. Image registration approach
断中的应用[C]∥第十九届全国中西医结合骨伤科学术 based on phase correlation in wavelet domain[J]. Computer
研讨会, 8 月24 日到26 日, 2012, 洛阳, 河南. 北京: 中 Engineering and Design, 2006, 27(22): 4271-4273.
国中西医结合学会, 2012: 30-34. [10] 谢甜. 使用改进的POCS 算法的超分辨率图像复原[J].
Zhang M, Zhang B Q, Guo H L, et al. The application 电子设计工程, 2013, 21(18): 142-144.
of functional imaging in the diagnosis of bone and joint Xie T. Super-resolution image restoration via improved
diseases[C]∥The 19th National Symposium of Integrated POCS algorithm[J]. Electronic Design Engineering, 2013,
Chinese and Western Medicine Orthopaedics, August 24- 21(18): 142-144.

1611004-7

<!-- page: 8 -->
研究论文 第59 卷第16 期/2022 年8 月/激光与光电子学进展
[11] 李斌, 马璐. 密集连接的生成对抗网络图像超分辨率重 2021, 58(8): 0810005.
建[J]. 激光与光电子学进展, 2020, 57(22): 221011. [14] Xu F, Hu C, Li J, et al. Special focus on deep learning
Li B, Ma L. Super-resolution reconstruction of densely in remote sensing image processing[J]. Science China
connected generative adversarial network images[J]. Information Sciences, 2020, 63(4): 140300.
Laser & Optoelectronics Progress, 2020, 57(22): 221011. [15] 胡芬, 林洋, 侯梦迪, 等. 基于深度学习的细胞骨架图
[12] 程换新, 刘文翰. 基于生成对抗网络的图像超分辨率研 像超分辨重建[J]. 光学学报, 2020, 40(24): 2410001.
究[J]. 电子测量技术, 2020, 43(14): 132-135. Hu F, Lin Y, Hou M D, et al. Super-resolution
Cheng H X, Liu W H. Images super-resolution using reconstruction of cytoskeleton image based on deep
improved generative adversarial networks[J]. Electronic learning[J]. Acta Optica Sinica, 2020, 40(24): 2410001.
Measurement Technology, 2020, 43(14): 132-135. [16] 张晓菲. 红外成像系统及其超分辨率重建技术的研究
[13] 查体博, 罗林, 杨凯, 等. 基于改进超分辨率生成对抗 [D]. 长春: 中国科学院长春光学精密机械与物理研究
网络的图像重建算法[J]. 激光与光电子学进展, 2021, 所, 2020.
58(8): 0810005. Zhang X F. Study on the imaging and super-resolution
Zha T B, Luo L, Yang K, et al. Image reconstruction reconstruction of the infrared optical system[D]. Changchun:
algorithm based on improved super-resolution generative Changchun Institute of Optics, Fine Mechanics and
adversarial network[J]. Laser & Optoelectronics Progress, Physics, Chinese Academy of Sciences, 2020.

1611004-8
