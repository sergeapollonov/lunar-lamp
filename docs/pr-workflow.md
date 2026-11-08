# Pull Request Workflow — Instructions for Agents

This document defines the Pull Request (PR) workflow for the **Lunar Lamp** project, adapted from the standardized workflow established in the **Doma** repository.

When asked to "create a PR", "prepare a PR", or "publish a PR", the agent must execute the change on a feature branch, push the branch, and **create a fully documented PR on GitHub**. The final output to the user must include a clickable link to the created PR.

## 1. Parameters & Guardrails

| Parameter | Value |
| :--- | :--- |
| **GitHub Repository** | `sergeapollonov/lunar-lamp` |
| **Push Remote** | `origin` |
| **Target Base Branch** | `main` |
| **Task Branch Pattern** | `feature/<task-name>` or `fix/<task-name>` |
| **PR Title & Body Language** | English |
| **User Chat Communication** | Russian |

* Never commit directly to `main` for project features. All changes must be tracked through PRs.
* Merge is never executed automatically by the agent unless explicitly requested by the user.

## 2. Pre-PR Checklist & Verification

Before opening a PR:
1. Compile the firmware using `arduino-cli compile --fqbn esp32:esp32:esp32 .` or verify syntax.
2. Ensure no Wi-Fi SSIDs, passwords, or personal credentials are hardcoded.
3. Ensure power safety limits (`FastLED.setMaxPowerInVoltsAndMilliamps`) are respected.
4. Keep commit history clean and commits focused on the specific task.

## 3. PR Description Structure

Save the PR description as a clean Markdown file and pass it via `--body-file`:

```markdown
## Summary
Concise description of the change and rationale.

## Changes
- Bullet list of changes made.
- Architectural decisions or formulas implemented.

## Verification
- Compilation commands run and exit code.
- Serial Monitor output or hardware tests.

## Hardware & Safety Impact
- Power budget considerations and GPIO safety checks.
```

## 4. Branch & PR Creation Steps

```bash
git checkout -b feature/<task-name>
git add <files>
git commit -m "feat: description of task"
git push -u origin feature/<task-name>
gh pr create --repo sergeapollonov/lunar-lamp --base main --head feature/<task-name> --title "..." --body-file /tmp/pr-desc.md
```

## 5. Report to User

Briefly report to the user in Russian:
- Link to the created Pull Request.
- Summary of changes implemented.
- Compilation and test verification results.
