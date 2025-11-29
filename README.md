<img src="https://s2.loli.net/2025/11/17/jLJhisMQX2poZ7K.png" />

<h1 align="center">Aikari</h1>
<h4 align="center">Unified UX tweak tool for Seewo E+</h4>
<h4 align="center">希沃易 + 用户体验优化工具</h4>

<div align="center">
  <a href="https://aikari.aurax.cc/">首页 (WIP)</a> · <a href="https://github.com/HugoAura/Seewo-HugoAura">主项目</a> · <a href="https://hugo.aurax.cc/about">关于 (WIP)</a> · <a href="https://aikari.docs.aurax.cc/">文档 (WIP)</a> · <a href="https://github.com/HugoAura/HugoAura-Aikari/issues">反馈</a> · <a href="https://forum.smart-teach.cn/t/hugoaura">社区</a>
</div>

> [!TIP]
> **这是 Project HugoAura 的一部分**
> 
> 前往 [这里](https://github.com/HugoAura/Seewo-HugoAura) 查看主项目。

## ✨ 概述

Aikari 是一个针对 [希沃易 + 系列软件](https://e.seewo.com/) 的全面体验调整工具。通过各类子模块, 让用户对希沃易 + 软件取得更高的自由度、更好的用户体验。

## 🧱 子模块

- [x] **PLS (Proxy Layer Services, 代理层服务)**: 通过创建假 Broker & 假 Client 并修改设备 Hosts, 实现对 `SeewoCore` (希沃集控 - 基础服务) 的 MQTT 流量 MITM 篡改。可实现伪造上报虚假信息、伪造集控端指令等功能。
- [ ] **AIT (Aura Icy BreakThrough, 环·破冰)**: 通过 Hook Windows API 关机事件 & 与 `SeewoFreezeUpdateAssistant` (希沃集控 - 冰点更新 RPC 服务) 交互, 实现「属于你自己的冰点穿透」, 更新自定义软件等。
- [ ] **AEL (Aura EasiLogin, 环·零刻登录)**: 通过取代原由 `EasiAgent` 接管的 `local.id.seewo.com` 本地 SSO, 实现希沃易 + 体系应用的一键登入。

> [!IMPORTANT]
>
> README 仍在施工进程中, 更多内容请等待后续更新
