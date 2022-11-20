# Assignment 4: Ray Tracing

|  姓名  |   学号   |
| :----: | :------: |
| 黄灿彬 | 20337039 |

## Task 1 完成射线类和简单的摄像机构建，并渲染一个渐变的蓝色天空背景图

### 射线类

射线的表示：

$$
\boldsymbol{P}(t)=\boldsymbol{A}+t\boldsymbol{b}
$$

其中，$\boldsymbol A$ 为射线的起点，$\boldsymbol b$ 是射线的方向向量。

根据射线的表示在 `ray.h` 中建立射线类：

```C++
#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
    ray() {}
    ray(const point3 &origin, const vec3 &direction) :
        orig(origin), dir(direction) {}

    vec3 origin() const { return orig; }
    vec3 direction() const { return dir; }
    vec3 at(float t) const { return orig + t * dir; }

    point3 orig;
    vec3 dir;
};

#endif
```

### 构建摄像机

光线追踪通过让光线通过像素来计算它们最终进入人眼时的颜色，主要步骤如下：

1. 计算从眼睛到像素的射线；
2. 计算射线与哪些物体相交；
3. 计算交点的颜色。

首先我们需要创建一个虚拟视口，视口的宽高比等于图像的宽高比。我们把视口的高设为 2 个单位长度，把投影点到投影平面的距离设为 1 个单位长度，摄像机坐标设为 $(0,0,0)$：

![1668327883766](assets/1668327883766.png)

代码如下：

```C++
auto viewport_height = 2.0;
auto viewport_width = aspect_ratio * viewport_height;
auto focal_length = 1.0;
auto origin = point3(0, 0, 0);
auto horizontal = vec3(viewport_width, 0, 0);
auto vertical = vec3(0, viewport_height, 0);
auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
```

### 渲染渐变蓝色天空背景

我们写一个根据 $y$ 轴坐标返回从白到蓝渐变色的函数：

```C++
color ray_color(const ray &r)
{
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}
```

然后在渲染循环中调用它获取当前像素颜色：

```C++
auto u = double(i) / (image_width - 1);
auto v = double(j) / (image_height - 1);
ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
color pixel_color = ray_color(r);
write_color(i, j, pixel_color);
```

效果如下：

![1668328302002](assets/1668328302002.png)

## Task2 渲染一个简单的球形物体

### 添加一个球体

设球心为 $\boldsymbol{C}=(C_x,C_y,C_z)$，球的半径为 $r$，则球面上的点 $\boldsymbol P$ 可以用如下方程表示：

$$
(\boldsymbol{P} - \boldsymbol{C})^2=r^2
$$

为了求出射线 $\boldsymbol{P}(t)=\boldsymbol{A}+t\boldsymbol{b}$ 与球面是否有交点，解方程：

$$
(\boldsymbol{A}+t\boldsymbol{b}-\boldsymbol{C})^2=r^2
$$

这是一个关于 $t$ 的一元二次方程，我们可以用判别式判断其是否有解。

我们在 $z$ 轴的 $-1$ 处放一个小球，并把与之相交的光线染成红色：

```C++
color ray_color(const ray &r)
{
    if (hit_sphere(point3(0, 0, -1), 0.5, r))
        return color(1, 0, 0);
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

bool hit_sphere(const point3 &center, double radius, const ray &r)
{
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto b = 2.0 * dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    return (discriminant > 0);
}
```

效果如下：

![1668331272997](assets/1668331272997.png)

但是这样做存在一个问题，我们没有区分物体是在我们面前还是背后（如果方程的解 $t<0$，那么物体在我们背后）。

### 表面法线

球体上点 $\boldsymbol P$ 处的表面法线可以用 $\boldsymbol{P}-\boldsymbol{C}$ 表示，我们通过把法向量的 $x,y,z$ 坐标映射到 $[0,1]$ 并解释为 RGB 值来可视化这些法线：

```C++
color ray_color(const ray &r)
{
    auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
    if (t > 0.0) {
        vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
        return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
    }
    vec3 unit_direction = unit_vector(r.direction());
    t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

double hit_sphere(const point3 &center, double radius, const ray &r)
{
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto b = 2.0 * dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}
```

效果如下：

![1668334774539](assets/1668334774539.png)

### 简化判断射线与球面相交的代码

上面我们用求根公式求解一元二次方程来判断射线与球面是否相交，我们可以对求根公式进行简化来减少计算量：

令 $h={b\over2}$

$$
\begin{split}
&\frac{-b \pm \sqrt{b^{2}-4 a c}}{2 a} \\
=&\frac{-2 h \pm \sqrt{(2 h)^{2}-4 a c}}{2 a} \\
=&\frac{-2 h \pm 2 \sqrt{h^{2}-a c}}{2 a} \\
=&\frac{-h \pm \sqrt{h^{2}-a c}}{a}
\end{split}
$$

简化后的代码如下：

```C++
double hit_sphere(const point3 &center, double radius, const ray &r)
{
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant)) / a;
    }
}
```

### `hittable` 抽象类

`hittable` 抽象类有一个 `hit` 函数，它计算射线位于 `t_min` 和 `t_max` 之间的部分是否与对象相交，如果相交，则把法向量等信息存储在一个记录里：

```C++
struct hit_record {
    point3 p;
    vec3 normal;
    double t;
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};
```

我们让球体类继承自 `hittable` 类：

```C++
class sphere : public hittable {
public:
    sphere() {}
    sphere(point3 cen, double r) : center(cen), radius(r) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
    point3 center;
    double radius;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;

    return true;
}
```

### 正面和背面

在前面的代码中，表面法线总是从球体的内部指向外部，无论射线是从内部射向球面还是从外部射向球面的。这样，当法线和射线方向相反时，说明射线是从物体外部射入的；当法线和射线方向相同时，说明射线是从物体内部射出的，通过射线和法向量做点乘就可以判断正反面。

但是，如果我们想让法线总是指向射线入射的一侧，那么就需要有一个额外的变量来存储射线是从哪一侧射入的：

```C++
struct hit_record {
    point3 p;
    vec3 normal;
    double t;
    bool front_face;

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    ...

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return true;
}
```

### `hittable` 对象的列表

```C++
class hittable_list : public hittable {
public:
    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }

    virtual bool hit(
        const ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
    std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}
```

### 常用常量和函数

我们在头文件 `rtweekend.h` 中定义无穷大和 $\pi$ 等常量和一些函数：

```C++
#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

// Common Headers
#include "ray.h"
#include "vec3.h"

#endif
```

然后修改 `main.cpp`：

```C++
void rendering()
{
    ...

    // World
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    ...

    // Render
    for (int j = image_height - 1; j >= 0; j--)
    {
        for (int i = 0; i < image_width; i++)
        {
            auto u = double(i) / (image_width - 1);
            auto v = double(j) / (image_height - 1);
            ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
            color pixel_color = ray_color(r, world);
            write_color(i, j, pixel_color);
        }
    }

    ...
}

color ray_color(const ray &r, const hittable& world)
{
    hit_record rec;
    if (world.hit(r, 0, infinity, rec)) {
        return 0.5 * (rec.normal + color(1, 1, 1));
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

```

效果如下：

![1668338927979](assets/1668338927979.png)

### 反走样

创建一个 `camera` 类来管理虚拟摄像机和场景采样任务：

```C++
class camera {
public:
    camera() {
        auto aspect_ratio = 16.0 / 9.0;
        auto viewport_height = 2.0;
        auto viewport_width = aspect_ratio * viewport_height;
        auto focal_length = 1.0;

        origin = point3(0, 0, 0);
        horizontal = vec3(viewport_width, 0, 0);
        vertical = vec3(0, viewport_height, 0);
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
    }

    ray get_ray(double u, double v) const {
        return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
    }

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
};
```

为了缓解物体边缘的“锯齿效应”，我们向一个像素内发射多条光线，采样得到多个颜色值然后取平均。在 `main.cpp` 的渲染循环中，我们每个像素采样 100 次并把颜色值累加：

```C++
// Render
for (int j = image_height - 1; j >= 0; j--) {
    for (int i = 0; i < image_width; i++) {
        color pixel_color(0, 0, 0);
        for (int s = 0; s < samples_per_pixel; s++) {
            auto u = (i + random_double()) / (image_width - 1);
            auto v = (j + random_double()) / (image_height - 1);
            ray r = cam.get_ray(u, v);
            pixel_color += ray_color(r, world);
        }
        write_color(i, j, pixel_color, samples_per_pixel);
    }
}
```

在 `write_color` 函数中，把颜色值取平均：

```C++
void write_color(int x, int y, color pixel_color, int samples_per_pixel) {
    // Divide the color by the number of samples
    auto scale = 1.0 / samples_per_pixel;
    pixel_color *= scale;

    // Note: x -> the column number, y -> the row number
    gCanvas[y][x] = pixel_color;
}
```

效果如下：

| 反走样前                                 | 反走样后                                 |
| ---------------------------------------- | ---------------------------------------- |
| ![1668341973321](assets/1668341973321.png) | ![1668342048360](assets/1668342048360.png) |

整体效果：

![1668342121815](assets/1668342121815.png)

## Task 3 添加漫反射材质、金属材质和电解质材质

### 3.1 漫反射材质

#### 3.1.1 简单的漫反射材质

漫反射材质不会发光，而是根据自己固有的颜色调节周围环境的颜色。漫反射材质表面反射光线的方向是随机的。

有一束光照到物体表面上一点 $P$，要求它的漫反射光线。取切于点 $\boldsymbol{P}$ 的两个单位半径的球体，它们的球心分别为 $\boldsymbol{P}+\boldsymbol{n}$ 和 $\boldsymbol{P}-\boldsymbol{n}$，前者在表面的外部，后者在表面的内部。在前者内随机取一点 $\boldsymbol{S}$，射线 $\boldsymbol{S}-\boldsymbol{P}$ 就是一条漫反射光线。

我们需要一种在单位半径的球体内随机取一点的方法：首先在 $x,y,z\in[-1,1]$ 的立方体内取一点，如果该点不在球体内就重新取。代码如下：

```C++
class vec3 {
  public:
    ...
    inline static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    inline static vec3 random(double min, double max) {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }
    ...
}

vec3 random_in_unit_sphere() {
    while (true) {
        auto p = vec3::random(-1, 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}
```

修改 `ray_color()` 函数函数，`depth` 参数用于控制函数的递归深度：

```C++
color ray_color(const ray &r, const hittable &world) {
    hit_record rec;
    if (world.hit(r, 0, infinity, rec)) {
        point3 target = rec.p + rec.normal + random_in_unit_sphere();
        return 0.5 * ray_color(ray(rec.p, target - rec.p), world);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}
```

效果如下：

![1668671508404](assets/1668671508404.png)

#### 3.1.2 使用 Gamma 校正

两个球体对每次反射吸收 50% 的光，但是上图中两个球体看起来都非常暗，这是因为几乎所有图像查看器都假设图像经过了 gamma 校正。简单起见，我们使用 gamma 2，即先把图像的颜色值提升为它们的平方根：

```C++
void write_color(int x, int y, color pixel_color, int samples_per_pixel) {
    ...
    pixel_color = color(sqrt(pixel_color.x()), sqrt(pixel_color.y()), sqrt(pixel_color.z()));
    ...
}
```

效果如下：

![1668672899099](assets/1668672899099.png)

#### 3.1.3 真正的 Lambertian 反射

前面我们在单位半径的球体内随机取一点来决定漫反射的方向，单位半径球体内每个点被选中的概率服从均匀分布，下面我们将实现 Lambertian 分布，使得里法向量更近的点被选中的概率更大。我们通过随机选择单位球体上（而不是球体内）的点来实现它：

```C++
inline vec3 random_unit_vector() {
	return unit_vector(random_in_unit_sphere());
}
```

```C++
color ray_color(const ray &r, const hittable &world, int depth) {
    ...
    if (world.hit(r, 0.001, infinity, rec)) {
        point3 target = rec.p + rec.normal + random_unit_vector();
        return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1);
    }
    ...
}

```

效果如下：

![1668675433579](assets/1668675433579.png)

### 3.2 金属材质

#### 3.2.1 材质类

我们用一个材质类来封装以下两种行为：

* 产生散射光线；
* 散射时表明散射光线应该衰减多少。

代码如下：

```C++
#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"

struct hit_record;

class material {
public:
    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const = 0;
};

#endif
```

接下来我们在 `hit_record` 结构体中增加一个指向 `material` 对象的共享指针：

```C++
struct hit_record {
    ...
    shared_ptr<material> mat_ptr;
    ...
};
```

在 `sphere` 类中加入指向 `material` 对象的共享指针：

```C++
class sphere : public hittable {
public:
    sphere() {}
    sphere(point3 cen, double r, shared_ptr<material> m) :
        center(cen), radius(r), mat_ptr(m){};

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

public:
    point3 center;
    double radius;
    shared_ptr<material> mat_prt;
};

bool sphere::hit(const ray &r, double t_min, double t_max, hit_record &rec) const {
    ...
    rec.mat_ptr = mat_ptr;

    return true;
}
```

接下来对材料对光的散射和反射进行建模，先定义一种漫反射材质 `lambertian`：

```C++
class lambertian : public material {
public:
    lambertian(const color &a) :
        albedo(a) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

public:
    color albedo;
};
```

接下来定义金属材质：

```C++
class metal : public material {
public:
    metal(const color &a) :
        albedo(a) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

public:
    color albedo;
};
```

最后修改计算光线颜色的函数：

```C++
color ray_color(const ray &r, const hittable &world, int depth) {
    hit_record rec;

    if (depth <= 0) return color(0, 0, 0);
    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        return color(0, 0, 0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

```

现在，我们可以在 `main.cpp` 中增加一些金属材质的球体：

```C++
// World
hittable_list world;
auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8));
auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2));

world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

```

效果如下：

![1668869142000](assets/1668869142000.png)

#### 3.2.2 模糊反射

类似于漫反射材质，我们可以给金属材质的反射光方向也增加一些随机性：

```C++
class metal : public material {
public:
    metal(const color &a, double f) :
        albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

public:
    color albedo;
    double fuzz;
};
```

在 `main.cpp` 中传入模糊度参数：

```C++
auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
```

效果如下：

![1668869762206](assets/1668869762206.png)

### 3.3 电解质材质

#### 3.3.1 折射

电解质材质对光的折射可以用 Snell 定律来描述：

$$
\eta\sin\theta=\eta'\sin\theta'
$$

![1668870611012](assets/1668870611012.png)

折射光线 $\boldsymbol{R}'$ 可以分解为平行和垂直于 $\boldsymbol{n}'$ 的两部分：

$$
\begin{array}{c}
\mathbf{R}^{\prime}=\mathbf{R}_{\perp}^{\prime}+\mathbf{R}_{\|}^{\prime} \\
\mathbf{R}_{\perp}^{\prime}=\frac{\eta}{\eta^{\prime}}(\mathbf{R}+\cos \theta \mathbf{n}) = \frac{\eta}{\eta^{\prime}}(\mathbf{R}+(-\mathbf{R}\mathbf{n}) \mathbf{n}) \\
\mathbf{R}_{\|}^{\prime}=-\sqrt{1-\left|\mathbf{R}_{\perp}^{\prime}\right|^{2}} \mathbf{n}
\end{array}
$$

基于以上数学原理，在 `vec3.h` 中加入计算折射的函数：

```C++
vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
```

然后我们就可以编写电解质材料类了：

```C++
class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        color& attenuation,
        ray& scattered
    ) const override {
        attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;
        vec3 unit_direction = unit_vector(r_in.direction());
        vec3 refracted = refract(unit_direction, rec.normal, refraction_ratio);

        scattered = ray(rec.p, refracted);
        return true;
    }

public:
    double ir;
};
```

最后我们把两个球体的材质改成电解质材料：

```C++
auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
auto material_center = make_shared<dielectric>(1.5);
auto material_left = make_shared<dielectric>(1.5);
auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
```

效果如下：

![1668933012854](assets/1668933012854.png)

#### 3.3.2 全内反射

上面实现的效果依然是不正确的，因为当射线位于材料内部且折射系数很高时，Snell 定律并没有实数解，此时材料不会折射光线，而是反射光线。

```C++
virtual bool scatter(
    const ray& r_in,
    const hit_record& rec,
    color& attenuation,
    ray& scattered
) const override {
    attenuation = color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;
    vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    vec3 direction;
    if (cannot_refract) {
        direction = reflect(unit_direction, rec.normal);
    } else {
        direction = refract(unit_direction, rec.normal, refraction_ratio);
    }
    scattered = ray(rec.p, direction);
    return true;
}
```

效果如下：

![1668934805935](assets/1668934805935.png)

#### 3.3.3 Schlick 近似

真实玻璃的反射会随着视角的变化而变化，我们可以用 Schlick 近似来产生这种效果。

```C++
class dielectric : public material {
public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray& r_in,
        const hit_record& rec,
        color& attenuation,
        ray& scattered
    ) const override {
        attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;
        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;
        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())) {
            direction = reflect(unit_direction, rec.normal);
        } else {
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        scattered = ray(rec.p, direction);
        return true;
    }

public:
    double ir;

private:
    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};
```

另外，将球体的半径设为负数可以使表面法线朝内，我们可以利用这个特点来产生中空的玻璃球体：

```C++
world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.4, material_left));
world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));
```

效果如下：

![1668935419951](assets/1668935419951.png)

## 问题

### 1

如果在 `rtweekend.h` 中包含了 `vec3` 而在 `vec3` 中又包含了 `rtweekend.h`，即使加了头文件保护符，也会报错：

![1668671118460](assets/1668671118460.png)

将 `rtweekend.h` 中的

```
#include "vec3.h"
#include "ray.h"
```

去掉就好了

### 2

task3 在 `vec3.h` 中加入如下函数时会报错：

```
vec3 random_in_unit_sphere() {
	while (true) {
		auto p = vec3::random(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

```

![1668671249957](assets/1668671249957.png)

加上 `inline` 关键字就好了
