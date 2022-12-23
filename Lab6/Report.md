# Assignment 6: Mass-Spring Simulation

|  姓名  |   学号   |
| :----: | :------: |
| 黄灿彬 | 20337039 |

## Task 1 使用显式积分法实现质点弹簧系统的动画仿真

只需根据

$$
\boldsymbol{f}_{ij}=k\left(l_{ij}-\|\boldsymbol{x}_i-\boldsymbol{x}_j\|\right){\boldsymbol{x}_i-\boldsymbol{x}_j\over\|\boldsymbol{x}_i-\boldsymbol{x}_j\|},j=0,1,\cdots,i-1,i+1,\cdots,N
$$

求出与质点 $i$ 相连的其他质点对 $i$ 的作用力，并将它们求和，再加上 $i$ 本身受到的重力，就能够得到 $i$ 受到的合力 $\boldsymbol{f}_i$。

然后再根据

$$
\boldsymbol{v}_{t+1}=\boldsymbol{v}_t+\Delta t{\boldsymbol{f}\over m_t}
$$

$$
\boldsymbol{x}_{t+1}=\boldsymbol{x}_t+\Delta t\boldsymbol{v}_t
$$

更新 $i$ 的速度和位置即可。

代码如下：

```C++
static constexpr float dt = 0.0001f;
static glm::vec2 gravity = glm::vec2(0.0f, -9.8f);

for (unsigned int i = 0; i < m_numParticles; ++i) {
    glm::vec2 force = gravity * m_particleMass;

    for (unsigned int j = 0; j < m_numParticles; ++j) {
        if (m_restLength[i][j] != 0) {
            force += m_stiffness * (m_restLength[i][j] - glm::distance(m_x[i], m_x[j])) * (m_x[i] - m_x[j]) / glm::distance(m_x[i], m_x[j]);
        }
    }
    // Update the m_x[i]
    if (i != 0) m_x[i] += dt * m_v[i];
    // Update the m_v[i]
    m_v[i] += dt * force / m_particleMass;
}
```

结果如下：

![1671799568684](assets/1671799568684.png)

[演示视频](./Video/Task1.mp4)