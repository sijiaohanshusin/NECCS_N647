<!-- page: 1 -->
2024 7th International Conference on Computer Information Science and Application Technology (CISAT)

Infrared Camera Array Image Super-Resolution based
on Zero-Shot Learning

Qianyu Chen Jungang Yang* Wei An
National University of Defense National University of Defense National University of Defense
Technology, China Technology, China Technology, China
chenqianyu18@nudt.edu.cn yangjungang@nudt.edu.cn anwei@nudt.edu.cn

Abstract—The infrared camera array is capable of capturing Since the images of infrared camera array can be generalized10.1109/CISAT62382.2024.10695386 infrared features in a single scene from multiple views, thereby to the light field (LF) data with large disparities, the successful
supplying rich angular information for a range of post-capture trials of disentangling spatial, angular and epipolar features inDOI: tasks, one of which is the image super-resolution (SR). Since the
| LF image SR are also available for references [18]. The state-of-
IEEE spatialeffects, resolutionprevalent SRof eachmethodologiessub-image endeavoris limited toby improvethe diffractionimage the art LF image SR methods generally separate the high- dimensional LF data into sub-dimensions and employ the most
quality by acquiring prior knowledge from extensive datasets. advanced deep learning architecture including the variants of
However, the scarcity of the publicly available infrared array©2024 convolutions [10]-[14], Transformer [15]-[17], diffusion [19]-
image dataset makes it difficult to rely on conventional model [20] and Mamba [21]-[22]. Their SR results in both the real- training strategies. To address this, inspired by the recent advance
world and synthetic public LF dataset HCI_old [23], HCI_new in zero-shot learning and light field image SR, this paper proposes
[24], EPFL [25], INRIA [26] and STFGantry [27] are appealing, a zero-shot infrared array image SR method. Specifically, we
however, they cannot be directly migrated to the infrared case design an array feature extraction module (AFEM) to incorporate
latent correlations within views and employ a zero-shot training due to the domain gap between infrared and visible light.
strategy to enable model learning with one input low-resolution Additionally, the scarcity of publicly available infrared light
scene. Experiments show that the proposed method can achieve field data presents a challenge in addressing this gap during the
reliable performance in both objective metrics and visual effects. training process.979-8-3503-7510-7/24/$31.00| To address these, we propose a zero-shot infrared array Keywords-Infrared image; camera array; zero-shot; super-
camera image super-resolution method, and to our best resolution
knowledge, it is a pioneering method for the infrared array(CISAT) images SR. Inspired by the recent advance in zero sample
I. INTRODUCTION learning in both single image SR [28] and LF image SR [29], we
Infrared camera array can record radiation features from train the array feature extraction module with the lower-low
different views in all light conditions and provide rich image pairs and test the performance with low-high image pairsTechnology complementary information for post-capture applications like in each scene. The main contributions of this paper can be
infrared target segmentation [1], tracking [2] and detection [3]. summarized as:
However, the acquisition of high-quality infrared array images -We propose an infrared array image super-resolution can be difficult due to the diffraction effects, noises and theApplication method to fit the large disparity issue in array cameras. inhomogeneity of sub-lens [4]-[5]. Inspired by the utilization of
and super-resolution (SR) techniques in single images, stereo images -We propose a zero-shot strategy for the infrared array image
Science andthe spotlight.videos, the SR of infrared image also becomes research in SR to deal with the shortage of infrared array image dataset. -Experiments show that the proposed method can achieve
Early infrared image SR approaches mainly rely on the remarkable performance in the real-world infrared array images.
interpolation, MAP, IBP and POCS methods. Driven by the
trend of machine learning, recent researches have employedInformation II. METHOD
some basic deep learning frameworks for infrared feature
extraction and reconstruction. For instance, Liao et al. [6] A. Preliminary
proposed an CNN-based infrared image SR method thatComputer The input low-resolution (LR) images of the infrared camera introduce grey scale transformation to SRCNN. Huang et al. [7]
on employed WDSR for infrared remote sensing image SR, and arrays can be represented as ݔ௅ோ∈ℝ௎×௏×ு×ௐ, in which the
ܷ × ܸ refers to the sub-lens array plane of the camera array, and evaluated the SR performance by subsequent target detection
ܪ × ܹ refers to the pixel plane in each sub-image. The array accuracy. Shao [8] proposed IRSR that based on residual
image SR techniques aim at generating the super-resolvedConference learning. Wang [9] introduced the ESRGAN to single infrared
images ݔௌோ∈ℝ௎×௏×௥ு×௥ௐ based on the input data ݔ௅ோ, where image SR. While these methods can demonstrate reliability with
the ݎ indicates the up-scaling factor. During the training phase single-view infrared images, their effectiveness in the context of
of the zero-shot pipeline, the ݔ ௅ோis also down-sampled to get infrared array camera images is somewhat constrained due to the
ೝு×భೝௐfor the model optimization. The overallInternational challenges associated with the insufficient decoupling of high- ݔ௅௅ோ∈ℝ௎×௏×భ
dimensional correlations. pipeline of the proposed method is shown in Figure 1.7th
2024

979-8-3503-7510-7/24/$31.00 ©2024 IEEE 384

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore. Restrictions apply.

<!-- page: 2 -->
Figure 1. The overall pipeline of proposed model

During the model training iterations, the ݔ ௅ோ is down-B. Array Feature Extraction Module sampled to get ݔ ௅௅ோ, and the array feature extraction module is
Different from the common light field that are captured by trained with the feedback of loss evaluation. The loss evaluation
micro-lens, the infrared camera array images have wider consists of the forward loss (the direct mean square error
baselines and requires a larger receptive field for feature between the super-resolved result ݔ ௅௅ோ→௅ோ and the reference ݔ௅ோ)
extraction and alignment. and the reverse loss (the MSE value of the down-sampled SR
result (ݔ ௅௅ோ→௅ோ)↓ and the original input ݔ௅௅ோ). We also Inspired by the advantages of deformable convolution
introduce a factor ߛ to adjust the proportion of reverse loss in the(DCNs) in enlarging the receptive field [13], we design an array
overall loss function.feature extraction module (AFEM) based on DCNs. Specifically,
the input low-resolution (LR) or lower than low-resolution (LLR) ݈݋ݏݏ = ܯܵܧ ( ݔ௅ோ, ݔ௅௅ோ→௅ோ) + ߛ ∗ ܯܵܧ ( ݔ௅௅ோ, ( ݔ௅௅ோ→௅ோ)↓)
images are sent to the initial CNNs (Conv) and residual CNNs
with dilations (ResConv) for feature extraction. Once the training iteration status meets the maximum setting
or the loss reach the threshold, the AFEM loads the parameter
ℱ௜௡௜௧= ܴ݁ݏܥ݋݊ݒ (ܴ݁ݏܣܵܲܲ (ܥ݋݊ݒ (ݔ ௅ோ))) weights and takes the ݔ ௅ோ as the input for test. The model
௖௩ weights can be recorded and optimized in the training and test The features are separated to the center-view ℱ௜௡௜௧ and side- phase of different scenes. Therefore, our pipeline can not onlyview groups ℱ௜௡௜௧௦௩, and sent to the DCN-based feature extraction perform super-resolution in the case of only one sample, but alsoprocess. During this process, the features are aligned, fused and
able to optimize the performance with a small number ofcollected in DCN coarse stage1, and then distributed to each
samples.view by DCN coarse stage2 to generate ℱ஼ௌଶ. The following
fine-level DCNs are similar to the coarse ones and generate ℱிௌଶ
based on the input ℱ஼ௌଶ. The extracted features go through the III. EXPERIMENTS
information fusion convolutions and up-sampling pixel-shuffle
to form the final SR results. A. Dataset Collection
Since the infrared array image datasets are not yet publicly The coarse offset ߜ is obtained by conducting convolutions
available currently, we collect a few scenes with a 3 × 3 infraredܥ݋݊ݒఋ on each side-view feature ℱ௦௩ and the center view camera array and verify the effectiveness of the proposedfeature ℱ௖௩ in stage 1, and the fine-level ߜ is calculated by the
method. As it is shown in Figure 2, the infrared cameras are
fused feature ℱௌଵand previous side-view feature in ܥ݋݊ݒఋᇱ. For oriented differently and equally spaced. The settings of each sub-example, the formulation of the DCN coarse stages can be
lens are the same, and the internal and external parameters arerepresented as:
pre-calibrated.
൜ℱ஼ௌଵ= ܦܥܰௌଵ(ℱ௜௡௜௧௦௩, ߜ௦௖ଵ), ߜ௦௖ଵ= ܥ݋݊ݒఋ(ℱ௜௡௜௧௖௩, ℱ௜௡௜௧௦௩)
ℱ஼ௌଶ= ܦܥܰௌଶ(ℱ௖௦ଵ, ߜ௦௖ଶ), ߜ௦௖ଶ= ܥ݋݊ݒఋᇱ(ℱ௜௡௜௧, ℱ௖௦ଵ)
The number of DCN-based feature extraction operation is set
to one in this paper, and can be increased in a stacked manner to
improve the performance in complex scenarios.

Figure 2. Toy model of the data collection method
C. Zero-Shot Strategy
Due to the difficulty and high cost of acquiring large number During the data collection, the band of the infrared camera
of infrared array image data, we try to implement our model in a array is 8~14μm and the frame size of each sub-image is set to
zero-shot manner. To be specific, we take each input ݔ ௅ோ as the 640×512. In order to evaluate the performance of our model in
reference of the SR results of ݔ௅௅ோ, and optimize the parameter recovering the details of images, we capture some sculptures and
weights based on the LLR-to-LR loss. buildings that have infrared texture features.

385

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore. Restrictions apply.

<!-- page: 3 -->
B. Implementation Details
In order to evaluate the super-resolution effect of the
proposed model, we took the original array images as the
references and the 0.5× and 0.25× downsampled image pairs as
the input LR images of the 2× and 4× SR model. The input
images were divided into patches for model training iterations, (a) buildings GT (b) bicubic PSNR=29.747 (c) proposed PSNR= 30.490
64 × 64 for 2 × SR, 256 × 256 for 4 × SR, and the data
augmentation strategy was also applied to these patches to avoid
overfitting. The proposed framework was implemented in
PyTorch and run with a laptop with Nvidia GTX 3060. The
maximum iteration number was set to 10000, and it could be
early stopped when the loss met the threshold. During the
iterations, we employed the Adam optimizer with an initial (d) sculpture1 GT (e) bicubic PSNR=29.120 (f) proposed PSNR=29.493
learning rate of 2e-4 and a decrease factor of 0.5 for every 2500
iteration. The test stage the input LR images would start when
the iteration ended, and the SR patches would be integrated for
the final result.

C. Results and Ablation Study
(g) sculpture2 GT (h) bicubic PSNR=28.114 (i) proposed PSNR= 28.821
a) Results: We compare the proposed model with the Figure 4. Visualization of zero-shot 4× SR results
traditional interpolation method in real-world datasets. Part of
the 2× and 4× SR results are shown in Figure 3 and 4. The peak- b) Ablation: In order to estimate the effectiveness of the
signal-to-noise value (PSNR) with unit dB has also been proposed framework, we trained and tested the variants of the
calculated to measure the SR effect objectively. proposed method. All variants employed the same zero-shot
learning strategy in the implementation subsection. The variant
w/o DCN substitutes DCN with the conventional CNNs for
array feature extraction in the same coarse-to-fine settings.
Variant w/o DCNFS removes the fine-level DCNs and the
variant w/o rloss subtracts the proportion of reverse loss in the
overall loss evaluation.
(a) buildings GT (b) bicubic PSNR=34.169 (c) proposed PSNR=35.540
TABLE I. THE PSNR RESULTS OF THE VARIANTS

PSNR 2×
variants
buildings sculpture1 sculpture2 sculpture3

bicubic 34.169 34.320 33.425 33.706
(d) sculpture1 GT (e) bicubic PSNR=34.320 (f) proposed PSNR=36.394
w/o DCN 35.135 36.152 35.327 35.059

w/o DCNFS 35.007 36.090 35.235 35.074

w/o rloss 34.887 36.104 35.377 35.040

proposed 35.540 36.394 35.740 35.780
(g) sculpture2 GT (h) bicubic PSNR=33.425 (i) proposed PSNR= 35.740
Figure 3. Visualization of zero-shot 2× SR results The results in Table 1 show that the proposed model with
coarse-to-fine deformable convolutions achieved the most
The results have been quantized to 0~255 to fit the intensity competitive performance among the variants. Comparing to the
level of infrared radiation response. The PSNR values of the CNN variant w/o DCN, the proposed method is more robust in
proposed method are 1dB higher than the interpolation method array images with large disparities. Besides, the fine-level
in 2× zero-shot SR. It is also worth noting that the performance DCNs have a remarkable effect on the accuracy and the
of proposed method is stable in different texture complexity introduction of reverse loss makes it easier to achieve the
conditions. The proposed method can be applied to SR task of optimal results.
different scales with appropriate size of input. For the 4× SR, as
the scaling ratio of the LR-to-LLR is the same with the HR-to- Apart from the framework design, the iteration setting also
LR, the input image patches is supposed to be 4 times larger than plays an important role in the SR performance. As in Figure 5,
the 2× input to maintain the size of LLR. As more detail the SR performance improves when the iteration number
information needs to be recovered in 4× SR, both the SR results increases in general, but it has a tendency of decrease over
of bicubic and our proposed method appear to be inferior to the 10000 iterations, which may due to the overfitting of the model
2×, but the proposed method is still ahead of the bicubic method. on LLR-to-LR pairs.

386

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore. Restrictions apply.

<!-- page: 4 -->
[9] Wuyi Wang. Research on Infrared Super-Resolution Algorithm Based on
35.8 Generative Adversarial Network. University of Electronic Science and
Technology of China, 2021.
[10] Youngjin Yoon, Hae-Gon Jeon, Donggeun Yoo, Joon-Young Lee, and In PSNR(dB) 35.3 So Kweon. Learning a deep convolutional network for light-field image
super-resolution. In IEEE International Conference on Computer Vision
Workshops (ICCVW), pages 24–32, 2015. 34.8
[11] Henry Wing Fung Yeung, Junhui Hou, Xiaoming Chen, Jie Chen, Zhibo
Chen, and Yuk Ying Chung. Light field spatial super-resolution using
34.3 deep efficient spatial-angular separable convolution. IEEE Transactions
on Image Processing, 28(5):2319–2330, 2018.
[12] Shuo Zhang, Youfang Lin, and Hao Sheng. Residual networks for light
33.8 field image super-resolution. In IEEE Conference on Computer Vision
0 5000 10000 15000 and Pattern Recognition (CVPR), pages 11046–11055, 2019.
iteration
[13] Yingqian Wang, Jungang Yang, Longguang Wang, Xinyi Ying, Tianhao
Figure 5. The 2× results in different iteration setting of scene sculpture3 Wu, Wei An, and Yulan Guo. Light field image super-resolution using
deformable convolution. IEEE Transactions on Image Processing, 2020.
In addition, the overall runtime is prolonged when the [14] Yingqian Wang, Longguang Wang, Gaochang Wu, Jungang Yang, Wei
iteration number increases, therefore, 10000 appears to be the An, Jingyi Yu, and Yulan Guo. Disentangling light fields for super-
most suitable iteration number. Overall, the ablation results resolution and disparity estimation. IEEE Transactions on Pattern Analysis and Machine Intelligence, pages 425–443, 2023.
demonstrate the rationality of our strategy.
[15] Zhengyu Liang, Yingqian Wang, Longguang Wang, Jungang Yang, and
Shilin Zhou. Light field image super-resolution with transformers. IEEE
IV. CONCLUSION Signal Processing Letters, pages 563–567, 2022.
[16] Zhengyu Liang, Yingqian Wang, Longguang Wang, Jungang Yang, Shilin
This paper proposes a zero-shot infrared array camera image Zhou, and Yulan Guo. Learning non-local spatial-angular correla-tion for
super-resolution method. By down-sampling the input data, we light field image super-resolution. 2023 IEEE/CVF International
generate the LLR images for initial model parameter training, Conference on Computer Vision (ICCV), 2023.
where the input LR images serve as the reference to facilitate [17] Ruixuan Cong, Hao Sheng, Da Yang, Zhenglong Cui, and Rongshan Chen.
the optimization process. With the coarse weight, we extract the Exploiting spatial and angular correlations with deep efficient
features from the input image and enhance the performance transformers for light field image super-resolution. IEEE Transactions on
Multimedia, pages 1–14, 2023.
through iterations. The feature extraction module incorporates
[18] Yingqian Wang, Longguang Wang, Zhengyu Liang, Jungang Yang, and
both spatial and angular features, utilizing the DCNs to expand et al. Ntire 2023 challenge on light field image super-resolution: Dataset,
the receptive field. We also acquire a real-world infrared dataset methods and results. In 2023 IEEE/CVF Conference on Computer Vision
utilizing a 3×3 camera array for the evaluation of our model. and Pattern Recognition Workshops (CVPRW), 2023.
The experimental results indicate that our method can attain [19] Wentao Chao, Fuqing Duan, Xuechun Wang, Yingqian Wang, and
competitive super-resolution performance even with a single Guanghui Wang. Lfsrdiff: Light field image super-resolution via diffusion
shot. models, arXiv 2023.
[20] Ruisheng Gao, Yutong Liu, Zeyu Xiao, and Zhiwei Xiong. Diffusion-
based light field synthesis, arXiv 2024.
REFERENCES [21] Ruisheng Gao, Zeyu Xiao, and Zhiwei Xiong. Mamba-based light field
[1] Renke Kou, Chunping Wang, Zhenming Peng, Zhihe Zhao, Yaohong super-resolution with efficient subspace scanning, arXiv 2024.
Chen, Jinhui Han, Fuyu Huang, Ying Yu, Qiang Fu. Infrared small target [22] Wang xia, Yao Lu, Shunzhou Wang, Ziqi Wang, Peiqi Xia, and Tianfei
segmentation networks: A survey. Pattern Recognition, 2023. Zhou. Lfmamba: Light field image super-resolution with state space
[2] Di Yuan, Haiping Zhang, Xiu Shu, Qiao Liu, Xiaojun Chang, Zhenyu He, model, arXiv 2024.
Guangming Shi. Thermal Infrared Target Tracking: A Comprehensive [23] Sven Wanner, Stephan Meister, and Bastian Goldluecke. Datasets and
Review. IEEE Transactions on Instrumentation and Measurement, 2024. benchmarks for densely sampled 4d light fields. In Vision, Modelling and
[3] Junhong Li, Ping Zhang, Xiaowei Wang, Shize Huang. Infrared Small- Visualization (VMV), volume 13, pages 225–226, 2013.
Target Detection Algorithms: A Survey. Journal of Image and Graphics, [24] Katrin Honauer, Ole Johannsen, Daniel Kondermann, and Bastian
2020. Goldluecke. A dataset and evaluation methodology for depth estimation
[4] Tengfei Wang, Yutian Fu. Design of Uncooled Long Wave Infrared Light on4d light fields. In Asian Conference on Computer Vision (ACCV),
Field Camera Imaging System. 2019 Symposium and Interdisciplinary pages19–34, 2016.
Forum on Infrared and Remote Sensing Technologies and Applications, [25] Martin Rerabek and Touradj Ebrahimi. New light field image dataset. In
2019. International Conference on Quality of Multimedia Experience
[5] Feiyue Zhu. Design of Far Infrared Light Field Imaging System and 3D (QoMEX),2016.
Reconstruction. Zhejiang University, 2021. [26] Mikael Le Pendu, Xiaoran Jiang, and Christine Guillemot. Light field
[6] Xiaohua Liao, Niannian Chen, Yong Jiang, Shifeng Qi. Infrared Image inpainting propagation via low rank matrix completion. IEEE
Super-resolution Using Improved Convolutional Neural Network. Transactions on Image Processing, 27(4):1981–1993, 2018.
Infrared Technology, 2020. [27] Vaibhav Vaish and Andrew Adams. The (new) stanford light field
[7] Shuo Huang, Yong Hu, Mingjian Gu, Cailan Gong, Fuqiang Zheng. archive.Computer Graphics Laboratory, Stanford University, 6(7), 2008.
Super-Resolution Infrared Remote-Sensing Target-Detection Algorithm [28] Xi Cheng, Zhenyong Fu, Jian Yang. Zero-Shot Image Super-Resolution
Based on Deep Learning. Laser and Optoelectronics Progress, 2021. with Depth Guided Internal Degradation Learning. In 2020 European
[8] Baotai Shao. Research on Infrared Super-Resolution Imaging and Small Conference on Computer Vision (ECCV), 2020.
Target Classification Technology. University of Chinese Academy of [29] Zhen Cheng, Zhiwei Xiong, Chang Chen, Dong Liu, Zheng-Jun Zha.
Sciences, 2019. Light field super-resolution with zero-shot learning. In 2021 IEEE/CVF
Conference on Computer Vision and Pattern Recognition (CVPR), 2021.

387

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore. Restrictions apply.
