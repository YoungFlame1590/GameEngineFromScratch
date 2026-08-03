# 游戏引擎项目 Git 合作流程规范

> 本文档定义了游戏引擎仓库的 Git 分支管理方案与协作流程，所有团队成员必须严格遵守。

---

## 一、分支体系总览

整个仓库共设置 **5 类分支**，各司其职：

| 分支类型 | 命名规范 | 生命周期 | 说明 |
|---|---|---|---|
| 主分支 | `main` | 永久 | 始终处于可构建、可发布的稳定状态 |
| 开发分支 | `develop` | 永久 | 日常开发的集成分支，所有功能在此汇合 |
| 功能分支 | `feature/xxx` | 临时 | 开发新功能，完成后合入 `develop` |
| 修复分支 | `hotfix/xxx` | 临时 | 紧急修复线上问题，合入 `main` 和 `develop` |
| 发布分支 | `release/x.x.x` | 临时 | 版本发布前的最终测试与修复 |

---

## 二、各分支详细说明

### 2.1 `main`（主分支）

- 每个 commit 都代表一个**可发布的稳定版本**
- **禁止直接推送**，只能通过 Pull Request 合入
- 每次合入后必须打 **Tag**（如 `v0.1.0`），方便回溯
- 主分支受保护分支机制保护，普通开发者无直接写入权限

### 2.2 `develop`（开发分支）

- 从 `main` 创建，是团队日常开发的**集成分支**
- 所有功能分支完成后都先合入这里
- 保证随时可以编译通过，但不保证完全无 Bug
- 禁止直接推送，通过 PR 合入，至少 1 人审核

### 2.3 `feature/xxx`（功能分支）

- 从 `develop` 创建，完成后通过 PR 合回 `develop`
- 命名规范：`feature/模块名`，使用连字符分隔
- 命名示例：
  - `feature/rendering-pipeline` — 渲染管线
  - `feature/physics-collision` — 物理碰撞
  - `feature/audio-system` — 音频系统
  - `feature/input-system` — 输入系统
- 分支上可自由推送，无需审核

### 2.4 `hotfix/xxx`（紧急修复分支）

- 从 `main` 创建，用于修复生产环境的紧急问题
- 修复完成后**同时合入 `main` 和 `develop`**
- 命名示例：`hotfix/memory-leak-renderer`
- 合入 `main` 后必须打 Tag 更新修订号

### 2.5 `release/x.x.x`（发布分支）

- 从 `develop` 创建，用于版本发布前的冻结测试
- 此分支上**只做 Bug 修复，不加新功能**
- 测试通过后合入 `main`（打 Tag）并同步回 `develop`
- 命名规范：`release/主版本号.次版本号.修订号`，如 `release/0.2.0`

---

## 三、分支保护规则

| 分支 | 直接推送 | 合并方式 | 审查要求 |
|---|---|---|---|
| `main` | 禁止 | Squash Merge | 至少 2 人审核 |
| `develop` | 禁止 | Squash Merge | 至少 1 人审核 |
| `feature/*` | 允许 | — | — |

---

## 四、日常工作流

### 4.1 开发者日常操作

```
步骤 1：同步最新代码
  git checkout develop
  git pull origin develop

步骤 2：创建功能分支
  git checkout -b feature/xxx

步骤 3：本地开发 & 提交
  git add .
  git commit -m "feat: 描述本次修改内容"

步骤 4：推送并发起 PR
  git push origin feature/xxx
  → 在网页端发起 Pull Request → 目标分支 develop

步骤 5：代码审查通过后合并，删除功能分支
  git branch -d feature/xxx          # 删除本地分支
  git push origin --delete feature/xxx  # 删除远程分支
```

### 4.2 Hotfix 紧急修复流程

```
步骤 1：从 main 创建修复分支
  git checkout main
  git pull origin main
  git checkout -b hotfix/xxx

步骤 2：修复并提交
  git add .
  git commit -m "fix: 描述修复内容"

步骤 3：推送并发起 PR
  git push origin hotfix/xxx
  → 发起 PR → 目标分支 main

步骤 4：合并后同步到 develop
  git checkout develop
  git merge hotfix/xxx
  git push origin develop

步骤 5：在 main 上打 Tag
  git tag -a v0.1.1 -m "修订号更新"
  git push origin v0.1.1
```

### 4.3 版本发布流程

```
步骤 1：从 develop 创建发布分支
  git checkout develop
  git pull origin develop
  git checkout -b release/0.2.0

步骤 2：在发布分支上修复 Bug
  git add .
  git commit -m "fix: 发布前 Bug 修复"
  git push origin release/0.2.0

步骤 3：测试通过后，合并到 main 并打 Tag
  git checkout main
  git merge release/0.2.0
  git tag -a v0.2.0 -m "版本 v0.2.0 发布"
  git push origin main --tags

步骤 4：同步回 develop
  git checkout develop
  git merge release/0.2.0
  git push origin develop

步骤 5：删除发布分支
  git branch -d release/0.2.0
  git push origin --delete release/0.2.0
```

---

## 五、提交信息规范

采用 **Conventional Commits** 规范，格式如下：

```
<类型>(可选的作用域): <描述>

[可选的正文]

[可选的脚注]
```

**类型说明：**

| 类型 | 说明 |
|---|---|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `docs` | 文档变更 |
| `style` | 代码格式调整（不影响逻辑） |
| `refactor` | 代码重构 |
| `test` | 测试相关 |
| `chore` | 构建/工具链变更 |

**示例：**

- `feat(rendering): 实现基础光照模型`
- `fix(physics): 修复碰撞检测内存泄漏`
- `docs(readme): 更新项目搭建说明`

---

## 六、版本号与 Tag 规范

采用**语义化版本（Semantic Versioning）**：`主版本号.次版本号.修订号`

| 版本号位 | 变更规则 | 示例场景 |
|---|---|---|
| 主版本号 | 架构级大改动，不兼容更新 | 从固定管线迁移到可编程管线 |
| 次版本号 | 新增功能模块，向后兼容 | 新增粒子系统 |
| 修订号 | Bug 修复，向后兼容 | 修复渲染器内存泄漏 |

**版本演进示例：** `v0.1.0` → `v0.2.0` → `v0.2.1` → `v1.0.0`

**Tag 操作：**

```bash
# 创建轻量 Tag
git tag v0.1.0

# 创建附注 Tag（推荐）
git tag -a v0.1.0 -m "首次发布 v0.1.0"

# 推送 Tag 到远程
git push origin v0.1.0
```

---

## 七、代码审查（Code Review）规范

### 7.1 Pull Request 要求

- PR 标题必须使用 Conventional Commits 格式
- PR 描述需包含：修改背景、变更内容、测试情况
- 涉及 API 变更的 PR 需同步更新文档
- 关联相关 Issue（如 `Closes #123`）

### 7.2 审查标准

- 代码逻辑正确，无潜在 Bug
- 符合项目编码规范
- 无未调试代码、调试打印语句残留
- 注释清晰，关键逻辑有说明
- 没有引入安全漏洞

---

## 八、游戏引擎项目额外建议

### 8.1 大文件管理

游戏引擎通常包含大量资源文件（模型、贴图、着色器等），建议使用 **Git LFS（Large File Storage）** 管理二进制大文件，避免仓库膨胀。

```bash
# 安装 Git LFS
git lfs install

# 跟踪大文件类型
git lfs track "*.png"
git lfs track "*.fbx"
git lfs track "*.wav"
git lfs track "*.ttf"
```

### 8.2 子模块拆分

如果引擎规模较大，可将核心模块拆分为独立仓库，通过 `git submodule` 引入：

- 物理引擎（如 Bullet、PhysX）
- 渲染引擎（如 DirectX/OpenGL/Vulkan 封装）
- 音频引擎（如 OpenAL、FMOD）
- 数学库（如 GLM）

### 8.3 CI/CD 集成

配置自动化流水线，每次 PR 自动执行：

- 编译检查（确保代码可构建）
- 单元测试（确保功能正确）
- 代码风格检查（lint 检查）
- 静态分析（检测潜在 Bug）

---

## 九、冲突解决流程

当多人并行开发时，如果两个分支同时修改了同一个文件的同一行代码，Git 无法自动判断保留哪一份，就会触发合并冲突。解决步骤：

1. 执行 `git merge <分支名>` 后，Git 会标记冲突文件
2. 打开冲突文件，找到冲突标记：`<<<<<<<`、`=======`、`>>>>>>>`
3. 手动编辑，保留正确的代码，删除冲突标记
4. 将修复后的文件加入暂存区：`git add <文件>`
5. 完成合并提交：`git commit -m "merge: 解决冲突"`

---

## 十、分支生命周期管理

| 分支类型 | 创建时机 | 合并目标 | 合并后操作 |
|---|---|---|---|
| `feature/xxx` | 开始新功能开发 | `develop` | 删除本地和远程分支 |
| `hotfix/xxx` | 修复生产紧急问题 | `main` + `develop` | 删除本地和远程分支 |
| `release/x.x.x` | 准备版本发布 | `main` + `develop` | 删除本地和远程分支 |

**重要原则：** 临时分支合并后必须及时删除，保持仓库整洁。

---

## 十一、权限角色定义

| 角色 | 权限说明 |
|---|---|
| **普通开发者（Dev）** | 可创建和推送 `feature/*`、`hotfix/*` 分支；通过 PR 请求合入 `develop`；不能直接推送到 `main` 和 `develop` |
| **核心开发者/管理员（Maintainer）** | 拥有 `main` 和 `develop` 的写入权限；负责审核 PR、解决复杂冲突、执行最终合并、管理版本 Tag |
| **项目所有者（Owner）** | 拥有所有分支的完全权限；可配置分支保护规则、管理团队成员权限 |

---

*本文档为游戏引擎项目的 Git 协作标准流程，所有团队成员应熟悉并严格遵守。如有变更需求，需通过 PR 提交修改并经过团队讨论。*
