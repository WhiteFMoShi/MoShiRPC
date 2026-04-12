## chrono库
一直以来都没有很好的使用过chrono库，今天写定时器的时候算是稍微了解了一下吧。

### chrono中的三种时钟
1. `steady_clock`
2. `system_clock`
3. `high_resolution_clock`

虽然时钟有三种，但它们的本质几乎是一样的：**一个纪元时间点+一个duration**，这也是为什么我们使用`***::now()`可以和一个`duration`相加的原因，这本质是和它的实现有关的。

#### 1. steady_clock
该定时器保证它是递增的，因此**开销很小**，但是！它不是系统时钟，他和平时我们所看到的电脑、手机的系统时间是完全不一样的。

但是，由于它递增的特性，它很适合用来进行时间测量操作。在RPC中，==就适合使用steady_clock进行定时器的实现，因为若是使用system_clock进行实现的话，若是用户手动对系统时间进行调整，这将会导致定时器的崩溃==。

#### 2. system_clock
该定时器就是获取系统时间了，因此系统时间若是进行了回滚，该时间也会进行回滚。

syatem_clock一般用于log、时间戳这类，需要准确的人类可读的时间场景。
但是，想要实现人类可读这一点，单使用chrono库可能还是不太够，前文有所提及：==chrono库单实现本质是和duration有关==，而duration的`count()`方法也只是返回该duration单位的ticks，因此是不能显示时间戳的，要实现时间戳，在C++20之前还是需要进行一段麻烦的转换：
`time_point -> time_t -> tm`

#### 3. high_resolution_clock
该时钟拥有最高精度的时间显示，但是**为了实现这个高精度，它消耗了大量的资源**，high_resolution_clock没有规定其实现，因此，不同平台的不同实现可能会导致精度有所不同。最高的精度可以到1ns。

> [!注意]
在很多实现中，high_resolution_clock是steady_clokc的别名，但是这个没有保证。

### duration
该类被设计为表达一段持续的时间，它是chrono库的核心。
duration是一个**模版类**，它的基础类型可以不用怎么了解，它是它的基础类型的别名肯定还是比较清楚的：
- std::chrono::mins
- std::chrono::hours
- std::chrono::days
等一系列。

我稍微夸张点说，整个chrono库就是一堆duration实现的。

### now()
在[C++ refrence](https://cppreference.cn/w/cpp/chrono/steady_clock)就提到了：now()的本质就是返回了一个`time_point`，而time_point的本质就是一个纪元的起始+一个duration，这一点其实在[chrono中的三种时钟](#chrono中的三种时钟)中也有所说明。