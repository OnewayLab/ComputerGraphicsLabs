# Assignment 0: Introduction to OpenGL

|  姓名  |   学号   |
| :----: | :------: |
| 黄灿彬 | 20337039 |

## Task 1 

**什么是 OpenGL？OpenGL 与计算机图形学的关系是什么？**

OpenGL（全称为Open Graphics Library） 是用于渲染2D、3D矢量图形的跨语言、跨平台的应用程序编程接口（API）。OpenGL 的实现以计算机图形学的渲染理论为基础。

## Task 2

**完成了着色器章节之后，请修改顶点着色器让三角形上下颠倒。**

只需在顶点着色器中将 y 轴坐标取相反数即可：

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

void main()
{
    gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);
    ourColor = aColor;
}
```

![image-20220904233502034](assets/image-20220904233502034.png)

## Task 3

**完成了纹理章节之后，尝试用不同的纹理环绕方式，设定一个从 0.0f 到 2.0f 范围内的（而不是原来的 0.0f 到 1.0f ）纹理坐标。试试看能不能在箱子的角落放置4个笑脸。记得一定要试试其它的环绕方式。简述原因并贴上结果。**

如果仅仅将当前的纹理坐标中的 `1.0f` 改成 `2.0f`，由于箱子和笑脸两个纹理单元对应同一个纹理坐标，最终实现效果是四个笑脸箱子，而不是在一个箱子中放 4 个笑脸。因此，我们需要设置两个纹理坐标，一个对应箱子纹理，一个对应笑脸纹理。

首先在顶点属性中加入笑脸的纹理坐标，范围从 `0.0f` 到 `2.0f`：

```C++
float vertices[] = {
    // positions         colors             box coords   face coords
     0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  2.0f, 2.0f, // top right
     0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  2.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 2.0f  // top left
};
```

注意还要修改顶点属性的链接：

```C++
// position attribute
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)0);
glEnableVertexAttribArray(0);
// color attribute
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
// box texture coord attribute
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(6 * sizeof(float)));
glEnableVertexAttribArray(2);
// face texture coord attribute
glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(8 * sizeof(float)));
glEnableVertexAttribArray(3);
```

然后在顶点着色器中将两个纹理坐标传递给片段着色器：

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aBoxTexCoord;
layout (location = 3) in vec2 aFaceTexCoord;

out vec3 ourColor;
out vec2 BoxTexCoord;
out vec2 FaceTexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    BoxTexCoord = aBoxTexCoord;
    FaceTexCoord = aFaceTexCoord;
}
```

在片段着色器中将两个纹理坐标应用于对应的纹理：

```glsl
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 BoxTexCoord;
in vec2 FaceTexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, BoxTexCoord), texture(texture2, FaceTexCoord), 0.2);
}
```

最后修改环绕方式，观察不同环绕方式的效果。一共有 `GL_REPEAT`、`GL_MIRRORED_REPEAT`、`GL_CLAMP_TO_EDGE` 和 `GL_CLAMP_TO_BORDER` 四种环绕方式，可以对 s 轴和 t 轴分别设置，故一共有 16 种组合：

| s\t                      | `GL_REPEAT`                                                  | `GL_MIRRORED_REPEAT`                                         | `GL_CLAMP_TO_EDGE`                                           | `GL_CLAMP_TO_BORDER`                                         |
| ------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| **`GL_REPEAT`**          | ![image-20220906092139906](assets/image-20220906092139906.png) | ![image-20220906092254218](assets/image-20220906092254218.png) | ![image-20220906092336314](assets/image-20220906092336314.png) | ![image-20220906092411317](assets/image-20220906092411317.png) |
| **`GL_MIRRORED_REPEAT`** | ![image-20220906092446108](assets/image-20220906092446108.png) | ![image-20220906092517499](assets/image-20220906092517499.png) | ![image-20220906092648601](assets/image-20220906092648601.png) | ![image-20220906092746834](assets/image-20220906092746834.png) |
| **`GL_CLAMP_TO_EDGE`**   | ![image-20220906093008250](assets/image-20220906093008250.png) | ![image-20220906092917161](assets/image-20220906092917161.png) | ![image-20220906092855397](assets/image-20220906092855397.png) | ![image-20220906092831624](assets/image-20220906092831624.png) |
| **`GL_CLAMP_TO_BORDER`** | ![image-20220906093216044](assets/image-20220906093216044.png) | ![image-20220906093517811](assets/image-20220906093517811.png) | ![image-20220906093538722](assets/image-20220906093538722.png) | ![image-20220906093557709](assets/image-20220906093557709.png) |

## Task 4

**完成了坐标系统章节之后，对GLM的projection函数中的FoV和aspect-ratio参数进行实验。看能否搞懂它们是如何影响透视平截头体的。**

FoV 是 `perspective` 函数的第一个参数，用来表示视野大小，即平截头体侧面两条楞的夹角，如下图所示：

![ perspective_frustum](assets/perspective_frustum.png)

教程中把 FoV 的值设为 45 度，效果如下：

![image-20220907215928568](assets/image-20220907215928568.png)

减小 FoV 即减小平截头体两条侧棱的夹角，从而减小底面积，即减小了视野，当窗口大小固定时，物体看起来被放大了，下面是将 FoV 减小到 20 度的效果：

![image-20220907220148402](assets/image-20220907220148402.png)

反之，增大 FoV 将增加视野中的东西，所以物体看起来被缩小了，下面是将 FoV 增大到 70 度的效果：

![image-20220907220758335](assets/image-20220907220758335.png)

aspect-ratio 是 perspective 函数的第二个参数，它设置了视野的宽高比，即平截头体底面的宽高比，教程中把它设为视口的宽除以高，此时即使视口并不是正方形，但我们依然能够将箱子的侧面正确渲染为正方形，效果如下：

![image-20220907221535509](assets/image-20220907221535509.png)

如果我们修改这个参数，使得平截头体的宽高比增大，也就是在横向上有更大的视野，那么我们在横向上就能够看到更多东西，但是当渲染的结果显示在视口上时，物体就像被横向压缩了一样：

![image-20220907221809794](assets/image-20220907221809794.png)

同理，如果减小平截头体的宽高比，物体就像在横向上被拉伸了：

![image-20220907221926150](assets/image-20220907221926150.png)

## Task 5

**请按照顺序将跟着教程实现的运行结果贴出来，要求将运行出来的窗口的标题改成自己的学号。（Tip：`glfwCreateWindow` 函数）**

1. 你好，窗口

   ![image-20220904125652403](assets/image-20220904125652403.png)

2. 你好，三角形

   ![image-20220904164610003](assets/image-20220904164610003.png)

   ![image-20220904170746458](assets/image-20220904170746458.png)

   ![image-20220904170818030](assets/image-20220904170818030.png)

3. 着色器

   ![image-20220904200235255](assets/image-20220904200235255.png)

   ![image-20220904221809858](assets/image-20220904221809858.png)

   ![image-20220904223826108](assets/image-20220904223826108.png)

4. 纹理

   ![image-20220905221020201](assets/image-20220905221020201.png)

   ![image-20220905221431052](assets/image-20220905221431052.png)

   ![image-20220905223919307](assets/image-20220905223919307.png)

5. 变换

   ![image-20220906131827627](assets/image-20220906131827627.png)

   ![image-20220906165440894](assets/image-20220906165440894.png)

   ![image-20220906172147097](assets/image-20220906172147097.png)

   ![image-20220906173536884](assets/image-20220906173536884.png)

6. 摄像机

   * 摄像机绕一个圆旋转：

     ![image-20220906201755631](assets/image-20220906201755631.png)

   * 使用键盘控制摄像机移动：

     ![image-20220906224321448](assets/image-20220906224321448.png)

   * 使用鼠标控制摄像机视角和缩放：

     ![image-20220906231514932](assets/image-20220906231514932.png)

     

   