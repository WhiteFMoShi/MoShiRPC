## 高
- [ ] 学习C++模版,尽可能编写类似Rust的Result的错误处理机制
- [x] 重写Protocol,当前的实际太差劲了
- [ ] 重构moslog
- [ ] RPC: 明确一次调用的最小模型与 API: `RpcClient::Invoke`/`RpcServer::Register`/`Start/Stop`
- [ ] RPC: 设计并落地协议帧(可流式解析): `magic/version/type/flags/request_id/timeout/body_len/checksum`
- [ ] RPC: 实现 Framing/Parser: 解决粘包/拆包/半包; 增加 `max_frame_size` 等安全阈值
- [ ] RPC: 实现 Codec(至少一种): 明确 envelope: `service/method/headers/status/payload` 并支持错误返回
- [ ] RPC: 实现 `Dispatcher/ServiceRegistry`: `service+method -> handler`，支持参数校验与错误码映射
- [ ] RPC: 实现客户端请求复用(同连接并发): `request_id -> promise/future`，支持超时回收
- [ ] RPC: 实现服务端执行模型: I/O 线程只做收发与解析，业务 handler 下沉到 `ThreadPool`
- [ ] Network: 引入 `Connection` 抽象(生命周期/状态机)，完善写路径: output buffer + `EPOLLOUT` 驱动(处理短写)
- [ ] Network: socket 工程化: nonblocking + `accept4` + `SO_REUSEADDR` + `TCP_NODELAY` + keepalive
- [ ] 测试: 端到端 RPC 集成测试: 单次调用/并发调用/同连接复用/超时/服务端异常/半包与乱序写

## 中
- [ ] RPC: 连接治理: 心跳/空闲断开/优雅关闭(half-close 处理)
- [ ] RPC: deadline 传播与取消: 过期请求丢弃; client cancel 不再等待 future
- [ ] RPC: 重试与幂等策略: 哪些错误可重试/退避; 服务端错误不重试
- [ ] RPC: 限流与背压: 每连接/全局并发上限、队列长度、payload size 上限、慢客户端保护
- [ ] RPC: 可观测性: 统一日志字段(request_id/service/method/latency)，最小 metrics(成功率/延迟分位)
- [ ] RPC: 服务发现与负载均衡: 先支持静态列表，再扩展 registry(etcd/consul 等)
- [ ] RPC: 安全性: TLS(可选)、鉴权 token/签名(可选)、输入校验与反序列化防御
- [ ] RPC: 生成/声明式接口: 宏/模板封装 register/invoke；后续可扩展 IDL -> stub 代码生成
- [ ] RPC: Python binding(如果按文档目标): 用 pybind11 暴露 client/server API + demo service
- [ ] 文档: 写一篇最小可用教程(启动 server、注册 handler、client 调用、超时/错误示例)
- [ ] 基准: RPC 级 bench(吞吐/延迟/抖动/不同 payload) + 与 timer_bench 一起跑

## 低
- [ ] RPC: 压缩与校验: zstd/snappy + checksum 可选，协议 flags 协商
- [ ] RPC: 流式/双向 streaming(若需要): backpressure + window/credit
- [ ] 测试: 协议 parser 的 fuzz/随机测试(随机切片输入、恶意超长帧、畸形 header)
- [ ] 工程: CI(含 sanitizers/TSAN/ASAN)、clang-tidy、格式化与基准回归

## 已完成
### Timer(通用定时器)清单
- [x] 明确定义API契约: `Start/Stop/Clear/AddMsTask` 线程安全与可重入语义
- [x] `Stop()` 语义: 返回后不再有新回调执行; 远期任务存在时也要及时返回(唤醒 wait)
- [x] `Clear()` 语义: 取消未完成任务; 对 loop 任务的取消在 in-flight 竞态下也必须成立(禁止 Clear 后重排)
- [x] 正确性: `HandleAlarm_()` 避免 TOCTOU/数据竞争; `now` 需要在循环内更新
- [x] 生命周期: 析构/Stop 时处理 `joinable` 和 self-join 风险(回调线程内调用 Stop/析构)
- [x] 可测试性: 保留并发压力用例; 增加 in-flight Clear 取消、Stop 及时返回等回归测试
