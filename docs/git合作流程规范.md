# Git 使用规范（个人学习项目）

> 自己手搓引擎，流程从简，但保持基本的好习惯。

---

## 一、分支策略

只用 **`main`** 一个主分支，新功能直接在上面开发，不做发布管理。

想尝试大改动怕搞坏时，拉个分支：

```bash
git checkout -b experiment/xxx   # 开实验分支
# 搞完了
git checkout main
git merge experiment/xxx         # 合回来
git branch -d experiment/xxx     # 删掉
```

---

## 二、提交信息规范

采用 **Conventional Commits**（好习惯，不费事）：

```
<类型>: <描述>
```

| 类型 | 说明 |
|---|---|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `docs` | 文档变更 |
| `refactor` | 代码重构 |
| `chore` | 杂项（构建、配置等） |

**示例：**

- `feat: 实现基础光照模型`
- `fix: 修复碰撞检测内存泄漏`
- `docs: 更新 README`

---

## 三、日常操作

```bash
# 拉最新
git pull origin main

# 改代码 → 提交
git add .
git commit -m "feat: 做了什么"

# 推送
git push origin main
```

---

## 四、冲突解决

```bash
git pull origin main    # 拉取时冲突
# 打开冲突文件，找到 <<<<<<< ======= >>>>>>> 标记
# 手动修改，保留正确代码，删掉标记
git add .
git commit -m "merge: 解决冲突"
git push origin main
```

---

## 五、不需要的东西

- ❌ 不搞 release 分支和版本 Tag
- ❌ 不搞 hotfix 流程
- ❌ 不需要 PR 和 Code Review（就自己一个人）
- ❌ 不搞 CI/CD

保持简单，专注写引擎。
