# ios_shadow_tank
Shadow Tank PNG Based on iOS PNG Decoder with iDOT processor.



## 特别鸣谢

- 感谢 `cppascalinux` 大佬分析出了 PNG 交错的基本实现思路
- 感谢 `HocRiser` 大佬协助测试 `MacOS` 上的显示效果



## 图片示例

- 下图 `sample.png`，在 `iOS` 以及 `MacOS` 平台会观察到文字 `Hello, Apple!`
- 在其他平台上，会观察到文字 `Hello, Android!`

![](./img/sample.png)



## 灵感来源

- 这个算法并不是我们首创的，由于我们在网上收集到过以下图片，详见 `img` 文件夹下的 `p1.png`,  `p2.png`,  `p3.png`
- 以下图片在 `iOS` 和 `MacOS` 上能够观察到该图片裸体的版本，其他平台能看到穿衣服的版本

<img src="./img/p1.png" style="width: 300px">

- 由于我们并没有 Google 到相关的开源实现或博客，因此我们独立完成了该算法的分析与实现，并进行了代码开源



## 前置条件

- 安装 `python3` 
- 安装 `python3` 模块 `pillow` 以及 `tqdm`
- 有 `zlib` 库



## 使用方法

