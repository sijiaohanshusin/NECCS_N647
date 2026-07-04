2024 7th International Conference on Computer Information Science and Application Technology (CISAT)

## Infrared Camera Array Image Super-Resolution based on Zero-Shot Learning

Qianyu Chen National University of Defense Technology, China chenqianyu18@nudt.edu.cn

Jungang Yang* Wei An National University of Defense National University of Defense Technology, China Technology, China yangjungang@nudt.edu.cn anwei@nudt.edu.cn

_**Abstract**_ **—The infrared camera array is capable of capturing infrared features in a single scene from multiple views, thereby supplying rich angular information for a range of post-capture tasks, one of which is the image super-resolution (SR). Since the spatial resolution of each sub-image is limited by the diffraction effects, prevalent SR methodologies endeavor to improve image quality by acquiring prior knowledge from extensive datasets. However, the scarcity of the publicly available infrared array image dataset makes it difficult to rely on conventional model training strategies. To address this, inspired by the recent advance in zero-shot learning and light field image SR, this paper proposes a zero-shot infrared array image SR method. Specifically, we design an array feature extraction module (AFEM) to incorporate latent correlations within views and employ a zero-shot training strategy to enable model learning with one input low-resolution scene. Experiments show that the proposed method can achieve reliable performance in both objective metrics and visual effects.**

_**Keywords-Infrared image; camera array; zero-shot; superresolution**_

## I. INTRODUCTION

Infrared camera array can record radiation features from different views in all light conditions and provide rich complementary information for post-capture applications like infrared target segmentation [1], tracking [2] and detection [3]. However, the acquisition of high-quality infrared array images can be difficult due to the diffraction effects, noises and the inhomogeneity of sub-lens [4]-[5]. Inspired by the utilization of super-resolution (SR) techniques in single images, stereo images and videos, the SR of infrared image also becomes research in the spotlight.

Early infrared image SR approaches mainly rely on the interpolation, MAP, IBP and POCS methods. Driven by the trend of machine learning, recent researches have employed some basic deep learning frameworks for infrared feature extraction and reconstruction. For instance, Liao _et al._ [6] proposed an CNN-based infrared image SR method that introduce grey scale transformation to SRCNN. Huang _et al._ [7] employed WDSR for infrared remote sensing image SR, and evaluated the SR performance by subsequent target detection accuracy. Shao [8] proposed IRSR that based on residual learning. Wang [9] introduced the ESRGAN to single infrared image SR. While these methods can demonstrate reliability with single-view infrared images, their effectiveness in the context of infrared array camera images is somewhat constrained due to the challenges associated with the insufficient decoupling of highdimensional correlations.

Since the images of infrared camera array can be generalized to the light field (LF) data with large disparities, the successful trials of disentangling spatial, angular and epipolar features in LF image SR are also available for references [18]. The state-ofthe art LF image SR methods generally separate the highdimensional LF data into sub-dimensions and employ the most advanced deep learning architecture including the variants of convolutions [10]-[14], Transformer [15]-[17], diffusion [19][20] and Mamba [21]-[22]. Their SR results in both the realworld and synthetic public LF dataset HCI_old [23], HCI_new [24], EPFL [25], INRIA [26] and STFGantry [27] are appealing, however, they cannot be directly migrated to the infrared case due to the domain gap between infrared and visible light. Additionally, the scarcity of publicly available infrared light field data presents a challenge in addressing this gap during the training process.

To address these, we propose a zero-shot infrared array camera image super-resolution method, and to our best knowledge, it is a pioneering method for the infrared array images SR. Inspired by the recent advance in zero sample learning in both single image SR [28] and LF image SR [29], we train the array feature extraction module with the lower-low image pairs and test the performance with low-high image pairs in each scene. The main contributions of this paper can be summarized as:

-We propose an infrared array image super-resolution method to fit the large disparity issue in array cameras.

-We propose a zero-shot strategy for the infrared array image SR to deal with the shortage of infrared array image dataset.

-Experiments show that the proposed method can achieve remarkable performance in the real-world infrared array images.

## II. METHOD

## _A. Preliminary_

The input low-resolution (LR) images of the infrared camera arrays can be represented as ��� ∈ℝ[�×�×�×�] , in which the �× � refers to the sub-lens array plane of the camera array, and �× � refers to the pixel plane in each sub-image. The array image SR techniques aim at generating the super-resolved images ��� ∈ℝ[�×�×��×��] based on the input data ���, where the � indicates the up-scaling factor. During the training phase of the zero-shot pipeline, the ��� is also down-sampled to get � ~~×~~[�] � ���� ∈ℝ[�×�×] �[�] � for the model optimization. The overall pipeline of the proposed method is shown in Figure 1.

979-8-3503-7510-7/24/$31.00 ©2024 IEEE

384

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore.  Restrictions apply.

**==> picture [438 x 78] intentionally omitted <==**

**==> picture [438 x 78] intentionally omitted <==**

Figure 1. The overall pipeline of proposed model

## _B. Array Feature Extraction Module_

Different from the common light field that are captured by micro-lens, the infrared camera array images have wider baselines and requires a larger receptive field for feature extraction and alignment.

Inspired by the advantages of deformable convolution (DCNs) in enlarging the receptive field [13], we design an array feature extraction module (AFEM) based on DCNs. Specifically, the input low-resolution (LR) or lower than low-resolution (LLR) images are sent to the initial CNNs ( _Conv_ ) and residual CNNs with dilations ( _ResConv_ ) for feature extraction.

## ℱ���� = �������(�������(����(��� )))

The features are separated to the center-view ℱ������ and sideview groups ℱ������ , and sent to the DCN-based feature extraction process. During this process, the features are aligned, fused and collected in DCN coarse stage1, and then distributed to each view by DCN coarse stage2 to generate ℱ���. The following fine-level DCNs are similar to the coarse ones and generate ℱ��� based on the input ℱ���. The extracted features go through the information fusion convolutions and up-sampling pixel-shuffle to form the final SR results.

The coarse offset � is obtained by conducting convolutions ����� on each side-view feature ℱ[��] and the center view feature ℱ[��] in stage 1, and the fine-level � is calculated by the fused feature ℱ��and previous side-view feature in ������. For example, the formulation of the DCN coarse stages can be represented as:

> �� , ����),  ���� = �����(ℱ������ , ℱ������ ) �[ℱ] ℱ[���] ���[= ���] = ���[��] ��[(ℱ] (ℱ[����] ���, ����),  ���� = ������(ℱ����, ℱ���) The number of DCN-based feature extraction operation is set to one in this paper, and can be increased in a stacked manner to improve the performance in complex scenarios.

## _C. Zero-Shot Strategy_

Due to the difficulty and high cost of acquiring large number of infrared array image data, we try to implement our model in a zero-shot manner. To be specific, we take each input ��� as the reference of the SR results of ����, and optimize the parameter weights based on the LLR-to-LR loss.

During the model training iterations, the ��� is downsampled to get ����, and the array feature extraction module is trained with the feedback of loss evaluation. The loss evaluation consists of the forward loss (the direct mean square error between the super-resolved result ����→�� and the reference ���) and the reverse loss (the MSE value of the down-sampled SR result (����→�� )↓ and the original input ���� ). We also introduce a factor � to adjust the proportion of reverse loss in the overall loss function.

## ����= ���(���, ����→��) + �∗���(����, (����→��)↓)

Once the training iteration status meets the maximum setting or the loss reach the threshold, the AFEM loads the parameter weights and takes the ��� as the input for test. The model weights can be recorded and optimized in the training and test phase of different scenes. Therefore, our pipeline can not only perform super-resolution in the case of only one sample, but also able to optimize the performance with a small number of samples.

## III. EXPERIMENTS

## _A. Dataset Collection_

Since the infrared array image datasets are not yet publicly available currently, we collect a few scenes with a 3 × 3 infrared camera array and verify the effectiveness of the proposed method. As it is shown in Figure 2, the infrared cameras are oriented differently and equally spaced. The settings of each sublens are the same, and the internal and external parameters are pre-calibrated.

**==> picture [165 x 60] intentionally omitted <==**

Figure 2. Toy model of the data collection method

During the data collection, the band of the infrared camera array is 8~14μm and the frame size of each sub-image is set to 640×512. In order to evaluate the performance of our model in recovering the details of images, we capture some sculptures and buildings that have infrared texture features.

385

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore.  Restrictions apply.

## _B. Implementation Details_

In order to evaluate the super-resolution effect of the proposed model, we took the original array images as the references and the 0.5× and 0.25× downsampled image pairs as the input LR images of the 2× and 4× SR model. The input images were divided into patches for model training iterations, 64 × 64 for 2 ×  SR, 256 × 256 for 4 ×  SR, and the data augmentation strategy was also applied to these patches to avoid overfitting. The proposed framework was implemented in PyTorch and run with a laptop with Nvidia GTX 3060. The maximum iteration number was set to 10000, and it could be early stopped when the loss met the threshold. During the iterations, we employed the Adam optimizer with an initial learning rate of 2e-4 and a decrease factor of 0.5 for every 2500 iteration. The test stage the input LR images would start when the iteration ended, and the SR patches would be integrated for the final result.

## _C. Results and Ablation Study_

_a) Results:_ We compare the proposed model with the traditional interpolation method in real-world datasets. Part of the 2× and 4× SR results are shown in Figure 3 and 4. The peaksignal-to-noise value (PSNR) with unit dB has also been calculated to measure the SR effect objectively.

**==> picture [78 x 62] intentionally omitted <==**

**==> picture [78 x 62] intentionally omitted <==**

**==> picture [79 x 62] intentionally omitted <==**

**==> picture [237 x 163] intentionally omitted <==**

**----- Start of picture text -----**<br>
(a) buildings GT  (b) bicubic PSNR=34.169  (c) proposed PSNR=35.540<br>(d) sculpture1 GT  (e) bicubic PSNR=34.320  (f) proposed PSNR=36.394<br>(g) sculpture2 GT  (h) bicubic PSNR=33.425  (i) proposed PSNR= 35.740<br>Figure 3. Visualization of zero-shot 2× SR results<br>**----- End of picture text -----**<br>


The results have been quantized to 0~255 to fit the intensity level of infrared radiation response. The PSNR values of the proposed method are 1dB higher than the interpolation method in 2× zero-shot SR. It is also worth noting that the performance of proposed method is stable in different texture complexity conditions. The proposed method can be applied to SR task of different scales with appropriate size of input. For the 4× SR, as the scaling ratio of the LR-to-LLR is the same with the HR-toLR, the input image patches is supposed to be 4 times larger than the  2 ×  input to maintain the size of LLR. As more detail information needs to be recovered in 4× SR, both the SR results of bicubic and our proposed method appear to be inferior to the 2×, but the proposed method is still ahead of the bicubic method.

**==> picture [78 x 62] intentionally omitted <==**

**==> picture [79 x 62] intentionally omitted <==**

**==> picture [78 x 62] intentionally omitted <==**

**==> picture [237 x 163] intentionally omitted <==**

**----- Start of picture text -----**<br>
(a) buildings GT  (b) bicubic PSNR=29.747  (c) proposed PSNR= 30.490<br>(d) sculpture1 GT  (e) bicubic PSNR=29.120  (f) proposed PSNR=29.493<br>(g) sculpture2 GT  (h) bicubic PSNR=28.114  (i) proposed PSNR= 28.821<br>Figure 4. Visualization of zero-shot 4× SR results<br>**----- End of picture text -----**<br>


_b) Ablation:_ In order to estimate the effectiveness of the proposed framework, we trained and tested the variants of the proposed method. All variants employed the same zero-shot learning strategy in the implementation subsection. The variant w/o DCN substitutes DCN with the conventional CNNs for array feature extraction in the same coarse-to-fine settings. Variant w/o DCNFS removes the fine-level DCNs and the variant w/o rloss subtracts the proportion of reverse loss in the overall loss evaluation.

TABLE I. THE PSNR RESULTS OF THE VARIANTS

|variants||PSNR 2×|PSNR 2×||
|---|---|---|---|---|
||buildings|sculpture1|sculpture2|sculpture3|
|bicubic|34.169|34.320|33.425|33.706|
|w/o DCN|35.135|36.152|35.327|35.059|
|w/o DCNFS|35.007|36.090|35.235|35.074|
|w/o rloss|34.887|36.104|35.377|35.040|
|proposed|35.540|36.394|35.740|35.780|



The results in Table 1 show that the proposed model with coarse-to-fine deformable convolutions achieved the most competitive performance among the variants. Comparing to the CNN variant w/o DCN, the proposed method is more robust in array images with large disparities. Besides, the fine-level DCNs have a remarkable effect on the accuracy and the introduction of reverse loss makes it easier to achieve the optimal results.

Apart from the framework design, the iteration setting also plays an important role in the SR performance. As in Figure 5, the SR performance improves when the iteration number increases in general, but it has a tendency of decrease over 10000 iterations, which may due to the overfitting of the model on LLR-to-LR pairs.

386

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore.  Restrictions apply.

**==> picture [242 x 138] intentionally omitted <==**

**----- Start of picture text -----**<br>
35.8<br>35.3<br>34.8<br>34.3<br>33.8<br>0 5000 10000 15000<br>iteration<br>PSNR(dB)<br>**----- End of picture text -----**<br>


Figure 5. The 2× results in different iteration setting of scene sculpture3

In addition, the overall runtime is prolonged when the iteration number increases, therefore, 10000 appears to be the most suitable iteration number. Overall, the ablation results demonstrate the rationality of our strategy.

## IV. CONCLUSION

This paper proposes a zero-shot infrared array camera image super-resolution method. By down-sampling the input data, we generate the LLR images for initial model parameter training, where the input LR images serve as the reference to facilitate the optimization process. With the coarse weight, we extract the features from the input image and enhance the performance through iterations. The feature extraction module incorporates both spatial and angular features, utilizing the DCNs to expand the receptive field. We also acquire a real-world infrared dataset utilizing a 3×3 camera array for the evaluation of our model. The experimental results indicate that our method can attain competitive super-resolution performance even with a single shot.

- [9] Wuyi Wang. Research on Infrared Super-Resolution Algorithm Based on Generative Adversarial Network. University of Electronic Science and Technology of China, 2021.

- [10] Youngjin Yoon, Hae-Gon Jeon, Donggeun Yoo, Joon-Young Lee, and In So Kweon. Learning a deep convolutional network for light-field image super-resolution. In IEEE International Conference on Computer Vision Workshops (ICCVW), pages 24–32, 2015.

- [11] Henry Wing Fung Yeung, Junhui Hou, Xiaoming Chen, Jie Chen, Zhibo Chen, and Yuk Ying Chung. Light field spatial super-resolution using deep efficient spatial-angular separable convolution. IEEE Transactions on Image Processing, 28(5):2319–2330, 2018.

- [12] Shuo Zhang, Youfang Lin, and Hao Sheng. Residual networks for light field image super-resolution. In IEEE Conference on Computer Vision and Pattern Recognition (CVPR), pages 11046–11055, 2019.

- [13] Yingqian Wang, Jungang Yang, Longguang Wang, Xinyi Ying, Tianhao Wu, Wei An, and Yulan Guo. Light field image super-resolution using deformable convolution. IEEE Transactions on Image Processing, 2020.

- [14] Yingqian Wang, Longguang Wang, Gaochang Wu, Jungang Yang, Wei An, Jingyi Yu, and Yulan Guo. Disentangling light fields for superresolution and disparity estimation. IEEE Transactions on Pattern Analysis and Machine Intelligence, pages 425–443, 2023.

- [15] Zhengyu Liang, Yingqian Wang, Longguang Wang, Jungang Yang, and Shilin Zhou. Light field image super-resolution with transformers. IEEE Signal Processing Letters, pages 563–567, 2022.

- [16] Zhengyu Liang, Yingqian Wang, Longguang Wang, Jungang Yang, Shilin Zhou, and Yulan Guo. Learning non-local spatial-angular correla-tion for light field image super-resolution. 2023 IEEE/CVF International Conference on Computer Vision (ICCV), 2023.

- [17] Ruixuan Cong, Hao Sheng, Da Yang, Zhenglong Cui, and Rongshan Chen. Exploiting spatial and angular correlations with deep efficient transformers for light field image super-resolution. IEEE Transactions on Multimedia, pages 1–14, 2023.

- [18] Yingqian Wang, Longguang Wang, Zhengyu Liang, Jungang Yang, and et al. Ntire 2023 challenge on light field image super-resolution: Dataset, methods and results. In 2023 IEEE/CVF Conference on Computer Vision and Pattern Recognition Workshops (CVPRW), 2023.

- [19] Wentao Chao, Fuqing Duan, Xuechun Wang, Yingqian Wang, and Guanghui Wang. Lfsrdiff: Light field image super-resolution via diffusion models, arXiv 2023.

- [20] Ruisheng Gao, Yutong Liu, Zeyu Xiao, and Zhiwei Xiong. Diffusionbased light field synthesis, arXiv 2024.

## REFERENCES

- [1] Renke Kou, Chunping Wang, Zhenming Peng, Zhihe Zhao, Yaohong Chen, Jinhui Han, Fuyu Huang, Ying Yu, Qiang Fu. Infrared small target segmentation networks: A survey. Pattern Recognition, 2023.

- [2] Di Yuan, Haiping Zhang, Xiu Shu, Qiao Liu, Xiaojun Chang, Zhenyu He, Guangming Shi. Thermal Infrared Target Tracking: A Comprehensive Review. IEEE Transactions on Instrumentation and Measurement, 2024.

- [3] Junhong Li, Ping Zhang, Xiaowei Wang, Shize Huang. Infrared SmallTarget Detection Algorithms: A Survey. Journal of Image and Graphics, 2020.

- [4] Tengfei Wang, Yutian Fu. Design of Uncooled Long Wave Infrared Light Field Camera Imaging System. 2019 Symposium and Interdisciplinary Forum on Infrared and Remote Sensing Technologies and Applications, 2019.

- [5] Feiyue Zhu. Design of Far Infrared Light Field Imaging System and 3D Reconstruction. Zhejiang University, 2021.

- [6] Xiaohua Liao, Niannian Chen, Yong Jiang, Shifeng Qi. Infrared Image Super-resolution Using Improved Convolutional Neural Network. Infrared Technology, 2020.

- [7] Shuo Huang, Yong Hu, Mingjian Gu, Cailan Gong, Fuqiang Zheng. Super-Resolution Infrared Remote-Sensing Target-Detection Algorithm Based on Deep Learning. Laser and Optoelectronics Progress, 2021.

- [8] Baotai Shao. Research on Infrared Super-Resolution Imaging and Small Target Classification Technology. University of Chinese Academy of Sciences, 2019.

- [21] Ruisheng Gao, Zeyu Xiao, and Zhiwei Xiong. Mamba-based light field super-resolution with efficient subspace scanning, arXiv 2024.

- [22] Wang xia, Yao Lu, Shunzhou Wang, Ziqi Wang, Peiqi Xia, and Tianfei Zhou. Lfmamba: Light field image super-resolution with state space model, arXiv 2024.

- [23] Sven Wanner, Stephan Meister, and Bastian Goldluecke. Datasets and benchmarks for densely sampled 4d light fields. In Vision, Modelling and Visualization (VMV), volume 13, pages 225–226, 2013.

- [24] Katrin Honauer, Ole Johannsen, Daniel Kondermann, and Bastian Goldluecke. A dataset and evaluation methodology for depth estimation on4d light fields. In Asian Conference on Computer Vision (ACCV), pages19–34, 2016.

- [25] Martin Rerabek and Touradj Ebrahimi. New light field image dataset. In International Conference on Quality of Multimedia Experience (QoMEX),2016.

- [26] Mikael Le Pendu, Xiaoran Jiang, and Christine Guillemot. Light field inpainting propagation via low rank matrix completion. IEEE Transactions on Image Processing, 27(4):1981–1993, 2018.

- [27] Vaibhav Vaish and Andrew Adams. The (new) stanford light field archive.Computer Graphics Laboratory, Stanford University, 6(7), 2008.

- [28] Xi Cheng, Zhenyong Fu, Jian Yang. Zero-Shot Image Super-Resolution with Depth Guided Internal Degradation Learning. In 2020 European Conference on Computer Vision (ECCV), 2020.

- [29] Zhen Cheng, Zhiwei Xiong, Chang Chen, Dong Liu, Zheng-Jun Zha. Light field super-resolution with zero-shot learning. In 2021 IEEE/CVF Conference on Computer Vision and Pattern Recognition (CVPR), 2021.

387

Authorized licensed use limited to: Harvard University SEAS. Downloaded on March 31,2026 at 10:51:41 UTC from IEEE Xplore.  Restrictions apply.
