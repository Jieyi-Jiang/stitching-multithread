# Reading: OpenCV stitching_detail.cpp prompt

```C++
"Rotation model images stitcher.\n\n"
```
“基于旋转模型的图像拼接” -- 不考虑平移，只考虑旋转，不适合相机有明显平移的场景

 ```C++
 << argv[0] << " img1 img2 [...imgN] [flags]\n\n"
"Flags:\n"
"  --preview\n"
"      Run stitching in the preview mode. Works faster than usual mode,\n"
"      but output image will have lower resolution.\n"
```
**预览模式**，更快但分辨率低一点。

```C++
"  --try_cuda (yes|no)\n"
"      Try to use CUDA. The default value is 'no'. All default values\n"
"      are for CPU mode.\n"
```
**尝试使用CUDA**，默认不使用CUDA，所有默认值都是为CPU模式准备的。

```C++
"\nMotion Estimation Flags:\n"
```
运动预测Flag

```C++
"  --work_megapix <float>\n"
"      Resolution for image registration step. The default is 0.6 Mpx.\n"
```
**工作百万像素数**（float值），图像配准步骤的分辨率。默认为0.6百万像素。
```C++
"  --features (surf|orb|sift|akaze)\n"
"      Type of features used for images matching.\n"
"      The default is surf if available, orb otherwise.\n"
```
**特征选项**，可选（surf|orb|sift|akaze），用于图像匹配的特征类型。默认为SURF特征（如果可用的话），
也可以使用 orb 等特征类型。
```C++
"  --matcher (homography|affine)\n"
"      Matcher used for pairwise image matching.\n"
```
**匹配器类型**，可选（homography | affine），即（单应性变换 | 仿射变换），用于成对匹配图像匹配。

## 单应性变换 
单应性变换即透视变换， <br>
相关链接 
> [知乎：单应性Homography估计：从传统算法到深度学习](https://zhuanlan.zhihu.com/p/74597564) <br>
> [Wiki: Homography (computer vision)](https://en.wikipedia.org/wiki/Homography_ (computer_vision)) <br>
> [Wiki: Homography](https://en.wikipedia.org/wiki/Homography) <br>

```C++
"  --estimator (homography|affine)\n"
"      Type of estimator used for transformation estimation.\n"
```
**预测器类型**，可选（homography|affine），用于变换预测<br>
问题：和匹配器类型有什么区别

```C++
"  --match_conf <float>\n"
"      Confidence for feature matching step. The default is 0.65 for surf and 0.3 for orb.\n"
```
**特征匹配置信度阈值**，float类型，特征匹配步骤的置信度阈值。surf默认为0.65,orb默认为0.3。

```C++
"  --conf_thresh <float>\n"
"      Threshold for two images are from the same panorama confidence.\n"
"      The default is 1.0.\n"
```
**置信度阈值** 两张照片为来自同一张全景图的置信度，默认为1.0

```C++
"  --ba (no|reproj|ray|affine)\n"
"      Bundle adjustment cost function. The default is ray.\n"
```
**捆绑调整**，可选项为（no|reproj|ray|affine），捆绑调整代价函数，默认为 ray。 <br>
这一步要重点关注。


```C++
"  --ba_refine_mask (mask)\n"
"      Set refinement mask for bundle adjustment. It looks like 'x_xxx',\n"
"      where 'x' means refine respective parameter and '_' means don't\n"
"      refine one, and has the following format:\n"
"      <fx><skew><ppx><aspect><ppy>. The default mask is 'xxxxx'. If bundle\n"
"      adjustment doesn't support estimation of selected parameter then\n"
"      the respective flag is ignored.\n"
```

**捆绑调整优化掩码** ，设置捆绑调整的优化掩码，形如 'x_xxx'，其中 'x' 表示优化相应的参数，'_' 表示不优化一个，格式如下：<br>
\<fx\>\<skew\>\<ppx\>\<aspect\>\<ppy\>。默认掩码为 'xxxxx'。如果捆绑调整不支持估计选定的参数，则忽略相应的标志。<br>
```text
fx - 焦距x方向参数
skew - 图像倾斜参数
ppx - 主点x坐标
aspect - 宽高比
ppy - 主点y坐标
```

```C++
"  --wave_correct (no|horiz|vert)\n"
"      Perform wave effect correction. The default is 'horiz'.\n"
```
**波形校正**，可选（no|horiz|vert），执行波形校正。默认为水平方向。<br>
这是干嘛用的？

```C++
"  --save_graph <file_name>\n"
"      Save matches graph represented in DOT language to <file_name> file.\n"
"      Labels description: Nm is number of matches, Ni is number of inliers,\n"
"      C is confidence.\n"
```
**保存匹配图**，将表示为DOT语言的匹配图保存到\<file_name\>文件中。标签描述：Nm是匹配数，Ni是内点数，C是置信度。

```C++
"\nCompositing Flags:\n"
```
合成Flag

```C++
"  --warp (affine|plane|cylindrical|spherical|fisheye|stereographic|compressedPlaneA2B1|compressedPlaneA1.5B1|compressedPlanePortraitA2B1|compressedPlanePortraitA1.5B1|paniniA2B1|paniniA1.5B1|paniniPortraitA2B1|paniniPortraitA1.5B1|mercator|transverseMercator)\n"
"      Warp surface type. The default is 'spherical'.\n"
```
表面扭曲类型，默认为 spherical (球形)
```Text
affine - 仿射变换
plane - 平面
cylindrical - 圆柱形
spherical - 球形
fisheye - 鱼眼
stereographic - 指数
compressedPlaneA2B1 - 压缩平面A2B1
compressedPlaneA1.5B1 - 压缩平面A1.5B1
compressedPlanePortraitA2B1 - 压缩平面竖屏A2B1
compressedPlanePortraitA1.5B1 - 压缩平面竖屏A1.5B1
paniniA2B1 - 潘尼尼A2B1
paniniA1.5B1 - 潘尼尼A1.5B1
paniniPortraitA2B1 - 潘尼尼竖屏A2B1
paniniPortraitA1.5B1 - 潘尼尼竖屏A1.5B1
```

```C++
"  --seam_megapix <float>\n"
"      Resolution for seam estimation step. The default is 0.1 Mpx.\n"
```
**缝合线估计分辨率**，float类型，缝合线估计步骤的分辨率。默认为0.1百万像素。

```C++
"  --seam (no|voronoi|gc_color|gc_colorgrad)\n"
"      Seam estimation method. The default is 'gc_color'.\n"
```
**缝合**，可选项为（no|voronoi|gc_color|gc_colorgrad）， 缝合线预测方法，默认为 `gc_color`（图割-颜色法）。

```C++
"  --compose_megapix <float>\n"
"      Resolution for compositing step. Use -1 for original resolution.\n"
"      The default is -1.\n"
```
**合成百万像素数**，设置合成步骤的分辨率，用-1表示原始分辨率，默认为`-1`，为原始分辨率。

~~~C++
"  --expos_comp (no|gain|gain_blocks|channels|channels_blocks)\n"
"      Exposure compensation method. The default is 'gain_blocks'.\n"
~~~
**曝光补偿**，可选项为（no|gain|gain_blocks|channels|channels_blocks）<br>
曝光补偿方法，默认为 `gain_blocks` <br>
~~~text
no - 不进行曝光补偿。
gain - 使用简单的增益补偿方法。
gain_blocks - 剑图像分块进行增益补偿（默认）
channels - 对每个颜色通道分别进行补偿。
channels_blocks - 将图像分块并对每个颜色通道分别进行补偿。
~~~

```C++
"  --expos_comp_nr_feeds <int>\n"
"      Number of exposure compensation feed. The default is 1.\n"
```
**曝光补偿馈送次数**，为int类型，曝光补偿步骤的运行次数，默认为 `1`

~~~C++
"  --expos_comp_nr_filtering <int>\n"
"      Number of filtering iterations of the exposure compensation gains.\n"
"      Only used when using a block exposure compensation method.\n"
"      The default is 2.\n"
~~~
**曝光补偿滤波次数**，为int类型，曝光补偿增益的迭代滤波次数。仅当使用方块滤波时生效，默认为 `2`.

```C++
"  --expos_comp_block_size <int>\n"
"      BLock size in pixels used by the exposure compensator.\n"
"      Only used when using a block exposure compensation method.\n"
"      The default is 32.\n"
```
**曝光补偿分块大小**，为int类型。曝光补偿器使用的像素块的大小。仅当使用分块曝光补偿的时候生效，默认为32。

```C++
"  --blend (no|feather|multiband)\n"
"      Blending method. The default is 'multiband'.\n"
```
**混合/融合方式**，可选项为（no|feather|multiband），融合方法，默认为 `multiband` 方法。
```text
no - 不进行融合处理。
feather - 羽化融合，通过渐变的方式处理重叠区域。
multiband -多频段融合方法（默认），使用拉普拉斯金字塔进行融合。
```
```c++
"  --blend_strength <float>\n"
"      Blending strength from [0,100] range. The default is 5.\n"
```
**融合强度**，融合强度设定值的范围为 [0, 100]，默认为5。

```c++
"  --output <result_img>\n"
"      The default is 'result.jpg'.\n"
```
**输出**，指定输出图片的名称和路径，默认为 `result.jpg`。

```C++
"  --timelapse (as_is|crop) \n"
"      Output warped images separately as frames of a time lapse movie, with 'fixed_' prepended to input file names.\n"
```
**延时摄影**，可选项为（as_is|crop）。将扭曲的图像分别输出为延时摄影帧序列，并在`输入文件名`前添加‘fixed_’前缀作为`输出文件名`。

```text
​as_is -​ 保留图像原始边界（含黑色填充区域）, fixed_img1.jpg（全幅扭曲结果）
​crop​ - 自动裁剪无效区域（移除黑边），仅保留有效像素, fixed_img1.jpg（仅有效内容区）
```

```C++
"  --rangewidth <int>\n"
"      uses range_width to limit number of images to match with.\n";
```
**宽度范围**，为int类型，通过 range_width 值限制参与匹配的图像数量。
